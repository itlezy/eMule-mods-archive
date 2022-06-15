//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

// NEO: FCFG - [FileConfiguration] -- Xanatos -->

#include "Neo/GUI/CP/KCSideBannerWnd.h" // NEO: NPB - [PrefsBanner]

#pragma once
#include "ResizableLib/ResizablePage.h"
#include "ResizableLib/ResizableSheet.h"
#include "emule.h"
#include "FileInfoDialog.h"
#include "CommentDialogLst.h"
#include "MetaDataDlg.h"
#include "MuleListCtrl.h"
#include "ED2kLinkDlg.h"
#include "ListBoxST.h"
#include "Neo\Gui\ModeLess.h" // NEO: MLD - [ModelesDialogs]

#include "Neo\Cp\PPgRelease.h"
#include "Neo\Cp\PPgSources.h" 
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
#include "Neo/CP/PPgSourceStorage.h"
#endif // NEO: NSS END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
#include "PPgLancast.h"
#endif //LANCAST // NEO: NLC END

class CSearchFile;
struct Category_Struct;

///////////////////////////////////////////////////////////////////////////////
// CFilePreferencesDialog

#define IDC_PREFS_LISTBOX 222

class CFilePreferencesDialog : public CModListViewWalkerPreferenceSheet // NEO: MLD - [ModelesDialogs]
{
	DECLARE_DYNAMIC(CFilePreferencesDialog)

public:
	CFilePreferencesDialog(const CSimpleArray<CKnownFile*>* paFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	CFilePreferencesDialog(Category_Struct* category, bool shared = false);
	
	virtual ~CFilePreferencesDialog();

protected:
	CPPgRelease		m_wndRelease;
	CPPgSources		m_wndSources;
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	CPPgSourceStorage m_wndSourceStorage;
#endif // NEO: NSS END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	CPPgLancast		m_wndLancast;
#endif //LANCAST // NEO: NLC END

	CListBoxST		m_listbox;
	CButton			m_groupbox;
	CImageList		ImageList;
	int				m_iPrevPage;

	Category_Struct* m_Category;
	bool			m_bShared;

	void Localize();
	void OpenPage(UINT uResourceID);

	//UINT m_uPshInvokePage;
	//static LPCTSTR m_pPshStartPage;

	void UpdateTitle();

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
	afx_msg void OnSelChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	// NEO: FCFG - [FileConfiguration]
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnReset();
	// NEO: FCFG END
	afx_msg void OnBnClickedOk();

	UINT m_nActiveWnd;

	CKCSideBannerWnd m_banner; // NEO: NPB - [PrefsBanner]

private:
	// NEO: FCFG - [FileConfiguration]
	void	GetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs);
	void	SetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs);
	// NEO: FCFG END
};

// NEO: FCFG END <-- Xanatos --