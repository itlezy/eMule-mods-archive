//this file is part of eMule
// added by itsonlyme
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

// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->

#pragma once
#include "ResizableLib/ResizablePage.h"
#include "Neo/GUI/CP/TreeOptionsCtrl.h" // NEO: FIX - [TreeControl] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "Neo\GUI\ToolTips\PPToolTip.h"
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

class CUpDownClient;
struct BLCItem_struct;

//////////////////////////////////////////////////////////////////////////////
// CSharedFilesPage dialog

class CSharedFilesPage : public CResizablePage
{
	DECLARE_DYNAMIC(CSharedFilesPage)

public:
	CSharedFilesPage();   // standard constructor
	virtual ~CSharedFilesPage ();

	void SetClients(const CSimpleArray<CObject*>* paClients) { m_paClients = paClients; m_bDataChanged = true; }
	void SetLocalFiles() { m_bLocalFiles = true; m_bDataChanged = true; }
	void UpdateTree(CString newDir);
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	void SetTTDelay();
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

// Dialog Data
	enum { IDD = IDD_BROWSEFILES };

protected:
	CString m_strCaption;
	CTreeOptionsCtrl m_ctrlTree; // NEO: FIX - [TreeControl]
	const CSimpleArray<CObject*>* m_paClients;
	bool m_bDataChanged;
	uint32 m_timer;

	CString m_strToolTip;
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	CToolTipCtrl *m_toolTip;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	int m_iImgRoot;
	int m_iImgShDir;
	int m_iImgDir;
	bool m_bLocalFiles;

	CTypedPtrList<CPtrList, BLCItem_struct*> m_BLCItem_list;
	CRBMap<CString, HTREEITEM> m_HTIs_map;
	CRBMap<CString, bool> m_dirsList;

	CMap<int, int, int, int> m_iconMap;

	HTREEITEM m_htiRoot;
	HTREEITEM m_htiOldToolTipItemDown;

	void FillTree();
	void GetDirs();
	int GetFileIcon(CString filename);
	void SortDirs(HTREEITEM hParent);
	bool FilterDirs(CRBMap<CString, bool> *listDirs, HTREEITEM hParent);
	void Localize();
	HTREEITEM GetItemUnderMouse();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnSetActive();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	virtual BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnExtOptsDblClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnItemExpanding (NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnExtOptsRightClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); // NEO: NMX - [NeoMenuXP] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);

private:

	CPPToolTip m_ttip;
	//CPPToolTip m_othertips;
	int GetTabUnderMouse(CPoint* point);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
};

// NEO: XSF END <-- Xanatos --