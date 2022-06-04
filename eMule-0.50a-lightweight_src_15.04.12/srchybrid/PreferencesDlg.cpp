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
#include "PreferencesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPreferencesDlg, CTreePropSheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CModTreePropSheet) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	CTreePropSheet::SetPageIcon(&m_wndGeneral, _T("Preferences"));
	CTreePropSheet::SetPageIcon(&m_wndDisplay, _T("DISPLAY"));
	CTreePropSheet::SetPageIcon(&m_wndConnection, _T("CONNECTION"));
	CTreePropSheet::SetPageIcon(&m_wndProxy, _T("KADSERVER"));
	CTreePropSheet::SetPageIcon(&m_wndDirectories, _T("FOLDERS"));
	CTreePropSheet::SetPageIcon(&m_wndFiles, _T("Transfer"));
	CTreePropSheet::SetPageIcon(&m_wndSecurity, _T("SECURITY"));
	CTreePropSheet::SetPageIcon(&m_wndTweaks, _T("TWEAK"));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CTreePropSheet::SetPageIcon(&m_wndDebug, _T("Preferences"));
#endif

	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndProxy);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndSecurity);
	AddPage(&m_wndTweaks);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif

	// The height of the option dialog is already too large for 640x480. To show as much as
	// possible we do not show a page caption (which is an decorative element only anyway).
	//SetTreeViewMode(TRUE, GetSystemMetrics(SM_CYSCREEN) >= 600, FALSE); 
	SetTreeViewMode(TRUE, GetSystemMetrics(SM_CYSCREEN) <= 1, TRUE); 
	SetTreeWidth(170);

	m_pPshStartPage = NULL;
	m_bSaveIniFile = false;
}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
	CModTreePropSheet::OnDestroy();
	if (m_bSaveIniFile)
	{
		thePrefs.Save();
		m_bSaveIniFile = false;
	}
	m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
}

BOOL CPreferencesDlg::OnInitDialog()
{
	ASSERT( !m_bSaveIniFile );
	BOOL bResult = CModTreePropSheet::OnInitDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	InitWindowStyles(this);

	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		if (GetPage(i)->m_psp.pszTemplate == m_pPshStartPage)
		{
			SetActivePage(i);
			break;
		}
	}


	Localize();	
	return bResult;
}

void CPreferencesDlg::Localize()
{
	CString title = RemoveAmbersand(GetResString(IDS_EM_PREFS));
	if(thePrefs.prefReadonly)// X: [ROP] - [ReadOnlyPreference]
		title.AppendFormat(_T(" [%s]"), GetResString(IDS_READONLY));
	SetTitle(title); 

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndProxy.Localize();

	int c = 0;

	CTreeCtrl* pTree = GetPageTreeControl();
	if (pTree)
	{
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_GENERAL)/*)*/);
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_DISPLAY)/*)*/); 
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_CONNECTION)/*)*/); 
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_SERVER)/*)*/); 
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_DIR)/*)*/); 
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_FILES)/*)*/); 
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_SECURITY)/*)*/); 
		pTree->SetItemText(GetPageTreeItem(c++), /*RemoveAmbersand(*/GetResString(IDS_PW_TWEAK)/*)*/);
	#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		pTree->SetItemText(GetPageTreeItem(c++), _T("Debug"));
	#endif
	}
	SetDlgItemText(IDOK, GetResString(IDS_TREEOPTIONS_OK));// X: [AL] - [Additional Localize]
	SetDlgItemText(IDCANCEL, GetResString(IDS_CANCEL));
	SetDlgItemText(ID_APPLY_NOW, GetResString(IDS_PW_APPLY));

	UpdateCaption();
}

BOOL CPreferencesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDOK || wParam == ID_APPLY_NOW)
		m_bSaveIniFile = true;
	return __super::OnCommand(wParam, lParam);
}

void CPreferencesDlg::SetStartPage(UINT uStartPageID)
{
	m_pPshStartPage = MAKEINTRESOURCE(uStartPageID);
}
