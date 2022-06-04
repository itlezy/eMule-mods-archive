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
#include "FileDetailDialog.h" // sharesubdir
#include "emuledlg.h"
#include "TransferDlg.h"
#include "SharedFileList.h"
#include "SharedFilesWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TM_FOUNDNETWORKDRIVE		(WM_USER + 0x101)	// SLUGFILLER: shareSubdir - Multi-threading

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
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBeginDrag)
	ON_MESSAGE(TM_FOUNDNETWORKDRIVE, OnFoundNetworkDrive)	// SLUGFILLER: shareSubdir - Network drives enumerated
END_MESSAGE_MAP()

CSharedDirsTreeCtrl::CSharedDirsTreeCtrl()
{
	m_pRootDirectoryItem = NULL;
	m_bCreatingTree = false;
	m_pSharedFilesCtrl = NULL;
	m_pRootUnsharedDirectries = NULL;
	m_pDraggingItem = NULL;
	m_bFileSystemRootDirty = false;
	//MORPH START - Added, Downloaded History [Monki/Xman]
#ifndef NO_HISTORY
	pHistory=NULL;
#endif
	//MORPH END   - Added, Downloaded History [Monki/Xman]
	// SLUGFILLER START: shareSubdir - don't get deleted while we have a thread running
	NetworkThreadOffline = new CEvent(1, 1);
	NetworkThreadRun = true;	// quick exit
	// SLUGFILLER END: shareSubdir
}

CSharedDirsTreeCtrl::~CSharedDirsTreeCtrl()
{
	// SLUGFILLER START: shareSubdir - don't get deleted while we have a thread running
	NetworkThreadRun = false;	// quick exit
	NetworkThreadOffline->Lock();
	delete NetworkThreadOffline;
	// SLUGFILLER END: shareSubdir
	delete m_pRootDirectoryItem;
	delete m_pRootUnsharedDirectries;
	//MORPH START - Added, Downloaded History [Monki/Xman]
#ifndef NO_HISTORY
	delete pHistory;
#endif
	//MORPH END   - Added, Downloaded History [Monki/Xman]
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
		//MORPH START - Added, Downloaded History [Monki/Xman]
		/*
		pCurImageList->Replace(6, CTempIconLoader(_T("SharedFolderOvl")));	// 6: Overlay
		pCurImageList->Replace(7, CTempIconLoader(_T("NoAccessFolderOvl")));// 7: Overlay
		*/
		// when NO_HISTORY is defined the count for the following items is -1 of what's written there
		int iCount = 6;
#ifndef NO_HISTORY
		pCurImageList->Replace(iCount, CTempIconLoader(_T("DOWNLOAD"))); iCount++;//6
#endif

		// Avi3k: SharedView Ed2kType
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_Audio"))); iCount++; //7
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_Video"))); iCount++; //8
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_Picture"))); iCount++; //9
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_Program"))); iCount++; //10
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_Document"))); iCount++;  //11
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_Archive"))); iCount++; //12
		pCurImageList->Replace(iCount, CTempIconLoader(_T("SearchFileType_CDImage"))); iCount++; //13
		pCurImageList->Replace(iCount, CTempIconLoader(_T("AABCollectionFileType"))); iCount++; // 14
		// end Avi3k: SharedView Ed2kType

		pCurImageList->Replace(iCount, CTempIconLoader(_T("SharedFolderOvl"))); iCount++; // 15: Overlay
		pCurImageList->Replace(iCount, CTempIconLoader(_T("NoAccessFolderOvl")));// 16: Overlay
		//MORPH END   - Added, Downloaded History [Monki/Xman]
	}
	else
	{
		CImageList iml;
		iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
		iml.Add(CTempIconLoader(_T("AllFiles")));							// 0: All Directory
		iml.Add(CTempIconLoader(_T("Incomplete")));							// 1: Temp Directory
		iml.Add(CTempIconLoader(_T("Incoming")));							// 2: Incoming Directory
		iml.Add(CTempIconLoader(_T("Category")));							// 3: Cats
		iml.Add(CTempIconLoader(_T("HardDisk")));							// 4: All Dirs
		CString strTempDir(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
		if (strTempDir.Right(1) != _T("\\"))
			strTempDir += _T("\\");
		int nImage = theApp.GetFileTypeSystemImageIdx(strTempDir);			// 5: System Folder Icon
		if (nImage > 0 && theApp.GetSystemImageList() != NULL){
			HICON hIcon = ::ImageList_GetIcon(theApp.GetSystemImageList(), nImage, 0);
			iml.Add(hIcon);
			DestroyIcon(hIcon);
		}
		else{
			iml.Add(CTempIconLoader(_T("OpenFolder")));
		}
		//MORPH START - Added, Downloaded History [Monki/Xman]
		/*
		iml.SetOverlayImage(iml.Add(CTempIconLoader(_T("SharedFolderOvl"))), 1);	// 6: Overlay
		iml.SetOverlayImage(iml.Add(CTempIconLoader(_T("NoAccessFolderOvl"))), 2);	// 7: Overlay
		*/
		// when NO_HISTORY is defined the count for the following items is -1 of what's written there
#ifndef NO_HISTORY
		iml.Add(CTempIconLoader(_T("DOWNLOAD"))); //6
#endif

		//MORPH START - Added, SharedView Ed2kType [Avi3k]
		iml.Add(CTempIconLoader(_T("SearchFileType_Audio"))); //7
		iml.Add(CTempIconLoader(_T("SearchFileType_Video"))); //8
		iml.Add(CTempIconLoader(_T("SearchFileType_Picture"))); //9
		iml.Add(CTempIconLoader(_T("SearchFileType_Program"))); //10
		iml.Add(CTempIconLoader(_T("SearchFileType_Document")));  //11
		iml.Add(CTempIconLoader(_T("SearchFileType_Archive"))); //12
		iml.Add(CTempIconLoader(_T("SearchFileType_CDImage"))); //13
		iml.Add(CTempIconLoader(_T("AABCollectionFileType")));// 14
		//MORPH END   - Added, SharedView Ed2kType [Avi3k]

		iml.SetOverlayImage(iml.Add(CTempIconLoader(_T("SharedFolderOvl"))), 1); // 15: Overlay
		iml.SetOverlayImage(iml.Add(CTempIconLoader(_T("NoAccessFolderOvl"))), 2);	// 16: Overlay
		//MORPH END   - Added, Downloaded History [Monki/Xman]

		SetImageList(&iml, TVSIL_NORMAL);
		m_mapSystemIcons.RemoveAll();
		m_imlTree.DeleteImageList();
		m_imlTree.Attach(iml.Detach());
	}

	COLORREF crBk = GetSysColor(COLOR_WINDOW);
	COLORREF crFg = GetSysColor(COLOR_WINDOWTEXT);
	theApp.LoadSkinColorAlt(_T("SharedDirsTvBk"), _T("SharedFilesLvBk"), crBk);
	theApp.LoadSkinColorAlt(_T("SharedDirsTvFg"), _T("SharedFilesLvFg"), crFg);
	SetBkColor(crBk);
	SetTextColor(crFg);
}

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
	//MORPH START - Added, Downloaded History [Monki/Xman]
#ifndef NO_HISTORY
	delete pHistory;
#endif
	//MORPH END   - Added, Downloaded History [Monki/Xman]

	FetchSharedDirsList();

	m_pRootDirectoryItem = new CDirectoryItem(_T(""), TVI_ROOT);
	CDirectoryItem* pAll = new CDirectoryItem(_T(""), 0, SDI_ALL);
	pAll->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, GetResString(IDS_ALLSHAREDFILES), 0, 0, TVIS_EXPANDED, TVIS_EXPANDED, (LPARAM)pAll, TVI_ROOT, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pAll);
	
	CDirectoryItem* pIncoming = new CDirectoryItem(_T(""), pAll->m_htItem, SDI_INCOMING);
	pIncoming->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_INCOMING_FILES), 2, 2, 0, 0, (LPARAM)pIncoming, pAll->m_htItem, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pIncoming);
	
	CDirectoryItem* pTemp = new CDirectoryItem(_T(""), pAll->m_htItem, SDI_TEMP);
	pTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_INCOMPLETE_FILES), 1, 1, 0, 0, (LPARAM)pTemp, pAll->m_htItem, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pTemp);

	CDirectoryItem* pDir = new CDirectoryItem(_T(""), pAll->m_htItem, SDI_DIRECTORY);
	pDir->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, GetResString(IDS_SHARED_DIRECTORIES), 5, 5, TVIS_EXPANDED, TVIS_EXPANDED, (LPARAM)pDir, pAll->m_htItem, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pDir);

	m_pRootUnsharedDirectries = new CDirectoryItem(_T(""), TVI_ROOT, SDI_FILESYSTEMPARENT);
	m_pRootUnsharedDirectries->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN, GetResString(IDS_ALLDIRECTORIES), 4, 4, 0, 0, (LPARAM)m_pRootUnsharedDirectries, TVI_ROOT, TVI_LAST);

	//MORPH START - Added, Downloaded History [Monki/Xman]
#ifndef NO_HISTORY
	pHistory = new CDirectoryItem(_T(""), TVI_ROOT, SDI_DIRECTORY);
	pHistory->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, GetResString(IDS_DOWNHISTORY), 6, 6, TVIS_EXPANDED, TVIS_EXPANDED, (LPARAM)pHistory, TVI_ROOT, TVI_LAST);
#endif
	//MORPH END   - Added, Downloaded History [Monki/Xman]
}

bool CSharedDirsTreeCtrl::FilterTreeIsSubDirectory(CString strDir, CString strRoot, const CStringList& liDirs){
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
		// SLUGFILLER: shareSubdir - say "oops"
		/*
		CString strCurrent = thePrefs.shareddir_list.GetNext(pos);
*/
		CString strCurrent = liDirs.GetNext(pos);
		// SLUGFILLER: shareSubdir - say "oops"
		strCurrent.MakeLower();
		if (strCurrent.Right(1) != _T("\\")){
			strCurrent += _T("\\");
		}
		if (strRoot.Find(strCurrent, 0) != 0 && strDir.Find(strCurrent, 0) == 0 && strCurrent != strRoot && strCurrent != strDir)
			return true;
	}
	return false;
}

CString GetFolderLabel(const CString &strFolderPath, bool bTopFolder, bool bAccessible)
{
	CString strFolder(strFolderPath);
	PathRemoveBackslash(strFolder.GetBuffer());
	strFolder.ReleaseBuffer();

	CString strLabel(strFolder);
	if (strLabel.GetLength() == 2 && strLabel.GetAt(1) == _T(':'))
	{
		ASSERT( bTopFolder );
		strLabel += _T('\\');
	}
	else
	{
		strLabel = strLabel.Right(strLabel.GetLength() - (strLabel.ReverseFind(_T('\\')) + 1));
		if (bTopFolder)
		{
			CString strParentFolder(strFolder);
			PathRemoveFileSpec(strParentFolder.GetBuffer());
			strParentFolder.ReleaseBuffer();
			strLabel += _T("  (") + strParentFolder + _T(")");
		}
	}
	if (!bAccessible && bTopFolder)
		strLabel += _T(" [") + GetResString(IDS_NOTCONNECTED) + _T("]");

	return strLabel;
}

void CSharedDirsTreeCtrl::FilterTreeAddSubDirectories(CDirectoryItem* pDirectory, const CStringList& liDirs, 
													  int nLevel, bool &rbShowWarning, bool bParentAccessible){
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
		CString strCurrent = liDirs.GetNext(pos);
		CString strCurrentLow = strCurrent;
		strCurrentLow.MakeLower();
		if ( (strDirectoryPath.IsEmpty() || strCurrentLow.Find(strDirectoryPath + _T("\\"), 0) == 0) && strCurrentLow != strDirectoryPath){
			if (!FilterTreeIsSubDirectory(strCurrentLow, strDirectoryPath, liDirs)){
				bool bAccessible = !bParentAccessible ? false : (_taccess(strCurrent, 00) == 0);
				CString strName = GetFolderLabel(strCurrent, nLevel == 0, bAccessible);
				CDirectoryItem* pNewItem = new CDirectoryItem(strCurrent);
				pNewItem->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pNewItem, pDirectory->m_htItem, TVI_SORT);
				if (!bAccessible) {
					SetItemState(pNewItem->m_htItem, INDEXTOOVERLAYMASK(2), TVIS_OVERLAYMASK);
					rbShowWarning = true;
				}
				pDirectory->liSubDirectories.AddTail(pNewItem);
				FilterTreeAddSubDirectories(pNewItem, liDirs, nLevel+1, rbShowWarning, bAccessible);
			}
		}
	}
}

//MORPH START - Added, SharedView Ed2kType [Avi3k]
struct SEd2kTypeView
{
	int eType;
	UINT uStringId;
} _aEd2kTypeView[] =
{
	{ ED2KFT_AUDIO, IDS_SEARCH_AUDIO },
	{ ED2KFT_VIDEO, IDS_SEARCH_VIDEO },
	{ ED2KFT_IMAGE, IDS_SEARCH_PICS },
	{ ED2KFT_PROGRAM, IDS_SEARCH_PRG },
	{ ED2KFT_DOCUMENT, IDS_SEARCH_DOC },
	{ ED2KFT_ARCHIVE, IDS_SEARCH_ARC },
	{ ED2KFT_CDIMAGE, IDS_SEARCH_CDIMG },
	{ ED2KFT_EMULECOLLECTION, IDS_SEARCH_EMULECOLLECTION }
};
//MORPH END   - SharedView Ed2kType [Avi3k]

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

			case SDI_ALL:
				//MORPH START - Added, SharedView Ed2kType [Avi3]
				{
					for (int i = 0; i < ARRSIZE(_aEd2kTypeView); i++)
					{
						CDirectoryItem* pEd2kType = new CDirectoryItem(CString(_T("")), 0, SDI_ED2KFILETYPE, _aEd2kTypeView[i].eType);
#ifdef NO_HISTORY
						pEd2kType->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(_aEd2kTypeView[i].uStringId), i+6, i+6, 0, 0, (LPARAM)pEd2kType, pCurrent->m_htItem, TVI_LAST);
#else
						pEd2kType->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(_aEd2kTypeView[i].uStringId), i+7, i+7, 0, 0, (LPARAM)pEd2kType, pCurrent->m_htItem, TVI_LAST);
#endif
						pCurrent->liSubDirectories.AddTail(pEd2kType);
					}
				}
				//MORPH END   - Added, SharedView Ed2kType [Avi3]
				break;
			case SDI_INCOMING:{
				CString strMainIncDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
				if (strMainIncDir.Right(1) == _T("\\")){
					strMainIncDir = strMainIncDir.Left(strMainIncDir.GetLength()-1);
				}
				bool bShowWarning = false;
				if (thePrefs.GetCatCount() > 1){
					m_strliCatIncomingDirs.RemoveAll();
					for (int i = 0; i < thePrefs.GetCatCount(); i++){
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							CString strCatIncomingPath = pCatStruct->strIncomingPath;
							if (strCatIncomingPath.Right(1) == _T("\\")){
								strCatIncomingPath = strCatIncomingPath.Left(strCatIncomingPath.GetLength()-1);
							}
							if (!strCatIncomingPath.IsEmpty() && strCatIncomingPath.CompareNoCase(strMainIncDir) != 0
								&& m_strliCatIncomingDirs.Find(strCatIncomingPath) == NULL)
							{
								m_strliCatIncomingDirs.AddTail(strCatIncomingPath);
								bool bAccessible = _taccess(strCatIncomingPath, 00) == 0;
								CString strName = GetFolderLabel(strCatIncomingPath, true, bAccessible);
								CDirectoryItem* pCatInc = new CDirectoryItem(strCatIncomingPath, 0, SDI_CATINCOMING);
								pCatInc->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pCatInc, pCurrent->m_htItem, TVI_SORT);
								if (!bAccessible) {
									SetItemState(pCatInc->m_htItem, INDEXTOOVERLAYMASK(2), TVIS_OVERLAYMASK);
									bShowWarning = true;
								}
								pCurrent->liSubDirectories.AddTail(pCatInc);
							}
						}
					}
				}
				SetItemState(pCurrent->m_htItem, bShowWarning ? INDEXTOOVERLAYMASK(2) : 0, TVIS_OVERLAYMASK);
				break;
			}
			case SDI_TEMP:
				if (thePrefs.GetCatCount() > 1){
					for (int i = 0; i < thePrefs.GetCatCount(); i++){
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							//temp dir
							CDirectoryItem* pCatTemp = new CDirectoryItem(_T(""), 0, SDI_TEMP, i);
							pCatTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, pCatStruct->strTitle, 3, 3, 0, 0, (LPARAM)pCatTemp, pCurrent->m_htItem, TVI_LAST);
							pCurrent->liSubDirectories.AddTail(pCatTemp);

						}
					}
				}
				break;
			case SDI_DIRECTORY: {
				// SLUGFILLER START: shareSubdir - do both lists as well
				/*
				// add subdirectories
				bool bShowWarning = false;
				FilterTreeAddSubDirectories(pCurrent, m_strliSharedDirs, 0, bShowWarning, true);
				SetItemState(pCurrent->m_htItem, bShowWarning ? INDEXTOOVERLAYMASK(2) : 0, TVIS_OVERLAYMASK);
				*/
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
				bool bShowWarning = false;
				FilterTreeAddSubDirectories(pCurrent, strSubdirs, 0, bShowWarning, true);
				SetItemState(pCurrent->m_htItem, bShowWarning ? INDEXTOOVERLAYMASK(2) : 0, TVIS_OVERLAYMASK);
				// SLUGFILLER END: shareSubdir
				break;
			}
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
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	if (m_ShareDirsMenu) VERIFY( m_ShareDirsMenu.DestroyMenu() );

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOVERYLOW, GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING | MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING | MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK"));
	m_SharedFilesMenu.AppendMenu(MF_STRING | MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_UNSHAREDIR, GetResString(IDS_UNSHAREDIR));
	m_SharedFilesMenu.AppendMenu(MF_STRING, MP_UNSHAREDIRSUB, GetResString(IDS_UNSHAREDIRSUB));

	m_ShareDirsMenu.CreatePopupMenu();
	m_ShareDirsMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);
	m_ShareDirsMenu.AppendMenu(MF_STRING, MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_ShareDirsMenu.AppendMenu(MF_STRING | MF_SEPARATOR);
	m_ShareDirsMenu.AppendMenu(MF_STRING, MP_SHAREDIR, GetResString(IDS_SHAREDIR));
	m_ShareDirsMenu.AppendMenu(MF_STRING, MP_SHAREDIRSUB, GetResString(IDS_SHAREDIRSUB));
	m_ShareDirsMenu.AppendMenu(MF_STRING | MF_SEPARATOR);
	m_ShareDirsMenu.AppendMenu(MF_STRING, MP_UNSHAREDIR, GetResString(IDS_UNSHAREDIR));
	m_ShareDirsMenu.AppendMenu(MF_STRING, MP_UNSHAREDIRSUB, GetResString(IDS_UNSHAREDIRSUB));
}

void CSharedDirsTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{	
	if (point.x != -1 || point.y != -1) {
		CRect rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!rcClient.PtInRect(point)) {
			Default();
			return;
		}
	}

	CDirectoryItem* pSelectedDir = GetSelectedFilter();

	//MORPH START - Added, Downloaded History [Monki/Xman]
#ifndef NO_HISTORY
	if(pSelectedDir==pHistory)
		return; //no context menu
#endif
	//MORPH END   - Added, Downloaded History [Monki/Xman]

	if (pSelectedDir != NULL && pSelectedDir->m_eItemType != SDI_UNSHAREDDIRECTORY && pSelectedDir->m_eItemType != SDI_FILESYSTEMPARENT){
		int iSelectedItems = m_pSharedFilesCtrl->GetItemCount();
		int iCompleteFileSelected = -1;
		UINT uPrioMenuItem = 0;
		bool bFirstItem = true;
		for (int i = 0; i < iSelectedItems; i++)
		{
			const CKnownFile* pFile = (CKnownFile*)m_pSharedFilesCtrl->GetItemData(i);

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

		m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, (pSelectedDir != NULL ) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, (iCompleteFileSelected > 0 && !bWideRangeSelection) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_CMT, (iSelectedItems > 0 && !bWideRangeSelection) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_UNSHAREDIR, (pSelectedDir->m_eItemType == SDI_NO && !pSelectedDir->m_strFullPath.IsEmpty() && FileSystemTreeIsShared(pSelectedDir->m_strFullPath)) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_UNSHAREDIRSUB, (pSelectedDir->m_eItemType == SDI_DIRECTORY && ItemHasChildren(pSelectedDir->m_htItem) 
			|| (pSelectedDir->m_eItemType == SDI_NO && !pSelectedDir->m_strFullPath.IsEmpty() && (FileSystemTreeIsShared(pSelectedDir->m_strFullPath) 
			|| FileSystemTreeHasSharedSubdirectory(pSelectedDir->m_strFullPath, false)))) ? MF_ENABLED : MF_GRAYED);

		GetPopupMenuPos(*this, point);
		m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);
	}
	else if(pSelectedDir != NULL && pSelectedDir->m_eItemType == SDI_UNSHAREDDIRECTORY){
		// MORPH START sharesubdir 
		/*
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIR, FileSystemTreeIsShared(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIRSUB, (FileSystemTreeIsShared(pSelectedDir->m_strFullPath) || FileSystemTreeHasSharedSubdirectory(pSelectedDir->m_strFullPath, false)) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIR, !FileSystemTreeIsShared(pSelectedDir->m_strFullPath) && thePrefs.IsShareableDirectory(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIRSUB, FileSystemTreeHasSubdirectories(pSelectedDir->m_strFullPath) && thePrefs.IsShareableDirectory(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		*/
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIR, FileSystemTreeIsShared(pSelectedDir->m_strFullPath,false,false,false,true) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIRSUB, FileSystemTreeIsShared(pSelectedDir->m_strFullPath,false,false,true) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIR, !FileSystemTreeIsShared(pSelectedDir->m_strFullPath, true, true) ? MF_ENABLED : MF_GRAYED);	// SLUGFILLER: shareSubdir - don't suggest double sharing
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIRSUB, !FileSystemTreeIsShared(pSelectedDir->m_strFullPath, true, true,true/* only subdir */) ? MF_ENABLED : MF_GRAYED);
		// MORPH END sharesubdir 

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
	CTypedPtrList<CPtrList, CShareableFile*> selectedList;
	int iSelectedItems = m_pSharedFilesCtrl->GetItemCount();
	for (int i = 0; i < iSelectedItems; i++)
	{
		selectedList.AddTail((CShareableFile*)m_pSharedFilesCtrl->GetItemData(i));
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

	// file based
	if (selectedList.GetCount() > 0 && pSelectedDir != NULL)
	{
		CShareableFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		CKnownFile* pKnownFile = NULL;
		if (file != NULL && file->IsKindOf(RUNTIME_CLASS(CKnownFile)))
			pKnownFile = (CKnownFile*)file;

		switch (wParam){
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					CShareableFile* file = selectedList.GetNext(pos);
					if (file->IsKindOf(RUNTIME_CLASS(CKnownFile))){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += ((CKnownFile*)file)->GetED2kLink();
					}
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			// file operations
			case MP_REMOVE:
			case MPG_DELETE:
			{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				m_pSharedFilesCtrl->SetRedraw(FALSE);
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
				m_pSharedFilesCtrl->SetRedraw(TRUE);
				if (bRemovedItems) {
					m_pSharedFilesCtrl->AutoSelectItem();
					// Depending on <no-idea> this does not always cause a
					// LVN_ITEMACTIVATE message sent. So, explicitly redraw
					// the item.
					theApp.emuledlg->sharedfileswnd->ShowSelectedFilesDetails();
					theApp.emuledlg->sharedfileswnd->OnSingleFileShareStatusChanged(); // might have been a single shared file
				}
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MP_DETAIL:
			case MPG_ALTENTER:
				ShowFileDialog(selectedList);
				break;
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
			case MP_PRIOAUTO:
			{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						if (!selectedList.GetAt(pos)->IsKindOf(RUNTIME_CLASS(CKnownFile)))
							continue;
						CKnownFile* file = (CKnownFile*)selectedList.GetNext(pos);
						switch (wParam) {
							case MP_PRIOVERYLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYLOW);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIOLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_LOW);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIONORMAL:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_NORMAL);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIOHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_HIGH);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIOVERYHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYHIGH);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;	
							case MP_PRIOAUTO:
								file->SetAutoUpPriority(true);
								file->UpdateAutoUpPriority();
								m_pSharedFilesCtrl->UpdateFile(file); 
								break;
						}
					}
					break;
				}
			default:
				break;
		}
	}
	return TRUE;
}

void CSharedDirsTreeCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CShareableFile*>& aFiles, UINT uPshInvokePage)
{
	m_pSharedFilesCtrl->ShowFileDialog(aFiles, uPshInvokePage);
}

void CSharedDirsTreeCtrl::FileSystemTreeCreateTree()
{
	TCHAR drivebuffer[500];
	DWORD dwRet = GetLogicalDriveStrings(_countof(drivebuffer) - 1, drivebuffer);
	if (dwRet > 0 && dwRet < _countof(drivebuffer))
	{
		drivebuffer[_countof(drivebuffer) - 1] = _T('\0');

		const TCHAR* pos = drivebuffer;
		while(*pos != _T('\0')){

			// Copy drive name
			TCHAR drive[4];
			_tcsncpy(drive, pos, _countof(drive));
			drive[_countof(drive) - 1] = _T('\0');

			drive[2] = _T('\0');
			FileSystemTreeAddChildItem(m_pRootUnsharedDirectries, drive, true); // e.g. "C:"

			// Point to the next drive
			pos += _tcslen(pos) + 1;
		}
	}

	// SLUGFILLER START: shareSubdir - Multi-threaded network drives enumeration
	NetworkThreadRun = false;	// quick exit
	NetworkThreadOffline->Lock();	// Prevent a case of two running at the same time
	NetworkThreadRun = true;	// ready for startup
	NetworkThreadOffline->ResetEvent();	// Don't get deleted while the thread is running
	AfxBeginThread(EnumNetworkDrivesThreadProc, (LPVOID)this);
	// SLUGFILLER END: shareSubdir
}

void CSharedDirsTreeCtrl::FileSystemTreeAddChildItem(CDirectoryItem* pRoot, CString strText, bool bTopLevel)
{
	CString strPath = pRoot->m_strFullPath;
	if (strPath.Right(1) != _T("\\") && !strPath.IsEmpty())
		strPath += _T("\\");
	CString strDir = strPath + strText;
	TVINSERTSTRUCT itInsert;
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

	
	//MORPH START - Changed, ShareSubDir [Slugfiller]
	/*
	if (FileSystemTreeHasSharedSubdirectory(strDir, true) || FileSystemTreeIsShared(strDir))
	*/
	if (FileSystemTreeHasSharedSubdirectory(strDir, true) || FileSystemTreeIsShared(strDir,true,true))
	//MORPH END   - Changed, ShareSubDir [Slugfiller]
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
		//MORPH START - Changed, ShareSubDir [Slugfiller]
		/*
		if (FileSystemTreeIsShared(strDir)){
		*/
		if (FileSystemTreeIsShared(strDir,true,true)){
		//MORPH END   - Changed, ShareSubDir [Slugfiller]
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
	// Never try to enumerate the files of a drive and thus physically access the drive, just
	// for the information whether the drive has sub directories in the root folder. Depending
	// on the physical drive type (floppy disk, CD-ROM drive, etc.) this creates an annoying
	// physical access to that drive - which is to be avoided in each case. Even Windows
	// Explorer shows all drives by default with a '+' sign (which means that the user has
	// to explicitly open the drive to really get the content) - and that approach will be fine
	// for eMule as well.
	// Since the restriction for drives 'A:' and 'B:' was removed, this gets more important now.
	if (PathIsRoot(strDir))
		return true;
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

bool CSharedDirsTreeCtrl::FileSystemTreeHasSharedSubdirectory(CString strDir, bool bOrFiles)
{
	// SLUGFILLER START: shareSubdir
	if (!FileSystemTreeHasSubdirectories(strDir) && !bOrFiles)	// early check
		return false;
	// SLUGFILLER END: shareSubdir
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	strDir.MakeLower();
	// SLUGFILLER START: shareSubdir - check incoming dirs first
	CString istr = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	if (istr.Right(1) != _T('\\'))
		istr += _T('\\');
	istr.MakeLower();
	if (istr.Find(strDir) == 0 && strDir != istr)
		return true;
	//khaos::categorymod+
	/*
	for (int ix=1;ix<thePrefs.GetCatCount();ix++)
	*/
	for (int ix=0;ix<thePrefs.GetCatCount();ix++)
	//khaos::categorymod-
	{
		istr = thePrefs.GetCatPath(ix);
		if (istr.Right(1) != _T("\\"))
			istr += _T("\\");
		istr.MakeLower();
		if (istr.Find(strDir) == 0 && strDir != istr)
			return true;
	}
	// SLUGFILLER END: shareSubdir
	for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; )
	{
		CString strCurrent = m_strliSharedDirs.GetNext(pos);
		strCurrent.MakeLower();
		if (strCurrent.Find(strDir) == 0 && strDir != strCurrent)
			return true;
	}
	// SLUGFILLER START: shareSubdir - check second list
	for (POSITION pos = m_strliSharedDirsSubdir.GetHeadPosition(); pos != NULL; )
	{
		CString strCurrent = m_strliSharedDirsSubdir.GetNext(pos);
		strCurrent.MakeLower();
		if (strCurrent.Right(1) != _T("\\"))
			strCurrent += _T("\\");
		if (strCurrent.Find(strDir) == 0 || (strDir.Find(strCurrent) == 0 && strDir[strCurrent.GetLength()] == _T('\\')))
			return true;
	}
	// SLUGFILLER END: shareSubdir
	return bOrFiles && theApp.sharedfiles->ContainsSingleSharedFiles(strDir);
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

// SLUGFILLER: shareSubdir - allow checking indirect share
/*
bool CSharedDirsTreeCtrl::FileSystemTreeIsShared(CString strDir)
*/
bool CSharedDirsTreeCtrl::FileSystemTreeIsShared(CString strDir, bool bCheckParent, bool bCheckIncoming, bool bOnlySubdir, bool bOnlyNormalShared)
// SLUGFILLER: shareSubdir - allow checking indirect share
{
	if (!bOnlySubdir) // SLUGFILLER: shareSubdir - allow checking indirect share
	for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; )
	{
		if (CompareDirectories(m_strliSharedDirs.GetNext(pos), strDir) == 0)
			return true;
	}
	// SLUGFILLER START: shareSubdir - check second list
	if(!bOnlyNormalShared)
	{
		if (bCheckParent)
			strDir.MakeLower();
		for (POSITION pos = m_strliSharedDirsSubdir.GetHeadPosition(); pos != NULL; )
		{
			CString str = m_strliSharedDirsSubdir.GetNext(pos);
			if (str.Right(1) != _T('\\') && strDir.Right(1) == _T('\\')) // only add backslash when we need it for proper compare
				str += _T('\\');
			if (str.CompareNoCase(strDir) == 0 || // is this the shared with sub dir dir?
				(bCheckParent && strDir.Find(str.MakeLower()) == 0 && strDir[str.GetLength()] == _T('\\'))) // is this a shared subdir?
				return true;
		}
		if (bCheckIncoming) {
			// Don't single-share incoming directorys, since they're auto-shared
			CString istr = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
			if (istr.Right(1) != _T("\\"))
				istr += _T("\\");
			if (istr.CompareNoCase(strDir) == 0)
				return true;
			//khaos::categorymod+
			/*
			for (int ix=1;ix<thePrefs.GetCatCount();ix++)
			*/
			for (int ix=0;ix<thePrefs.GetCatCount();ix++)
			//khaos::categorymod-
			{
				istr = thePrefs.GetCatPath(ix);
				if (istr.Right(1) != _T("\\"))
					istr += _T("\\");
				if (istr.CompareNoCase(strDir) == 0)
					return true;
			}
		}
	}
	// SLUGFILLER END: shareSubdir
	return false;
}

void CSharedDirsTreeCtrl::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	pTVDispInfo->item.cChildren = 1;
	*pResult = 0;
}


// SLUGFILLER START: shareSubdir - code cloning, but in a good way
/*
void CSharedDirsTreeCtrl::AddSharedDirectory(CString strDir, bool bSubDirectories){
	if (!FileSystemTreeIsShared(strDir) && thePrefs.IsShareableDirectory(strDir)){
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
	}
}
*/
void CSharedDirsTreeCtrl::AddSharedDirectory(CString strDir, bool bSubDirectories){
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
}
// SLUGFILLER end : shareSubdir

void CSharedDirsTreeCtrl::RemoveSharedDirectory(CString strDir, bool bSubDirectories){
	if (strDir.Right(1) == _T("\\")){
		strDir = strDir.Left(strDir.GetLength()-1);
	}
	strDir.MakeLower();
	POSITION pos1, pos2;
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
	// SLUGFILLER START: shareSubdir - check second list
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
	// SLUGFILLER END: shareSubdir
}

void CSharedDirsTreeCtrl::RemoveAllSharedDirectories(){
	POSITION pos1, pos2;
	for (pos1 = m_strliSharedDirs.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_strliSharedDirs.GetNext(pos1);
		CString str = m_strliSharedDirs.GetAt(pos2);
		str.MakeLower();
		m_strliSharedDirs.RemoveAt(pos2);
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeUpdateBoldState(const CDirectoryItem* pDir){
	if (pDir == NULL)
		pDir = m_pRootUnsharedDirectries;
	//MORPH START - Changed, ShareSubDir [Slugfiller]
	/*
	SetItemState(pDir->m_htItem, ((FileSystemTreeHasSharedSubdirectory(pDir->m_strFullPath, true) || FileSystemTreeIsShared(pDir->m_strFullPath)) ? TVIS_BOLD : 0), TVIS_BOLD);
	*/
	SetItemState(pDir->m_htItem, ((FileSystemTreeHasSharedSubdirectory(pDir->m_strFullPath, true) || FileSystemTreeIsShared(pDir->m_strFullPath,true,true)) ? TVIS_BOLD : 0), TVIS_BOLD);
	//MORPH END   - Changed, ShareSubDir [Slugfiller]
	POSITION pos = pDir->liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		FileSystemTreeUpdateBoldState(pDir->liSubDirectories.GetNext(pos));
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeUpdateShareState(const CDirectoryItem* pDir){
	if (pDir == NULL)
		pDir = m_pRootUnsharedDirectries;
	//MORPH START - Changed, ShareSubDir [Slugfiller]
	/*
	SetItemState(pDir->m_htItem, FileSystemTreeIsShared(pDir->m_strFullPath) ? INDEXTOOVERLAYMASK(1) : 0, TVIS_OVERLAYMASK);
	*/
	SetItemState(pDir->m_htItem, FileSystemTreeIsShared(pDir->m_strFullPath,true,true) ? INDEXTOOVERLAYMASK(1) : 0, TVIS_OVERLAYMASK);
	//MORPH END   - Changed, ShareSubDir [Slugfiller]
	POSITION pos = pDir->liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		FileSystemTreeUpdateShareState(pDir->liSubDirectories.GetNext(pos));
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeSetShareState(const CDirectoryItem* pDir, bool bSubDirectories){
	if (m_bUseIcons && pDir->m_htItem != NULL)
		//MORPH START - Changed, ShareSubDir [Slugfiller]
		/*
		SetItemState(pDir->m_htItem, FileSystemTreeIsShared(pDir->m_strFullPath) ? INDEXTOOVERLAYMASK(1) : 0, TVIS_OVERLAYMASK);
		*/
		SetItemState(pDir->m_htItem, FileSystemTreeIsShared(pDir->m_strFullPath,true, !bSubDirectories) ? INDEXTOOVERLAYMASK(1) : 0, TVIS_OVERLAYMASK);
		//MORPH END   - Changed, ShareSubDir [Slugfiller]
	if (bSubDirectories){
		POSITION pos = pDir->liSubDirectories.GetHeadPosition();
		while (pos != NULL){
			FileSystemTreeSetShareState(pDir->liSubDirectories.GetNext(pos), true);
		}
	}
}

void CSharedDirsTreeCtrl::EditSharedDirectories(const CDirectoryItem* pDir, bool bAdd, bool bSubDirectories){
	ASSERT( pDir->m_eItemType == SDI_UNSHAREDDIRECTORY || pDir->m_eItemType == SDI_NO || (pDir->m_eItemType == SDI_DIRECTORY && !bAdd && pDir->m_strFullPath.IsEmpty()) );

	CWaitCursor curWait;
	if (bAdd)
		AddSharedDirectory(pDir->m_strFullPath, bSubDirectories);
	else if (pDir->m_eItemType == SDI_DIRECTORY)
		RemoveAllSharedDirectories();
	else
		RemoveSharedDirectory(pDir->m_strFullPath, bSubDirectories);

	if (pDir->m_eItemType == SDI_NO || pDir->m_eItemType == SDI_DIRECTORY) {
		// An 'Unshare' was invoked from within the virtual "Shared Directories" folder, thus we do not have
		// the tree view item handle of the item within the "All Directories" tree -> need to update the
		// entire tree in case the tree view item is currently visible.
		FileSystemTreeUpdateShareState();
	}
	else {
		// A 'Share' or 'Unshare' was invoked for a certain tree view item within the "All Directories" tree,
		// thus we know the tree view item handle which needs to be updated for showing the new share state.
		FileSystemTreeSetShareState(pDir, bSubDirectories);
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
	// SLUGFILLER START: shareSubdir - sync second list
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
	// SLUGFILLER END: shareSubdir

	//  update the sharedfiles list
	theApp.emuledlg->sharedfileswnd->Reload();
	if (GetSelectedFilter() != NULL && GetSelectedFilter()->m_eItemType == SDI_UNSHAREDDIRECTORY)
		m_pSharedFilesCtrl->UpdateWindow(); // if in filesystem view, update the list to reflect the changes in the checkboxes
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

		// SLUGFILLER START: shareSubdir - check for changes in shared subdirs
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
		// SLUGFILLER END: shareSubdir

		// check for changes in categories incoming dirs
		CString strMainIncDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		if (strMainIncDir.Right(1) == _T("\\"))
			strMainIncDir = strMainIncDir.Left(strMainIncDir.GetLength()-1);
		CStringList strliFound;
		for (int i = 0; i < thePrefs.GetCatCount(); i++){
			Category_Struct* pCatStruct = thePrefs.GetCategory(i);
			if (pCatStruct != NULL){
				CString strCatIncomingPath = pCatStruct->strIncomingPath;
				if (strCatIncomingPath.Right(1) == _T("\\"))
					strCatIncomingPath = strCatIncomingPath.Left(strCatIncomingPath.GetLength()-1);

				if (!strCatIncomingPath.IsEmpty() && strCatIncomingPath.CompareNoCase(strMainIncDir) != 0
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

	}
	if (bChanged || bForce){
		FetchSharedDirsList();
		FilterTreeReloadTree();
		if (m_bFileSystemRootDirty) {
			Expand(m_pRootUnsharedDirectries->m_htItem, TVE_COLLAPSE); // collapsing is enough to sync for the filtetree, as all items are recreated on every expanding
			m_bFileSystemRootDirty = false;
		}
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
		// SLUGFILLER START: shareSubdir
		if (!PathFileExists(strPath))	// only add directories which still exist
			continue;
		if (FileSystemTreeIsShared(strPath))	// don't double share
			continue;
		// SLUGFILLER END: shareSubdir
		m_strliSharedDirs.AddTail(strPath);
	}
	// SLUGFILLER START: shareSubdir - unshare incoming dirs(already auto-shared)
	RemoveSharedDirectory(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), false);
	//khaos::categorymod+
	/*
	for (int ix=1;ix<thePrefs.GetCatCount();ix++)
	*/
	for (int ix=0;ix<thePrefs.GetCatCount();ix++)
	//khaos::categorymod-
		RemoveSharedDirectory(thePrefs.GetCatPath(ix), false);
	m_strliSharedDirsSubdir.RemoveAll();
	pos = thePrefs.sharedsubdir_list.GetHeadPosition();
	// copy second list
	while (pos){
		CString strPath = thePrefs.sharedsubdir_list.GetNext(pos);
		if (strPath.Right(1) == _T("\\")){
			strPath = strPath.Left(strPath.GetLength()-1);
		}
		if (!PathFileExists(strPath))	// only add directories which still exist
			continue;
		if (FileSystemTreeIsShared(strPath, true))	// don't double share, and check indirect sharing
			continue;
		RemoveSharedDirectory(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), true);	// remove any new indirect double-shares
		m_strliSharedDirsSubdir.AddTail(strPath);
	}
	// SLUGFILLER END: shareSubdir
}

void CSharedDirsTreeCtrl::OnTvnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
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

void CSharedDirsTreeCtrl::OnVolumesChanged()
{
	m_bFileSystemRootDirty = true;
}

bool CSharedDirsTreeCtrl::ShowFileSystemDirectory(const CString& strDir)
{
	// expand directories untill we find our target directory and select it
	CDirectoryItem* pCurrentItem = m_pRootUnsharedDirectries;
	bool bContinue = true;
	while (bContinue)
	{
		bContinue = false;
		Expand(pCurrentItem->m_htItem, TVE_EXPAND);
		POSITION pos = pCurrentItem->liSubDirectories.GetHeadPosition();
		while (pos != NULL){
			CDirectoryItem* pTemp = pCurrentItem->liSubDirectories.GetNext(pos);
			if (strDir.CompareNoCase(pTemp->m_strFullPath + '\\') == 0)
			{
				Select(pTemp->m_htItem, TVGN_CARET);
				EnsureVisible(pTemp->m_htItem);
				return true;
			}
			else if (strDir.Find(pTemp->m_strFullPath + '\\') == 0)
			{
				pCurrentItem = pTemp;
				bContinue = true;
				break;
			}
		}
	}
	return false;
}

bool CSharedDirsTreeCtrl::ShowSharedDirectory(const CString& strDir)
{
	// expand directories untill we find our target directory and select it
	POSITION pos = m_pRootDirectoryItem->liSubDirectories.GetHeadPosition();
	while (pos != NULL)
	{
		CDirectoryItem* pTemp = m_pRootDirectoryItem->liSubDirectories.GetNext(pos);
		if (pTemp->m_eItemType == SDI_DIRECTORY)
		{
			Expand(pTemp->m_htItem, TVE_EXPAND);
			if (strDir.IsEmpty()) // we want the parent item only
			{
				Select(pTemp->m_htItem, TVGN_CARET);
				EnsureVisible(pTemp->m_htItem);
				return true;
			}
			else // search for the fitting sub dir
			{
				POSITION pos2 = pTemp->liSubDirectories.GetHeadPosition();
				while (pos2 != NULL)
				{
					CDirectoryItem* pTemp2 = pTemp->liSubDirectories.GetNext(pos2);
					if (strDir.CompareNoCase(pTemp2->m_strFullPath + _T('\\')) == 0)
					{
						Select(pTemp2->m_htItem, TVGN_CARET);
						EnsureVisible(pTemp2->m_htItem);
						return true;							
					}
				}
				return false;
			}
		}
	}
	return false;
}

void CSharedDirsTreeCtrl::ShowAllSharedFiles()
{
	Select(GetRootItem(), TVGN_CARET);
	EnsureVisible(GetRootItem());
}

// SLUGFILLER START: shareSubdir - Network drive enumeration(multi-threaded)
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
	// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash

	((CSharedDirsTreeCtrl*)pParam)->EnumNetworkDrives(NULL);
	((CSharedDirsTreeCtrl*)pParam)->NetworkThreadOffline->SetEvent();
	return 0;
}
// SLUGFILLER END: shareSubdir
