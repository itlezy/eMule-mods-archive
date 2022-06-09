// DirectoryTreeCtrl.cpp : implementation file
//
/////////////////////////////////////////////
// written by robert rostek - tecxx@rrs.at //
/////////////////////////////////////////////

#include "stdafx.h"
#include "DirectoryTreeCtrl.h"
#include "TitleMenu.h"
#include "otherfunctions.h"
#include "emule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDirectoryTreeCtrl, CTreeCtrl)
CDirectoryTreeCtrl::CDirectoryTreeCtrl()
{
	m_bSelectSubDirs = false;
	m_bCtrlPressed = false;
}

CDirectoryTreeCtrl::~CDirectoryTreeCtrl()
{
}

BEGIN_MESSAGE_MAP(CDirectoryTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnTvnItemexpanding)
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclickSharedList)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnTvnKeydown)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()


// CDirectoryTreeCtrl message handlers

void CDirectoryTreeCtrl::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	HTREEITEM hRemove = GetChildItem(hItem);

//	remove all subitems
	while(hRemove)
	{
		DeleteItem(hRemove);
		hRemove = GetChildItem(hItem);
	}

//	get the directory
	CString strDir = GetFullPath(hItem);

//	fetch all subdirectories and add them to the node
	AddSubdirectories(hItem, strDir);

	*pResult = 0;
}

void CDirectoryTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);
	HTREEITEM tItem = GetFirstVisibleItem();

	if((hItem) && (uFlags & TVHT_ONITEMSTATEICON))
	{
		CheckChanged(hItem, !GetCheck(hItem));

		if(nFlags & MK_CONTROL)
		{
			Expand(hItem, TVE_TOGGLE);
			HTREEITEM hChild;
			hChild = GetChildItem(hItem);
			while (hChild != NULL)
			{
				MarkChilds(hChild,!GetCheck(hItem));
				hChild = GetNextSiblingItem(hChild);
			}
			Expand(hItem, TVE_TOGGLE);
		}
	}
	SelectSetFirstVisible(tItem);
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void CDirectoryTreeCtrl::MarkChilds(HTREEITEM hChild, bool mark)
{
	CheckChanged(hChild, mark);
	SetCheck(hChild,mark);
	Expand(hChild, TVE_TOGGLE);
	HTREEITEM hChild2;
	hChild2 = GetChildItem(hChild);

	while( hChild2 != NULL)
	{
		MarkChilds(hChild2,mark);
		hChild2 = GetNextSiblingItem( hChild2 );
	}

	Expand(hChild, TVE_TOGGLE);
}

void CDirectoryTreeCtrl::Init(bool bAllowCDROM /*= true*/)
{
#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif

	DeleteAllItems();

	SHFILEINFO shFinfo;
	HIMAGELIST hImgList;
	CImageList imageList;

//	Get the system image list using a "path" which is available on all systems. [patch by bluecow]
	hImgList = (HIMAGELIST)SHGetFileInfo(_T("."), FILE_ATTRIBUTE_DIRECTORY, &shFinfo, sizeof(shFinfo), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	imageList.Attach(hImgList);
	SetImageList(&imageList, TVSIL_NORMAL);
//	Don't destroy the system's image list
	imageList.Detach();

	TCHAR drivebuffer[128], cDrv, *pos = drivebuffer;

	::GetLogicalDriveStrings(ARRSIZE(drivebuffer), drivebuffer); // e.g. "a:\ c:\ d:\"
	while(*pos != _T('\0'))
	{
		UINT	dwDrvType = ::GetDriveType(pos);

	//	Skip floppy drives (check letter as some USB drives can also be removable) and in some cases CD/DVD
		if ( ((dwDrvType != DRIVE_REMOVABLE) || (((cDrv = CHR2UP(*pos)) != _T('A')) && (cDrv != _T('B')))) &&
			(bAllowCDROM || (dwDrvType != DRIVE_CDROM)) )
		{
			pos[2] = _T('\0');
			AddChildItem(NULL, pos); // e.g. ("c:")
		}
	//	Point to the next drive (4 chars interval)
		pos += 4;
	}
}

HTREEITEM CDirectoryTreeCtrl::AddChildItem(HTREEITEM hRoot, CString strText)
{
	CString strDir = GetFullPath(hRoot);

	strDir += strText;
	if (hRoot == NULL)
		strDir += _T('\\');

	TV_INSERTSTRUCT	itInsert;
	SHFILEINFO		shFinfo;

	memzero(&itInsert, sizeof(itInsert));
	
	if ( SHGetFileInfo( strDir, FILE_ATTRIBUTE_DIRECTORY, &shFinfo, sizeof(shFinfo),
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES ) != NULL )
	{
		itInsert.item.mask |= TVIF_IMAGE;
		itInsert.item.iImage = shFinfo.iIcon;
	}
	
	if ( SHGetFileInfo( strDir, FILE_ATTRIBUTE_DIRECTORY, &shFinfo, sizeof(shFinfo),
		SHGFI_SYSICONINDEX | SHGFI_OPENICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES ) != NULL )
	{
		itInsert.item.mask |= TVIF_SELECTEDIMAGE;
		itInsert.item.iSelectedImage = shFinfo.iIcon;
	}

	if (hRoot != NULL)
		strDir += _T('\\');

	if (HasSharedSubdirectory(strDir))
		itInsert.item.state = TVIS_BOLD;

	if (HasSubdirectories(strDir))
		itInsert.item.cChildren = 1;		// used to display the + symbol next to each item

	itInsert.item.mask |= TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
	itInsert.item.stateMask = TVIS_BOLD;
	itInsert.item.pszText = const_cast<LPTSTR>(strText.GetString());
	itInsert.hInsertAfter = (hRoot == NULL) ? TVI_LAST : TVI_SORT;	// root items are already sorted
	itInsert.hParent = hRoot;

	HTREEITEM hItem = InsertItem(&itInsert);

	if (IsShared(strDir))
		SetCheck(hItem);

	return hItem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetFullPath() returns full path with '\\' at the end
CString CDirectoryTreeCtrl::GetFullPath(HTREEITEM hItem)
{
	CString strDir;
	HTREEITEM hSearchItem = hItem;

	while(hSearchItem != NULL)
	{
		strDir = GetItemText(hSearchItem) + _T('\\') + strDir;
		hSearchItem = GetParentItem(hSearchItem);
	}

	return strDir;
}

void CDirectoryTreeCtrl::AddSubdirectories(HTREEITEM hRoot, CString strDir)
{
	strDir += _T("*.*");

	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots())
			continue;
		if (finder.IsSystem())
			continue;
		if (!finder.IsDirectory())
			continue;
		
		CString	strFilename = finder.GetFileName();
		int		iIdx;

		if ((iIdx = strFilename.ReverseFind('\\')) != -1)
			strFilename = strFilename.Mid(iIdx + 1);

		AddChildItem(hRoot, strFilename);
	}

	finder.Close();
}

bool CDirectoryTreeCtrl::HasSubdirectories(CString strDir)
{
	strDir += _T("*.*");

	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir);

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


static bool DirectoryExists(const CString &Name)
{
	DWORD Code = GetFileAttributes(Name);
	return (Code != INVALID_FILE_ATTRIBUTES) && (Code & FILE_ATTRIBUTE_DIRECTORY);
}

bool CDirectoryTreeCtrl::HasSharedSubdirectory(CString strDir)
{
	CString str;

	strDir.MakeLower();
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL;)
	{
		str = m_lstShared.GetNext(pos);
		str.MakeLower();

		if (str.Find(strDir) == 0 && strDir != str)
			return DirectoryExists(str);
	}

	return false;
}

void CDirectoryTreeCtrl::CheckChanged(HTREEITEM hItem, bool bChecked)
{
	CString strDir = GetFullPath(hItem);

	if (bChecked)
		AddShare(strDir);
	else
		DelShare(strDir);

	UpdateParentItems(hItem);

	CWnd *pParent = GetParent();

	if(pParent)
		pParent->SendMessage(WM_COMMAND, DIRLIST_ITEMSTATECHANGED, (long)m_hWnd);
}

bool CDirectoryTreeCtrl::IsShared(CString strDir)
{
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL;)
	{
		CString str = m_lstShared.GetNext(pos);

		if (str.CompareNoCase(strDir) == 0)
			return true;
	}

	return false;
}

void CDirectoryTreeCtrl::AddShare(CString strDir)
{
	if (!IsShared(strDir) && strDir.CompareNoCase(g_App.m_pPrefs->GetConfigDir()))
		m_lstShared.AddTail(strDir);
}

void CDirectoryTreeCtrl::DelShare(CString strDir)
{
	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL;)
	{
		POSITION pos2 = pos;
		CString str = m_lstShared.GetNext(pos);

		if (str.CompareNoCase(strDir) == 0)
			m_lstShared.RemoveAt(pos2);
	}
}

void CDirectoryTreeCtrl::UpdateParentItems(HTREEITEM hChild)
{
	HTREEITEM hSearch = GetParentItem(hChild);

	while(hSearch != NULL)
	{
		if (HasSharedSubdirectory(GetFullPath(hSearch)))
			SetItemState(hSearch, TVIS_BOLD, TVIS_BOLD);
		else
			SetItemState(hSearch, 0, TVIS_BOLD);

		hSearch = GetParentItem(hSearch);
	}
}

void CDirectoryTreeCtrl::OnNMRclickSharedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
//	Get item under cursor
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point;
	ScreenToClient(&p);
	HTREEITEM hItem = HitTest(p);

	CTitleMenu  menuShared;

//	Create the menu
	menuShared.CreatePopupMenu();
	menuShared.AddMenuTitle(GetResString((m_lstShared.GetCount() == 0) ? IDS_NOSHAREDFOLDERS : IDS_SHAREDFOLDERS));

	bool		bFolder = false;

//	Add right clicked folder, if any
	if (hItem)
	{
		m_strLastRightClicked = GetFullPath(hItem);

		if (!IsShared(m_strLastRightClicked))
		{
			CString	strTmp;

			strTmp.Format(GetResString(IDS_VIEW_NOTSHARED), m_strLastRightClicked);
			menuShared.AppendMenu(MF_STRING, MP_SHAREDFOLDERS_FIRST - 1, strTmp);
			bFolder = true;
		}
	}

//	Add all shared directories
	int iCnt = 0;

	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL; iCnt++)
		menuShared.AppendMenu(MF_STRING,MP_SHAREDFOLDERS_FIRST+iCnt, (LPCTSTR)m_lstShared.GetNext(pos));

//	Display menu (do not display empty menu)
	if ((m_lstShared.GetHeadPosition() != NULL) || bFolder)
	{
		menuShared.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
	}
	*pResult = 0;
//	Menu objects are destroyed in their destructor
}

BOOL CDirectoryTreeCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	if (wParam < MP_SHAREDFOLDERS_FIRST)
	{
		ShellExecute(NULL, _T("open"), m_strLastRightClicked, NULL, NULL, SW_SHOW);
		return false;
	}

	unsigned cnt = 0;

	for (POSITION pos = m_lstShared.GetHeadPosition(); pos != NULL;)
	{
		CString str = m_lstShared.GetNext(pos);

		if (cnt == wParam - MP_SHAREDFOLDERS_FIRST)
		{
			ShellExecute(NULL, _T("open"), str, NULL, NULL, SW_SHOW);
			return true;
		}
		cnt++;
	}

	return true;
}

void CDirectoryTreeCtrl::OnTvnKeydown(NMHDR *pNMHDR, LRESULT *pResult)
{ 
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

	switch(pTVKeyDown->wVKey)
	{
		case VK_SPACE:
		case VK_RETURN:
		{
			HTREEITEM hItem = GetSelectedItem();

			if(hItem)
			{		
				HTREEITEM tItem = GetFirstVisibleItem();

				CheckChanged(hItem, !GetCheck(hItem));
								
				if(m_bCtrlPressed)
				{
					Expand(hItem, TVE_TOGGLE);
					HTREEITEM hChild;
					hChild = GetChildItem(hItem);

					while(hChild != NULL)
					{ 
						MarkChilds(hChild,!GetCheck(hItem));
						hChild = GetNextSiblingItem( hChild );
					}

					SetCheck(hItem, !GetCheck(hItem));
					Expand(hItem, TVE_TOGGLE);
				}
		 
				SelectSetFirstVisible(tItem);
			}

			break;
		}

		default:
			break;
	}

	*pResult = 0;
}

void CDirectoryTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_CONTROL)
		m_bCtrlPressed = true;

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDirectoryTreeCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_CONTROL)
		m_bCtrlPressed = false;

	CTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}
