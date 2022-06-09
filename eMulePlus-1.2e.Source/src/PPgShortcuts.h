//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

#include "KeyboardShortcut.h"

class CPreferences;

typedef struct
{
	HTREEITEM hHnd;
	CKeyboardShortcut Shortcut;
} ShortcutTreeItem;

enum
{
	SCUT_NODE_GEN,
	SCUT_NODE_GEN_WIN,
	SCUT_NODE_GEN_LINK,
	SCUT_NODE_GEN_FILES,
	SCUT_NODE_DL,
	SCUT_NODE_DL_ADV,
	SCUT_NODE_SRC,
	SCUT_NODE_SHARED,

	SCUT_NODES
};

class CPPgShortcuts : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgShortcuts)

public:
	CPPgShortcuts();
	virtual ~CPPgShortcuts();

	enum { IDD = IDD_PPG_SHORTCUTS };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void CreateShortcutsTree(void);
	void RefreshShortcutsTree(void);
	void RefreshKeyCombo(void);
	void SetPrefs(CPreferences* pPrefs)	{ m_pPrefs = pPrefs; }
	void Localize(void);
	int  GetKeyComboIndexFromShortcutKey(byte byteKey) const;
	byte GetShortcutKeyFromKeyComboIndex(int iIndex) const;
	int  GetShortcutTreeItemIndexFromItem(HTREEITEM hItem);
	afx_msg void OnSettingsChange(void);
	afx_msg void OnEnableShortcutClick(void);
	afx_msg void OnDestroy();
	afx_msg void OnTvnSelchangingShortcutsTree(NMHDR *pNMHDR, LRESULT *pResult);
	void UpdateCheckBoxesState(void);

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);
	void LoadShortcut(HTREEITEM item);
	void SaveShortcut(HTREEITEM item);

protected:
	CPreferences *m_pPrefs;
	BOOL	m_bModified;
	BOOL	m_bShortcutsEnable;
	BOOL	m_bShortcutsAlt;
	BOOL	m_bShortcutsCtrl;
	BOOL	m_bShortcutsShift;
	BOOL	m_bShortcutsWin;

private:
	CImageList m_ImagelistShortcutsTree;
	ShortcutTreeItem m_ShortcutTreeItem[SCUT_COUNT];
	HTREEITEM m_ahTree[SCUT_NODES];
	CComboBox m_ShortcutsKeyCombo;
	CTreeCtrl m_ShortcutsTree;
};
