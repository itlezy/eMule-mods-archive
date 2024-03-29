//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "TitleMenu.h"

enum ESpecialDirectoryItems{
	SDI_NO = 0,
	SDI_ALL,
//>>> WiZaRd::Tree Redesign
	SDI_AUDIO,
	SDI_VIDEO,
	SDI_IMAGE,
	SDI_PROGRAM,
	SDI_DOCUMENT,
	SDI_ARCHIVE,
	SDI_CDIMAGE,
	SDI_EMULECOLLECTION,
//<<< WiZaRd::Tree Redesign
	SDI_INCOMING,
	SDI_TEMP,
	SDI_DIRECTORY,
	SDI_CATINCOMING,
	SDI_ED2KFILETYPE_ALL,  //>>> WiZaRd::SharedView Ed2kType [Avi3k]
//>>> WiZaRd::SharedView Ed2kType [WiZaRd]
	SDI_ED2KFILETYPE_INC,  
	SDI_ED2KFILETYPE_TMP,  
	SDI_ED2KFILETYPE_SUB,  
//<<< WiZaRd::SharedView Ed2kType [WiZaRd]
	SDI_ARTIST_FILTER,		//>>> WiZaRd::Artist Filter
	SDI_UNSHAREDDIRECTORY,
	SDI_FILESYSTEMPARENT
};

class CSharedFilesCtrl;
class CKnownFile;

//**********************************************************************************
// CDirectoryItem

class CDirectoryItem{
public:
//>>> WiZaRd::Artist Filter
//	CDirectoryItem(CString strFullPath, HTREEITEM htItem = TVI_ROOT, ESpecialDirectoryItems eItemType = SDI_NO, int m_nCatFilter = -1);
	CDirectoryItem(CString strFullPath, HTREEITEM htItem = TVI_ROOT, ESpecialDirectoryItems eItemType = SDI_NO, int m_nCatFilter = -1, const ESpecialDirectoryItems eParentType = SDI_NO); 
//<<< WiZaRd::Artist Filter
	~CDirectoryItem();
//>>> WiZaRd::Artist Filter
//	CDirectoryItem*		CloneContent() const { return new CDirectoryItem(m_strFullPath, 0, m_eItemType, m_nCatFilter); }
	CDirectoryItem*		CloneContent() const { return new CDirectoryItem(m_strFullPath, 0, m_eItemType, m_nCatFilter, m_eParentType); }
//<<< WiZaRd::Artist Filter
	HTREEITEM			FindItem(CDirectoryItem* pContentToFind) const;

	CString		m_strFullPath;
	HTREEITEM	m_htItem;
	int			m_nCatFilter;
	CList<CDirectoryItem*, CDirectoryItem*> liSubDirectories;
	ESpecialDirectoryItems m_eItemType;
	ESpecialDirectoryItems m_eParentType; //>>> WiZaRd::Artist Filter
};

//**********************************************************************************
// CSharedDirsTreeCtrl

class CSharedDirsTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CSharedDirsTreeCtrl)

public:
	CSharedDirsTreeCtrl();
	virtual ~CSharedDirsTreeCtrl();
	
	void			Initalize(CSharedFilesCtrl* pSharedFilesCtrl);
	void			SetAllIcons();

	CDirectoryItem* GetSelectedFilter() const;
	bool			IsCreatingTree() const		{return m_bCreatingTree;};
	void			Localize();
	void			EditSharedDirectories(CDirectoryItem* pDir, bool bAdd, bool bSubDirectories);
	void			Reload(bool bFore = false);

	CDirectoryItem*		pHistory; //Xman [MoNKi: -Downloaded History-]

protected:
	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	void			CreateMenues();
	void			ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0);
	void			DeleteChildItems(CDirectoryItem* pParent);
	void			AddSharedDirectory(CString strDir, bool bSubDirectories);
	void			RemoveSharedDirectory(CString strDir, bool bSubDirectories);
	int				AddSystemIcon(HICON hIcon, int nSystemListPos);
	void			FetchSharedDirsList();

	DECLARE_MESSAGE_MAP()
	afx_msg void	OnSysColorChange();
	afx_msg void	OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg	void	OnRButtonDown(UINT nFlags, CPoint point );
	afx_msg	void	OnLButtonUp(UINT nFlags, CPoint point );
	afx_msg void	OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnCancelMode();
	afx_msg LRESULT OnFoundNetworkDrive(WPARAM wParam,LPARAM lParam);	// SLUGFILLER: shareSubdir - Multi-threading

	CTitleMenu			m_SharedFilesMenu;
	CTitleMenu			m_ShareDirsMenu;
	CMenu				m_PrioMenu;
	CDirectoryItem*		m_pRootDirectoryItem;
	CDirectoryItem*		m_pRootUnsharedDirectries;
	CDirectoryItem*		m_pDraggingItem;
	CSharedFilesCtrl*	m_pSharedFilesCtrl;
	CStringList			m_strliSharedDirs;
	CStringList			m_strliSharedDirsSubdir;	// SLUGFILLER: shareSubdir - second list
	CStringList			m_strliCatIncomingDirs;
	CImageList			m_imlTree;

private:
	void	InitalizeStandardItems();
	
	void	FileSystemTreeCreateTree();
	void	FileSystemTreeAddChildItem(CDirectoryItem* pRoot, CString strText, bool bTopLevel);
	bool	FileSystemTreeHasSubdirectories(CString strDir);
	bool	FileSystemTreeHasSharedSubdirectory(CString strDir);
	void	FileSystemTreeAddSubdirectories(CDirectoryItem* pRoot);
  /* old code sharesubdir
	bool	FileSystemTreeIsShared(CString strDir);
  */
	bool	FileSystemTreeIsShared(CString strDir, bool bCheckParent = false, bool bCheckIncoming = false,bool bOnlySubdir = false);	// SLUGFILLER: shareSubdir - allow checking indirect share
	void	FileSystemTreeUpdateBoldState(CDirectoryItem* pDir = NULL);
	void	FileSystemTreeSetShareState(CDirectoryItem* pDir, bool bShared, bool bSubDirectories);

	void	FilterTreeAddSharedDirectory(CDirectoryItem* pDir, bool bRefresh);
	void	FilterTreeAddSubDirectories(CDirectoryItem* pDirectory, CStringList& liDirs, int nLevel = 0);
	bool	FilterTreeIsSubDirectory(CString strDir, CString strRoot, CStringList& liDirs);
//	void	FilterTreeReloadTree(); //>>> WiZaRd::Artist Filter


	bool			m_bCreatingTree;
	bool			m_bUseIcons;
	CMap<int, int, int, int> m_mapSystemIcons;
	// SLUGFILLER START: shareSubdir - multi-threaded magic
	static UINT EnumNetworkDrivesThreadProc(LPVOID pParam);
	void EnumNetworkDrives(NETRESOURCE *source);
	// SLUGFILLER END: shareSubdir
 // SLUGFILLER START: shareSubdir - locks and lists
	CStringList		NetworkDrives;
	CCriticalSection NetworkDrivesLock;
	CEvent*			NetworkThreadOffline;
	bool			NetworkThreadRun;
	// SLUGFILLER END: shareSubdir
//>>> WiZaRd::Artist Filter
public:
	void	FilterTreeReloadTree();
//<<< WiZaRd::Artist Filter
};


