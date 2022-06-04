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
#include "DirectoryTreeCtrl.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include "TitleMenu.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define TM_FOUNDNETWORKDRIVE		(WM_USER + 0x101)	// SLUGFILLER: shareSubdir - Multi-threading

/////////////////////////////////////////////
// written by robert rostek - tecxx@rrs.at //
/////////////////////////////////////////////

struct STreeItem
{
	CString strPath;
};


// CDirectoryTreeCtrl

IMPLEMENT_DYNAMIC(CDirectoryTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CDirectoryTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnTvnItemexpanding)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnTvnGetdispinfo)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnTvnDeleteItem)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_DESTROY()
	ON_MESSAGE(TM_FOUNDNETWORKDRIVE, OnFoundNetworkDrive)	// SLUGFILLER: shareSubdir - Multi-threading
END_MESSAGE_MAP()

CDirectoryTreeCtrl::CDirectoryTreeCtrl()
{
	m_bSelectSubDirs = false;
}

CDirectoryTreeCtrl::~CDirectoryTreeCtrl()
{
	// don't destroy the system's image list
	m_image.Detach();
}

void CDirectoryTreeCtrl::OnDestroy()
{
	// If a treeview control is created with TVS_CHECKBOXES, the application has to 
	// delete the image list which was implicitly created by the control.
	CImageList *piml = GetImageList(TVSIL_STATE);
	if (piml)
		piml->DeleteImageList();

	CTreeCtrl::OnDestroy();
}

void CDirectoryTreeCtrl::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	CWaitCursor curWait;
	SetRedraw(FALSE);

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	// remove all subitems
	HTREEITEM hRemove = GetChildItem(hItem);
	while(hRemove)
	{
		DeleteItem(hRemove);
		hRemove = GetChildItem(hItem);
	}

	// get the directory
	CString strDir = GetFullPath(hItem);

	// fetch all subdirectories and add them to the node
	AddSubdirectories(hItem, strDir);

	SetRedraw(TRUE);
	Invalidate();
	*pResult = 0;
}

void CDirectoryTreeCtrl::ShareSubDirTree(HTREEITEM hItem, BOOL bRecurse)
{
	CWaitCursor curWait;
	SetRedraw(FALSE);
	//MORPH START sharesubdir 
	/*
	HTREEITEM hItemVisibleItem = GetFirstVisibleItem();
	CheckChanged(hItem, !GetCheck(hItem));
	if (bRecurse)
	{
		Expand(hItem, TVE_TOGGLE);
		HTREEITEM hChild = GetChildItem(hItem);
		while (hChild != NULL)
		{
			MarkChilds(hChild, !GetCheck(hItem));
			hChild = GetNextSiblingItem(hChild);
		}
		Expand(hItem, TVE_TOGGLE);
	}
	if (hItemVisibleItem)
		SelectSetFirstVisible(hItemVisibleItem);
	*/
	CheckChanged(hItem, !GetCheck(hItem), bRecurse==TRUE);	// SLUGFILLER: shareSubdir - Handled in a much cleaner fashion
	//MORPH END sharesubdir 

	SetRedraw(TRUE);
	Invalidate();
}

void CDirectoryTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	//VQB adjustments to provide for sharing or unsharing of subdirectories when control key is Down 
	UINT uHitFlags; 
	HTREEITEM hItem = HitTest(point, &uHitFlags); 
	if (hItem && (uHitFlags & TVHT_ONITEMSTATEICON))
		//MORPH START sharesubdir 
		/*
		ShareSubDirTree(hItem, nFlags & MK_CONTROL);
		*/
		ShareSubDirTree(hItem, (nFlags & MK_CONTROL) > 0 );
		//MORPH END sharesubdir 
	CTreeCtrl::OnLButtonDown(nFlags, point); 
}

void CDirectoryTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
	{
		HTREEITEM hItem = GetSelectedItem();
		if (hItem)
		{
			ShareSubDirTree(hItem, GetKeyState(VK_CONTROL) & 0x8000);

			// if Ctrl+Space is passed to the tree control, it just beeps and does not check/uncheck the item!
			if (!IsDisabled(hItem))	// SLUGFILLER: shareSubdir
			SetCheck(hItem, !GetCheck(hItem));
			return;
		}
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDirectoryTreeCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// If we let any keystrokes which are handled by us -- but not by the tree
	// control -- pass to the control, the user will hear a system event
	// sound (Standard Error!)
	BOOL bCallDefault = TRUE;

	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		if (nChar == VK_SPACE)
			bCallDefault = FALSE;
	}

	if (bCallDefault)
		CTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
}

// SLUGFILLER: shareSubdir
// This part makes the checkboxs disappear
void CDirectoryTreeCtrl::DisableChilds(HTREEITEM hItem, bool bDisable) {
	SetDisable(hItem, bDisable);
	HTREEITEM hChild;
	hChild = GetChildItem(hItem);
	while( hChild != NULL)
	{
		DisableChilds(hChild, bDisable);
		hChild = GetNextSiblingItem( hChild );
	}
}
// SLUGFILLER: shareSubdir

//MORPH START SHARESUBDIR remove code
/*
void CDirectoryTreeCtrl::MarkChilds(HTREEITEM hChild,bool mark) { 
	CheckChanged(hChild, mark); 
	SetCheck(hChild,mark);  
	Expand(hChild, TVE_TOGGLE); // VQB - make sure tree has entries 
	HTREEITEM hChild2; 
	hChild2 = GetChildItem(hChild); 
	while( hChild2 != NULL) 
	{ 
		MarkChilds(hChild2,mark); 
		hChild2 = GetNextSiblingItem( hChild2 ); 
	} 
	Expand(hChild, TVE_TOGGLE); // VQB - restore tree to initial disposition 
}
*/
//MORPH End SHARESUBDIR remove code

void CDirectoryTreeCtrl::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	pTVDispInfo->item.cChildren = 1;
	*pResult = 0;
}

// SLUGFILLER START: shareSubdir - local drives before network drives
static int CALLBACK DriveCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CString strItem1 = ((STreeItem*)lParam1)->strPath;
	CString strItem2 = ((STreeItem*)lParam2)->strPath;

	if (strItem1.Left(1) == _T('\\') && strItem2.Left(1) != _T('\\'))
		return 1;
	if (strItem1.Left(1) != _T('\\') && strItem2.Left(1) == _T('\\'))
		return -1;
	return strItem1.CompareNoCase(strItem2);
}
// SLUGFILLER END: shareSubdir

void CDirectoryTreeCtrl::Init(void)
{
	// Win98: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);

	ShowWindow(SW_HIDE);
	DeleteAllItems();

	// START: added by FoRcHa /////////////
	WORD wWinVer = thePrefs.GetWindowsVersion();	// maybe causes problems on 98 & nt4
	if (wWinVer != _WINVER_95_ && wWinVer != _WINVER_NT4_)
	{
		SHFILEINFO shFinfo;
		HIMAGELIST hImgList = NULL;

		// Get the system image list using a "path" which is available on all systems. [patch by bluecow]
		hImgList = (HIMAGELIST)SHGetFileInfo(_T("."), 0, &shFinfo, sizeof(shFinfo),
											 SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		if(!hImgList)
		{
			TRACE(_T("Cannot retrieve the Handle of SystemImageList!"));
			//return;
		}

		m_image.m_hImageList = hImgList;
		SetImageList(&m_image, TVSIL_NORMAL);
	}
	////////////////////////////////


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
			AddChildItem(NULL, drive); // e.g. "C:"

			// Point to the next drive
			pos += _tcslen(pos) + 1;
		}
	}

	// SLUGFILLER START: shareSubdir - local drives before network drives
	TVSORTCB tvs;

	tvs.hParent = NULL;
	tvs.lpfnCompare = DriveCompareProc;
	tvs.lParam = (LPARAM)this;

	SortChildrenCB(&tvs);

	// Multi-threaded network drives enumeration
	CNetworkEnumThread* networkenumthread = (CNetworkEnumThread*) AfxBeginThread(RUNTIME_CLASS(CNetworkEnumThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
	networkenumthread->SetTarget(m_hWnd);
	networkenumthread->ResumeThread();
	// SLUGFILLER END: shareSubdir

	ShowWindow(SW_SHOW);
}

HTREEITEM CDirectoryTreeCtrl::AddChildItem(HTREEITEM hRoot, CString strText)
{
	CString strPath = GetFullPath(hRoot);
	if (hRoot != NULL && strPath.Right(1) != _T("\\"))
		strPath += _T("\\");
	CString strDir = strPath + strText;
	TVINSERTSTRUCT itInsert = {0};
	
	// START: changed by FoRcHa /////
	WORD wWinVer = thePrefs.GetWindowsVersion();
	if (wWinVer != _WINVER_95_ && wWinVer != _WINVER_NT4_)
	{
		itInsert.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT |
							 TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		itInsert.item.stateMask = TVIS_BOLD | TVIS_STATEIMAGEMASK;
	}
	else
	{
		itInsert.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
		itInsert.item.stateMask = TVIS_BOLD;
	}
	// END: changed by FoRcHa ///////
	
	if (HasSharedSubdirectory(strDir))
		itInsert.item.state = TVIS_BOLD;
	else
		itInsert.item.state = 0;
	if (HasSubdirectories(strDir))
		itInsert.item.cChildren = I_CHILDRENCALLBACK;		// used to display the + symbol next to each item
	else
		itInsert.item.cChildren = 0;

	itInsert.item.pszText = const_cast<LPTSTR>((LPCTSTR)strText);
	itInsert.hInsertAfter = hRoot ? TVI_SORT : TVI_LAST;
	itInsert.hParent = hRoot;
	// SLUGFILLER START: shareSubdir - always include lParam for sort
	if (hRoot == NULL) {
		STreeItem* pti = new STreeItem;
		pti->strPath = strText;
		itInsert.item.mask |= TVIF_PARAM;
		itInsert.item.lParam = (LPARAM)pti;
	}
	// SLUGFILLER END: shareSubdir
	
	// START: added by FoRcHa ////////////////
	if (wWinVer != _WINVER_95_ && wWinVer != _WINVER_NT4_)
	{
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
			itInsert.itemex.iImage = shFinfo.iIcon;
			DestroyIcon(shFinfo.hIcon);
			if (hRoot == NULL && shFinfo.szDisplayName[0] != _T('\0'))
			{
				//MORPH START SHARESUBDIR old ode
				/*
				STreeItem* pti = new STreeItem;
				pti->strPath = strText;
				strText = shFinfo.szDisplayName;
				itInsert.item.pszText = const_cast<LPTSTR>((LPCTSTR)strText);
				itInsert.item.mask |= TVIF_PARAM;
				itInsert.item.lParam = (LPARAM)pti;
				*/
				// SLUGFILLER START: shareSubdir remove - lParam already present
				strText = shFinfo.szDisplayName;
				strText.Trim();	// SLUGFILLER: shareSubdir - clean up whitespaces
				itInsert.item.pszText = strText.GetBuffer();
				itInsert.item.cchTextMax = strText.GetLength();
				// SLUGFILLER END: shareSubdir remove - lParam already present
			}
		}

		if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo),
							SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON))
		{
			TRACE(_T("Error Gettting SystemFileInfo!"));
			itInsert.itemex.iImage = 0;
		}
		else
		{
			itInsert.itemex.iSelectedImage = shFinfo.iIcon;
			DestroyIcon(shFinfo.hIcon);
		}
	}
	// END: added by FoRcHa //////////////

	HTREEITEM hItem = InsertItem(&itInsert);
	//MORPH START SHARESUBDIR old code
	/*
	if (IsShared(strDir))
		SetCheck(hItem);
	*/
	// SLUGFILLER START: shareSubdir
	if (IsDisabled(hItem))	// new mode to look out for
		SetDisable(hItem, true);
	else if (IsShared(strDir,true))
		SetCheck(hItem);
	// SLUGFILLER END : shareSubdir

	return hItem;
}

CString CDirectoryTreeCtrl::GetFullPath(HTREEITEM hItem)
{
	CString strDir;
	HTREEITEM hSearchItem = hItem;
	while(hSearchItem != NULL)
	{
		CString strSearchItemDir;
		STreeItem* pti = (STreeItem*)GetItemData(hSearchItem);
		if (pti)
			strSearchItemDir = pti->strPath;
		else
			strSearchItemDir = GetItemText(hSearchItem);
		strDir = strSearchItemDir + _T("\\") + strDir;
		hSearchItem = GetParentItem(hSearchItem);
	}
	return strDir;
}

void CDirectoryTreeCtrl::AddSubdirectories(HTREEITEM hRoot, CString strDir)
{
	if (strDir.Right(1) != _T("\\"))
		strDir += _T("\\");
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
		
		CString strFilename = finder.GetFileName();
		if (strFilename.ReverseFind(_T('\\')) != -1)
			strFilename = strFilename.Mid(strFilename.ReverseFind(_T('\\')) + 1);
		AddChildItem(hRoot, strFilename);
	}
	finder.Close();
}

bool CDirectoryTreeCtrl::HasSubdirectories(CString strDir)
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

//MORPH START SHARESUBDIR old code
/*
void CDirectoryTreeCtrl::GetSharedDirectories(CStringList* list)
{
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; )
		list->AddTail(m_lstShared.GetNext(pos));
}
*/
// SLUGFILLER: shareSubdir - two lists now
void CDirectoryTreeCtrl::GetSharedDirectories(CStringList* list, CStringList* listsubdir)
{
	list->RemoveAll();
	listsubdir->RemoveAll();
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; )
		list->AddTail(m_lstShared.GetNext(pos));
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; )
		listsubdir->AddTail(m_lstSharedSubdir.GetNext(pos));
}
//MORPH END SHARESUBDIR

//MORPH START SHARESUBDIR old code
/*
void CDirectoryTreeCtrl::SetSharedDirectories(CStringList* list)
{
	m_lstShared.RemoveAll();

	for (POSITION pos = list->GetHeadPosition(); pos != NULL; )
	{
		CString str = list->GetNext(pos);
		if (str.Left(2)==_T("\\\\")) continue;
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		m_lstShared.AddTail(str);
	}
	Init();
}

*/
// SLUGFILLER: shareSubdir
void CDirectoryTreeCtrl::SetSharedDirectories(CStringList* list, CStringList* listsubdir) // ssd
{
	m_lstShared.RemoveAll();
	m_lstSharedSubdir.RemoveAll();

	for (POSITION pos = list->GetHeadPosition(); pos != NULL; )
	{
		CString str = list->GetNext(pos);
		if (str.Left(2)==_T("\\\\")) continue; //TODO CHECK MERGE leuk_he
		if (str.Right(1) != _T('\\'))
			str += _T('\\');

		if (!PathFileExists(str))	// only add directories which still exist
			continue;
		if (IsShared(str))
			continue;

		m_lstShared.AddTail(str);
	}

	DelShare(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
	//khaos::categorymod+
	/*
	for (int ix=1;ix<thePrefs.GetCatCount();ix++)
	*/
	for (int ix=0;ix<thePrefs.GetCatCount();ix++)
	//khaos::categorymod-
		DelShare(thePrefs.GetCatPath(ix));
	for (POSITION pos = listsubdir->GetHeadPosition(); pos != NULL; )
	{
		CString str = listsubdir->GetNext(pos);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		if (!PathFileExists(str))	// only add directories which still exist
			continue;
		if (IsShared(str, true))
			continue;
		m_lstSharedSubdir.AddTail(str);
		UnshareChildItems(str);
	}

	Init();
}
// SLUGFILLER: shareSubdir

bool CDirectoryTreeCtrl::HasSharedSubdirectory(CString strDir)
{
	// SLUGFILLER START: shareSubdir
	if (!HasSubdirectories(strDir))	// early check
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
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_lstShared.GetNext(pos);
		str.MakeLower();
		if (str.Find(strDir) == 0 && strDir != str)//strDir.GetLength() != str.GetLength())
			return true;
	}
	// SLUGFILLER START: shareSubdir - check second list
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_lstSharedSubdir.GetNext(pos);
		str.MakeLower();
		if (str.Find(strDir) == 0 || strDir.Find(str) == 0)
			return true;
	}
	// SLUGFILLER END: shareSubdir
	return false;
}

//SLUGFILLER: shareSubdir - child disabling goes here
/*
void CDirectoryTreeCtrl::CheckChanged(HTREEITEM hItem, bool bChecked)
{
	CString strDir = GetFullPath(hItem);
*/
void CDirectoryTreeCtrl::CheckChanged(HTREEITEM hItem, bool bChecked, bool bWithSubdir)
{
	if (IsDisabled(hItem))	// fail-safe, no access
		return;

	CString strDir = GetFullPath(hItem);
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');

	// Incoming directorys can only be shared in one way
	if (!bWithSubdir) {
		CString str = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		if (!str.CompareNoCase(strDir))
			bWithSubdir = true;
		else {
			//khaos::categorymod+
			/*
			for (int ix=1;ix<thePrefs.GetCatCount();ix++)
			*/
			for (int ix=0;ix<thePrefs.GetCatCount();ix++)
			//khaos::categorymod-
			{
				str = thePrefs.GetCatPath(ix);
				if (str.Right(1) != _T("\\"))
					str += _T("\\");
				str.MakeLower();

				if (!str.CompareNoCase(strDir)) {
					bWithSubdir = true;
					break;
				}
			}
		}
	}
// SHARESUBDIR END
	if (bChecked)
		//MORPH START SHARESUBDIR old code
		/*
		AddShare(strDir);
		*/
		AddShare(strDir, bWithSubdir);
		//MORPH END SHARESUBDIR old code
	else
		DelShare(strDir);

	UpdateParentItems(hItem);
	GetParent()->SendMessage(WM_COMMAND, UM_ITEMSTATECHANGED, (long)m_hWnd);
	// SLUGFILLER START : shareSubdir 
	if (bChecked && bWithSubdir)
		UnshareChildItems(strDir);	// no double-sharing

	// remove/restore checkboxes for all subdirectories
	HTREEITEM hChild;
	hChild = GetChildItem(hItem);
	while (hChild != NULL)
	{
		DisableChilds(hChild, bChecked && bWithSubdir);
		hChild = GetNextSiblingItem( hChild );
	}
	// SLUGFILLER END : shareSubdir 
}

// SLUGFILLER START : shareSubdir - allow checking indirect share
/*
bool CDirectoryTreeCtrl::IsShared(CString strDir)
*/
bool CDirectoryTreeCtrl::IsShared(CString strDir, bool bCheckParent)
// SLUGFILLER END : shareSubdir - allow checking indirect share
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_lstShared.GetNext(pos);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		if (str.CompareNoCase(strDir) == 0)
			return true;
	}
	// SLUGFILLER: shareSubdir - check second list
	strDir.MakeLower();
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_lstSharedSubdir.GetNext(pos);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		str.MakeLower();
		if ((bCheckParent && strDir.Find(str) == 0) || str == strDir)
			return true;
	}
	// SLUGFILLER: shareSubdir
	return false;
}

// SLUGFILLER START : shareSubdir - this is where the split really takes place
/*
void CDirectoryTreeCtrl::AddShare(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	
	if (IsShared(strDir) || !strDir.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)))
		return;

	m_lstShared.AddTail(strDir);
}
*/
void CDirectoryTreeCtrl::AddShare(CString strDir, bool bWithSubdir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	
	DelShare(strDir);	// in case switching between with subdirs and without
	if (IsShared(strDir, true))		// could be indirectly shared
		return;

	if (bWithSubdir)
		m_lstSharedSubdir.AddTail(strDir);
	else
	m_lstShared.AddTail(strDir);
}
// SLUGFILLER END : shareSubdir

void CDirectoryTreeCtrl::DelShare(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; )
	{
		POSITION pos2 = pos;
		CString str = m_lstShared.GetNext(pos);
		if (str.CompareNoCase(strDir) == 0)
			m_lstShared.RemoveAt(pos2);
	}
	// SLUGFILLER START: shareSubdir - check second list
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; )
	{
		POSITION pos2 = pos;
		CString str = m_lstSharedSubdir.GetNext(pos);
		if (str.CompareNoCase(strDir) == 0)
			m_lstSharedSubdir.RemoveAt(pos2);
	}
	// SLUGFILLER END: shareSubdir
}

// SLUGFILLER START: shareSubdir - checkbox removing code
bool CDirectoryTreeCtrl::IsDisabled(HTREEITEM hItem)
{
	CString strDir = GetFullPath(hItem);
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	strDir.MakeLower();
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_lstSharedSubdir.GetNext(pos);
		str.MakeLower();
		if (strDir.Find(str) == 0 && strDir != str)
			return true;
	}
	return false;
}
// SLUGFILLER END: shareSubdir

// SLUGFILLER START: shareSubdir - checkbox removing code
void CDirectoryTreeCtrl::SetDisable(HTREEITEM hItem, bool bDisable)
{
	if (bDisable){
		SetItemState(hItem, 0, TVIS_STATEIMAGEMASK);
		if (HasSubdirectories(GetFullPath(hItem)))
			SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
	}
	else {
		if (!(GetItemState(hItem, TVIS_STATEIMAGEMASK) & INDEXTOSTATEIMAGEMASK(3))){
			SetCheck(hItem, false);
			if (HasSubdirectories(GetFullPath(hItem)))
				SetItemState(hItem, 0, TVIS_BOLD);
		}
	}
}
// SLUGFILLER END: shareSubdir

void CDirectoryTreeCtrl::UpdateParentItems(HTREEITEM hChild)
{
	// SLUGFILLER START : shareSubdir - start from the top
	/*
	HTREEITEM hSearch = GetParentItem(hChild);
	*/
	HTREEITEM hSearch = hChild;
	// SLUGFILLER END : shareSubdir - start from the top
	while(hSearch != NULL)
	{
		if (HasSharedSubdirectory(GetFullPath(hSearch)))
			SetItemState(hSearch, TVIS_BOLD, TVIS_BOLD);
		else
			SetItemState(hSearch, 0, TVIS_BOLD);
		hSearch = GetParentItem(hSearch);
	}
}

// SLUGFILLER START: shareSubdir - removing double-shares
void CDirectoryTreeCtrl::UnshareChildItems(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; )
	{
		POSITION pos2 = pos;
		CString str = m_lstShared.GetNext(pos);
		if (str.Find(strDir) == 0 && strDir != str)
			m_lstShared.RemoveAt(pos2);
	}
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; )
	{
		POSITION pos2 = pos;
		CString str = m_lstSharedSubdir.GetNext(pos);
		if (str.Find(strDir) == 0 && strDir != str)
			m_lstSharedSubdir.RemoveAt(pos2);
	}
}
// SLUGFILLER END: shareSubdir

void CDirectoryTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
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

	CPoint ptMenu(-1, -1);
	if (point.x != -1 && point.y != -1)
	{
		ptMenu = point;
		ScreenToClient(&point);
	}
	else
	{
		HTREEITEM hSel = GetNextItem(TVI_ROOT, TVGN_CARET);
		if (hSel)
		{
			CRect rcItem;
			if (GetItemRect(hSel, &rcItem, TRUE))
			{
				ptMenu.x = rcItem.left;
				ptMenu.y = rcItem.top;
				ClientToScreen(&ptMenu);
			}
		}
		else
		{
			ptMenu.SetPoint(0, 0);
			ClientToScreen(&ptMenu);
		}
	}

	HTREEITEM hItem = HitTest(point);

	// create the menu
	CTitleMenu SharedMenu;
	SharedMenu.CreatePopupMenu();
	SharedMenu.AddMenuTitle(GetResString(IDS_SHAREDFOLDERS));
	bool bMenuIsEmpty = true;

	// add all shared directories
	int iCnt = 0;
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; iCnt++)
	{
		CString strDisplayPath(m_lstShared.GetNext(pos));
		PathRemoveBackslash(strDisplayPath.GetBuffer(strDisplayPath.GetLength()));
		strDisplayPath.ReleaseBuffer();
		SharedMenu.AppendMenu(MF_STRING,MP_SHAREDFOLDERS_FIRST+iCnt, GetResString(IDS_VIEW1) + strDisplayPath);
		bMenuIsEmpty = false;
	}
	// SLUGFILLER START: shareSubdir - add second list
	for (POSITION pos = m_lstSharedSubdir.GetHeadPosition(); pos != NULL; iCnt++)
	{
		CString strDisplayPath(m_lstSharedSubdir.GetNext(pos));
		PathRemoveBackslash(strDisplayPath.GetBuffer(strDisplayPath.GetLength()));
		strDisplayPath.ReleaseBuffer();
		SharedMenu.AppendMenu(MF_STRING,MP_SHAREDFOLDERS_FIRST+iCnt, GetResString(IDS_VIEW1) + strDisplayPath + GetResString(IDS_VIEW3) /*GetResString(IDS_VIEW3)*/);
		bMenuIsEmpty = false;
	}
	// SLUGFILLER END: shareSubdir

	// add right clicked folder, if any
	if (hItem)
	{
		m_strLastRightClicked = GetFullPath(hItem);
		if (!IsShared(m_strLastRightClicked))
		{
			CString strDisplayPath(m_strLastRightClicked);
			PathRemoveBackslash(strDisplayPath.GetBuffer(strDisplayPath.GetLength()));
			strDisplayPath.ReleaseBuffer();
			if (!bMenuIsEmpty)
				SharedMenu.AppendMenu(MF_SEPARATOR);
			SharedMenu.AppendMenu(MF_STRING, MP_SHAREDFOLDERS_FIRST-1, GetResString(IDS_VIEW1) + strDisplayPath + GetResString(IDS_VIEW2));
			bMenuIsEmpty = false;
		}
	}

	// display menu
	if (!bMenuIsEmpty)
		SharedMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, ptMenu.x, ptMenu.y, this);
	VERIFY( SharedMenu.DestroyMenu() );
}

void CDirectoryTreeCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	// catch WM_RBUTTONDOWN and do not route it the default way.. otherwise we won't get a WM_CONTEXTMENU.
	//CTreeCtrl::OnRButtonDown(nFlags, point);
}

BOOL CDirectoryTreeCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam < MP_SHAREDFOLDERS_FIRST)
	{
		ShellExecute(NULL, _T("open"), m_strLastRightClicked, NULL, NULL, SW_SHOW);
	}
	else
	{
		POSITION pos = m_lstShared.FindIndex(wParam - MP_SHAREDFOLDERS_FIRST);
		if (pos)
			ShellExecute(NULL, _T("open"), m_lstShared.GetAt(pos), NULL, NULL, SW_SHOW);
		// SLUGFILLER START: shareSubdir - check second list
		else {
			pos = m_lstSharedSubdir.FindIndex(wParam - m_lstShared.GetCount() - MP_SHAREDFOLDERS_FIRST);
			if (pos)
				ShellExecute(NULL, _T("open"), m_lstSharedSubdir.GetAt(pos), NULL, NULL, SW_SHOW);
		}
		// SLUGFILLER END: shareSubdir
	}

	return TRUE;
}

void CDirectoryTreeCtrl::OnTvnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pNMTreeView->itemOld.lParam)
		delete (STreeItem*)pNMTreeView->itemOld.lParam;
	*pResult = 0;
}

// SLUGFILLER START: shareSubdir - Multi-threading
static CStringList NetworkDrives;
static CCriticalSection NetworkDrivesLock;

LRESULT CDirectoryTreeCtrl::OnFoundNetworkDrive(WPARAM /* wParam*/,LPARAM /*lParam*/){
	NetworkDrivesLock.Lock();
	while (!NetworkDrives.IsEmpty())
		AddChildItem(NULL, NetworkDrives.RemoveHead());
	NetworkDrivesLock.Unlock();

	// local drives before network drives
	TVSORTCB tvs;

	tvs.hParent = NULL;
	tvs.lpfnCompare = DriveCompareProc;
	tvs.lParam = (LPARAM)this;

	SortChildrenCB(&tvs);

	return 0;
}

IMPLEMENT_DYNCREATE(CNetworkEnumThread, CWinThread)

void CNetworkEnumThread::SetTarget(HWND hTarget){
	m_hTarget = hTarget;
	NetworkDrivesLock.Lock();
	NetworkDrives.RemoveAll();	// Reset list for new enum
	NetworkDrivesLock.Unlock();
}

void CNetworkEnumThread::EnumNetworkDrives(NETRESOURCE *source)
{
	char buffer[sizeof(NETRESOURCE)+4*MAX_PATH];	// NETRESOURCE structure+4 strings
	NETRESOURCE &folder = (NETRESOURCE&)buffer;
	HANDLE hEnum;
	if (WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, source, &hEnum) != NO_ERROR)
		return;
	while (::IsWindow(m_hTarget)){
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
	if (::IsWindow(m_hTarget))
		::PostMessage(m_hTarget, TM_FOUNDNETWORKDRIVE, 0, 0);	// Notify update
}

int CNetworkEnumThread::Run(){
	// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash
	EnumNetworkDrives(NULL);
	return 0;
}
// SLUGFILLER END: shareSubdir
