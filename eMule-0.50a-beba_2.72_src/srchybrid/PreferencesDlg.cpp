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


IMPLEMENT_DYNAMIC(CPreferencesDlg, CModelessTreePropSheet)	// Tux: Feature: Modeless dialogs

BEGIN_MESSAGE_MAP(CPreferencesDlg, CModelessTreePropSheet)	// Tux: Feature: Modeless dialogs
	ON_WM_DESTROY()
	// Tux: LiteMule: Remove Help
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndStats.m_psp.dwFlags &= ~PSH_HASHELP;
	// Tux: LiteMule: Remove IRC
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	// Tux: LiteMule: Remove Scheduler
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndMessages.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndBeba.m_psp.dwFlags &= ~PSH_HASHELP;	// Tux: others: beba prefs
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	CTreePropSheet::SetPageIcon(&m_wndGeneral, _T("GENOPTS"));	// Tux: changed icon
	CTreePropSheet::SetPageIcon(&m_wndDisplay, _T("DISPLAY"));
	CTreePropSheet::SetPageIcon(&m_wndConnection, _T("CONNECTION"));
	CTreePropSheet::SetPageIcon(&m_wndProxy, _T("PROXY"));
	CTreePropSheet::SetPageIcon(&m_wndServer, _T("SERVEROPTS"));	// Tux: changed icon
	CTreePropSheet::SetPageIcon(&m_wndDirectories, _T("FOLDERS"));
	CTreePropSheet::SetPageIcon(&m_wndFiles, _T("SHAREDOPTS"));	// Tux: changed icon
	CTreePropSheet::SetPageIcon(&m_wndNotify, _T("NOTIFICATIONS"));
	CTreePropSheet::SetPageIcon(&m_wndStats, _T("STATOPTS"));	// Tux: changed icon
	// Tux: LiteMule: Remove IRC
	CTreePropSheet::SetPageIcon(&m_wndSecurity, _T("SECURITY"));
	// Tux: LiteMule: Remove Scheduler
	CTreePropSheet::SetPageIcon(&m_wndWebServer, _T("WEB"));
	CTreePropSheet::SetPageIcon(&m_wndTweaks, _T("TWEAK"));
	CTreePropSheet::SetPageIcon(&m_wndMessages, _T("MESSAGESOPTS"));	// Tux: changed icon
	CTreePropSheet::SetPageIcon(&m_wndBeba, _T("BEBA"));	// Tux: others: beba prefs
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CTreePropSheet::SetPageIcon(&m_wndDebug, _T("TWEAK"));	// Tux: changed icon
#endif

	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndProxy);
	AddPage(&m_wndServer);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndNotify);
	AddPage(&m_wndStats);
	// Tux: LiteMule: Remove IRC
	AddPage(&m_wndMessages);
	AddPage(&m_wndSecurity);
	// Tux: LiteMule: Remove Scheduler
	AddPage(&m_wndWebServer);
	AddPage(&m_wndTweaks);
	AddPage(&m_wndBeba);	// Tux: others: beba prefs
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif

	// The height of the option dialog is already too large for 640x480. To show as much as
	// possible we do not show a page caption (which is an decorative element only anyway).
	SetTreeViewMode(TRUE, GetSystemMetrics(SM_CYSCREEN) >= 600, TRUE);
	SetTreeWidth(170);

	m_pPshStartPage = NULL;
	m_bSaveIniFile = false;
}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
	CTreePropSheet::OnDestroy();
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
	BOOL bResult = CModelessTreePropSheet::OnInitDialog();	// Tux: Feature: Modeless dialogs: changed line
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
	SetTitle(RemoveAmbersand(GetResString(IDS_EM_PREFS))); 

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndServer.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndStats.Localize();
	m_wndNotify.Localize();
	// Tux: LiteMule: Remove IRC
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndWebServer.Localize();
	// Tux: LiteMule: Remove Scheduler
	m_wndProxy.Localize();
	m_wndMessages.Localize();
	m_wndBeba.Localize();	// Tux: others: beba prefs
	// int c = 0;		// Tux: Improvement: replaced enumeration by GetPageIndex

	CTreeCtrl* pTree = GetPageTreeControl();
	if (pTree)
	{
		// Tux: Improvement: replaced enumeration by GetPageIndex [start]
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndGeneral)), RemoveAmbersand(GetResString(IDS_PW_GENERAL)));
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndDisplay)), RemoveAmbersand(GetResString(IDS_PW_DISPLAY))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndConnection)), RemoveAmbersand(GetResString(IDS_PW_CONNECTION))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndProxy)), RemoveAmbersand(GetResString(IDS_PW_PROXY)));
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndServer)), RemoveAmbersand(GetResString(IDS_PW_SERVER))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndDirectories)), RemoveAmbersand(GetResString(IDS_PW_DIR))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndFiles)), RemoveAmbersand(GetResString(IDS_PW_FILES))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndNotify)), RemoveAmbersand(GetResString(IDS_PW_EKDEV_OPTIONS))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndStats)), RemoveAmbersand(GetResString(IDS_STATSSETUPINFO))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndMessages)), RemoveAmbersand(GetResString(IDS_MESSAGESCOMMENTS)));
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndSecurity)), RemoveAmbersand(GetResString(IDS_SECURITY))); 
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndWebServer)), RemoveAmbersand(GetResString(IDS_PW_WS)));
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndTweaks)), RemoveAmbersand(GetResString(IDS_PW_TWEAK)));
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndBeba)), RemoveAmbersand(GetResString(IDS_PW_BEBA)));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		pTree->SetItemText(GetPageTreeItem(GetPageIndex(&m_wndDebug)), _T("Debug"));
#endif
		// Tux: Improvement: replaced enumeration by GetPageIndex [end]
	}

	UpdateCaption();
}

// Tux: LiteMule: Remove Help: removed code block

BOOL CPreferencesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// Tux: LiteMule: Remove Help: removed code block

	if (wParam == IDOK || wParam == ID_APPLY_NOW)
		m_bSaveIniFile = true;
	return __super::OnCommand(wParam, lParam);
}

// Tux: LiteMule: Remove Help: removed code block

void CPreferencesDlg::SetStartPage(UINT uStartPageID)
{
	m_pPshStartPage = MAKEINTRESOURCE(uStartPageID);
}
