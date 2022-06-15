//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

#pragma once
/////////////////////////////////////////////
// written by robert rostek - tecxx@rrs.at //
/////////////////////////////////////////////

#define MP_SHAREDFOLDERS_FIRST	46901

class CDirectoryTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CDirectoryTreeCtrl)

public:
	// initialize control
	void Init(void);
	// SLUGFILLER: shareSubdir - two-list versions
	// get all shared directories
	void GetSharedDirectories(CStringList* list, CStringList* listsubdir);
	// set shared directories
	void SetSharedDirectories(CStringList* list, CStringList* listsubdir);
	// SLUGFILLER: shareSubdir

private:
	CImageList m_image; 
	// add a new item
	HTREEITEM AddChildItem(HTREEITEM hRoot, CString strText);
	// add subdirectory items
	void AddSubdirectories(HTREEITEM hRoot, CString strDir);
	// return the full path of an item (like C:\abc\somewhere\inheaven\)
	CString GetFullPath(HTREEITEM hItem);
	// returns true if strDir has at least one subdirectory
	bool HasSubdirectories(CString strDir);
	// check status of an item has changed
	void CheckChanged(HTREEITEM hItem, bool bChecked, bool bWithSubdir);	// SLUGFILLER: shareSubdir - two-list version
	// returns true if a subdirectory of strDir is shared
	bool HasSharedSubdirectory(CString strDir);
	// when sharing a directory, make all parent directories bold
	void UpdateParentItems(HTREEITEM hChild);
	void ShareSubDirTree(HTREEITEM hItem, BOOL bShare);
	// SLUGFILLER: shareSubdir - no double-shares
	// when sharing with subdirectories, prevent reshare of subdirectories
	void UnshareChildItems(CString strDir);
	// SLUGFILLER: shareSubdir

	// share list access
	bool IsShared(CString strDir, bool bCheckParent = false);	// SLUGFILLER: shareSubdir - allow checking indirect share
	void AddShare(CString strDir, bool bWithSubdir);	// SLUGFILLER: shareSubdir - two-list version
	void DelShare(CString strDir);
	// SLUGFILLER: shareSubdir - for removing checkboxes
	bool IsDisabled(HTREEITEM hItem);
	void SetDisable(HTREEITEM hItem, bool bDisable);
	void DisableChilds(HTREEITEM hItem, bool bDisable);
	// SLUGFILLER: shareSubdir

	CStringList m_lstShared;
	CStringList m_lstSharedSubdir;	// SLUGFILLER: shareSubdir - second list
	CString m_strLastRightClicked;
	bool m_bSelectSubDirs;

public:
	// construction / destruction
	CDirectoryTreeCtrl();
	virtual ~CDirectoryTreeCtrl();
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTvnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT OnFoundNetworkDrive(WPARAM wParam,LPARAM lParam);	// SLUGFILLER: shareSubdir - Multi-threading
};


// SLUGFILLER: shareSubdir - multi-threaded magic
class CNetworkEnumThread : public CWinThread
{
		DECLARE_DYNCREATE(CNetworkEnumThread)
protected:
	CNetworkEnumThread()	{}
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void	SetTarget(HWND hTarget);
private:
	void EnumNetworkDrives(NETRESOURCE *source);
	HWND	m_hTarget;
};
// SLUGFILLER: shareSubdir
