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
#pragma once
#include "MuleListCtrl.h"
#include "TitleMenu.h"
#include "ListCtrlItemWalk.h"
#include "Neo/GUI/FileProcessing.h"// NEO: CRC - [MorphCRCTag] <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

class CSharedFileList;
class CKnownFile;
class CDirectoryItem;
class CToolTipCtrlX;

// NEO: SP - [SharedParts] -- Xanatos -->
class CSharedFileList;
class CKnownFile;
class CSharedFilesCtrl;

///////////////////////////////////////////////////////////////////////////////
// CSharedItem

class CSharedItem: public CObject
{
	DECLARE_DYNAMIC(CSharedItem)
public:

	bool		isFile;
	bool		isOpen;
	uint16		Part;
	uint16		Parts;
	CKnownFile	*KnownFile;
};

///////////////////////////////////////////////////////////////////////////////
// CSharedFilesListCtrlItemWalk

class CSharedFilesListCtrlItemWalk : public CListCtrlItemWalk
{
public:
	CSharedFilesListCtrlItemWalk(CSharedFilesCtrl* pListCtrl);

	virtual CObject* GetNextSelectableItem();
	virtual CObject* GetPrevSelectableItem();

	void SetItemTypeParts(bool Parts) { m_iParts = Parts; }

protected:
	CSharedFilesCtrl* m_pSharedFilesListCtrl;
	bool m_iParts;
};
// NEO: SP END <-- Xanatos --

///////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl

class CSharedFilesCtrl : public CMuleListCtrl, public CSharedFilesListCtrlItemWalk // NEO: SP - [SharedParts] <-- Xanatos --
//class CSharedFilesCtrl : public CMuleListCtrl, public CListCtrlItemWalk
{
	friend class CSharedDirsTreeCtrl;
	DECLARE_DYNAMIC(CSharedFilesCtrl)

public:
	CSharedFilesCtrl();
	virtual ~CSharedFilesCtrl();

	void	Init();
	void	CreateMenues();
	void	ReloadFileList();
	// NEO: AKF - [AllKnownFiles] -- Xanatos -->
	void	ChangeView(UINT view = 0); 
	bool	IsKnownList()	{ return m_ShowAllKnow; }
	// NEO: AKF END <-- Xanatos --
	void	AddFile(const CKnownFile* file);
	void	RemoveFile(const CKnownFile* file);
	void	UpdateFile(const CKnownFile* file);
	// NEO: SP - [SharedParts] -- Xanatos -->
	void	UpdatePart(const CSharedItem* part); 
	BOOL	DeleteAllItems();
	// NEO: SP END -- Xanatos --
	void	SetColoring(int mode); // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	void	Localize();
	//void	ShowFilesCount();
	void	ShowFilesCount(DWORD Percentage = 0); // NEO: MOD - [HashProgress] <-- Xanatos --
	void	ShowComments(CKnownFile* file);
	void	SetAICHHashing(uint32 nVal)				{ nAICHHashing = nVal; }
	void	SetDirectoryFilter(CDirectoryItem* pNewFilter, bool bRefresh = true);

	CFileProcessingThread m_FileProcessingThread; // NEO: CRC - [MorphCRCTag] <-- Xanatos --

protected:
	// NEO: NMX - [NeoMenuXP] -- Xanatos -->
	CMenuXP		m_SharedFilesMenu;
	CMenuXP		m_CollectionsMenu;
	CMenuXP		m_CRC32Menu; // NEO: CRC - [MorphCRCTag] <-- Xanatos --
	CTitleMenu	m_PrioMenu;
	CTitleMenu	m_PermMenu; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	CMenuXP		m_PWProtMenu; // NEO: PP - [PasswordProtection] <-- Xanatos --
	CMenuXP		m_VirtualDirMenu; // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
	CMenuXP		m_ReleaseMenu; // NEO: MOD - [ReleaseMenu] <-- Xanatos --
	// NEO: NMX END <-- Xanatos --
	bool			sortstat[4];
	CImageList		m_ImageList;
	CDirectoryItem*	m_pDirectoryFilter;
	volatile uint32 nAICHHashing;
	CToolTipCtrlX*	m_pToolTip;
	bool			m_ShowAllKnow; // NEO: AKF - [AllKnownFiles] <-- Xanatos --

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	// NEO: SP - [SharedParts] -- Xanatos -->
	static int Compare(const CKnownFile* item1, const CKnownFile* item2, LPARAM lParamSort);
	static int Compare(const CSharedItem* item1, const CSharedItem* item2, LPARAM lParamSort);
	// NEO: SP END <-- Xanatos --

	void OpenFile(const CKnownFile* file);
	//void ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0);
	void ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, BOOL Preferences = FALSE); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	void SetAllIcons();
	int FindFile(const CKnownFile* pFile);
	int FindPart(const CSharedItem* pPart); // NEO: SP - [SharedParts] <-- Xanatos --
	void GetItemDisplayText(const CKnownFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const;
	bool IsFilteredItem(const CKnownFile* pKnownFile) const;

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// NEO: SP - [SharedParts] -- Xanatos -->
	void ExpandCollapseItem(int Item);
	void DrawFileItem(CDC *dc, int iColumn, LPRECT cur_rec, CKnownFile *file);
	void DrawPartItem(CDC *dc, int iColumn, LPRECT cur_rec, CSharedItem *part);
	// NEO: SP END <-- Xanatos --

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClick(NMHDR *pNMHDR, LRESULT *pResult); // NEO: NTS - [NeoTreeStyle] <-- Xanatos --
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	afx_msg void OnBegindragFilelist(NMHDR* pNMHDR, LRESULT* pResult); // NEO: SDD - [ShareDargAndDrop] <-- Xanatos --
	// NEO: CRC - [MorphCRCTag] -- Xanatos -->
	afx_msg LRESULT OnCRC32RenameFile (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCRC32UpdateFile (WPARAM wParam, LPARAM lParam);
	// NEO: CRC END <-- Xanatos --

private:
	// Pointer to the function that contain the color schema for part traffic bars
	COLORREF(*GetTrafficColor)(float); // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
};
