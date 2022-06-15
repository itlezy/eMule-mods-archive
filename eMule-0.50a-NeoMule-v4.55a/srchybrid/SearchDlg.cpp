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
#include "emuleDlg.h"
#include "SearchDlg.h"
#include "SearchParamsWnd.h"
#include "SearchResultsWnd.h"
#include "OtherFunctions.h"
#include "HelpIDs.h"
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "SearchFile.h"
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	IDBAR_SEARCH_PARAMS		(AFX_IDW_CONTROLBAR_FIRST + 32 + 1)	// do NOT change that ID, if not absolutely needed (it is stored by MFC in the bar profile!)
#define	SEARCH_PARAMS_PROFILE	_T("SearchFrmBarState")

IMPLEMENT_DYNCREATE(CSearchDlg, CFrameWnd)

BEGIN_MESSAGE_MAP(CSearchDlg, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_WM_HELPINFO()
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
END_MESSAGE_MAP()

CSearchDlg::CSearchDlg()
{
	m_pwndParams = new CSearchParamsWnd;
	m_pwndResults = NULL;
}

CSearchDlg::~CSearchDlg()
{
	delete m_pwndParams;
}

BOOL CSearchDlg::Create(CWnd* pParent)
{
	// *) The initial size of that frame window must not exceed the window size of the
	//    dialog resource template of the client window (the search results window).
	// *) The dialog resource template's window size (of the search results window) must not 
	//	  exceed the minimum client area of the frame window.
	// Otherwise we may get scrollbars in the search results window
	CRect rc(0, 0, 50, 50);
	return CFrameWnd::Create(NULL, _T("Search"), WS_CHILD, rc, pParent, NULL, 0, NULL);
}

int CSearchDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CCreateContext context;
	context.m_pCurrentFrame = this;
	context.m_pCurrentDoc = NULL;
	context.m_pNewViewClass = RUNTIME_CLASS(CSearchResultsWnd);
	context.m_pNewDocTemplate = NULL;
	m_pwndResults = (CSearchResultsWnd*)CreateView(&context);
	m_pwndParams->m_searchdlg = m_pwndResults;
	m_pwndResults->ModifyStyle(WS_BORDER, 0);
	m_pwndResults->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);

	m_pwndParams->Create(this, IDD_SEARCH_PARAMS, 
						 WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_SIZE_FIXED | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER, 
						 IDBAR_SEARCH_PARAMS);
	m_pwndParams->SetWindowText(GetResString(IDS_SEARCHPARAMS));
	m_pwndParams->EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(m_pwndParams, AFX_IDW_DOCKBAR_TOP, (LPRECT)NULL);

	m_pwndResults->m_pwndParams = m_pwndParams;
	m_pwndResults->SendMessage(WM_INITIALUPDATE);

	LoadBarState(SEARCH_PARAMS_PROFILE);
	DockParametersWnd(); // Too much bug reports about vanished search parameters window. Force to dock.
	ShowControlBar(m_pwndParams, TRUE, TRUE);
	Localize();

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.Create(this);
	m_ttip.AddTool(&m_pwndResults->searchlistctrl, _T(""));
	SetTTDelay();
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	return 0;
}

void CSearchDlg::OnClose()
{
	SaveBarState(SEARCH_PARAMS_PROFILE);
	CFrameWnd::OnClose();
}

void CSearchDlg::DeleteAllSearchListCtrlItems()
{
	m_pwndResults->searchlistctrl.DeleteAllItems();
}

void CSearchDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	if (m_pwndParams->IsFloating())
	{
		//ShowControlBar(m_pwndParams, bShow, TRUE);
		DockParametersWnd(); // Too much bug reports about vanished search parameters window. Force to dock.
	}
}

void CSearchDlg::OnSetFocus(CWnd* pOldWnd)
{
	CFrameWnd::OnSetFocus(pOldWnd);
	if (m_pwndParams->m_hWnd)
		m_pwndParams->SetFocus();
}

void CSearchDlg::SaveAllSettings()
{
	m_pwndParams->SaveSettings(); 
}

void CSearchDlg::ResetHistory()
{
	m_pwndParams->ResetHistory();
}

BOOL CSearchDlg::IsSearchParamsWndVisible() const
{
	return m_pwndParams->IsWindowVisible();
}

void CSearchDlg::OpenParametersWnd()
{
	ShowControlBar(m_pwndParams, TRUE, TRUE);
}

void CSearchDlg::DockParametersWnd()
{
	if (m_pwndParams->IsFloating())
	{
		UINT uMRUDockID = AFX_IDW_DOCKBAR_TOP;
		if (m_pwndParams->m_pDockContext)
			uMRUDockID = m_pwndParams->m_pDockContext->m_uMRUDockID;
		DockControlBar(m_pwndParams, uMRUDockID);
	}
}

int CSearchDlg::GetSelectedCat()
{
	return m_pwndResults->GetSelectedCat();
}

void CSearchDlg::UpdateCatTabs()
{
	m_pwndResults->UpdateCatTabs();
}

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
void CSearchDlg::SetToolTipsDelay(UINT uDelay)
{
	CToolTipCtrl* tooltip = m_pwndResults->searchlistctrl.GetToolTips();
	if (tooltip)
		tooltip->SetDelayTime(TTDT_INITIAL, uDelay);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

void CSearchDlg::RemoveResult(const CSearchFile* pFile)
{
	m_pwndResults->searchlistctrl.RemoveResult(pFile);
}

bool CSearchDlg::CreateNewTab(SSearchParams* pParams, bool bActiveIcon)
{
	return m_pwndResults->CreateNewTab(pParams, bActiveIcon);
}

SSearchParams* CSearchDlg::GetSearchParamsBySearchID(uint32 nSearchID){
	return m_pwndResults->GetSearchResultsParams(nSearchID);
}

void CSearchDlg::LocalEd2kSearchEnd(UINT nCount, bool bMoreResultsAvailable)
{
	m_pwndResults->LocalEd2kSearchEnd(nCount, bMoreResultsAvailable);
}

void CSearchDlg::CancelEd2kSearch()
{
	m_pwndResults->CancelEd2kSearch();
}

void CSearchDlg::CancelKadSearch(UINT uSearchID)
{
	m_pwndResults->CancelKadSearch(uSearchID);
}

void CSearchDlg::SetNextSearchID(uint32 uNextID)
{ 
	m_pwndResults->SetNextSearchID(uNextID); 
}

void CSearchDlg::AddGlobalEd2kSearchResults(UINT nCount)
{
	m_pwndResults->AddGlobalEd2kSearchResults(nCount);
}

void CSearchDlg::Localize()
{
	m_pwndResults->Localize();
	m_pwndParams->Localize();
}

void CSearchDlg::CreateMenus()
{
	m_pwndResults->searchlistctrl.CreateMenues();
}

bool CSearchDlg::DoNewKadSearch(SSearchParams* pParams)
{
	return m_pwndResults->DoNewKadSearch(pParams);
}

bool CSearchDlg::DoNewEd2kSearch(SSearchParams* pParams)
{
	return m_pwndResults->DoNewEd2kSearch(pParams);
}

void CSearchDlg::DeleteAllSearches()
{
	m_pwndResults->DeleteAllSearches();
}

bool CSearchDlg::CanDeleteSearch(uint32 nSearchID) const
{
	return m_pwndResults->CanDeleteSearch(nSearchID);
}

bool CSearchDlg::CanDeleteAllSearches() const
{
	return m_pwndResults->CanDeleteAllSearches();
}

void CSearchDlg::DeleteSearch(uint32 nSearchID)
{
	m_pwndResults->DeleteSearch(nSearchID);
}

void CSearchDlg::DownloadSelected(bool bPaused)
{
	m_pwndResults->DownloadSelected(bPaused);
}

void CSearchDlg::DownloadSelected()
{
	m_pwndResults->DownloadSelected();
}

CClosableTabCtrl& CSearchDlg::GetSearchSelector()
{
	return m_pwndResults->searchselect;
}

void CSearchDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_KEYMENU)
	{
		if (lParam == EMULE_HOTMENU_ACCEL)
			theApp.emuledlg->SendMessage(WM_COMMAND, IDC_HOTMENU);
		else
			theApp.emuledlg->SendMessage(WM_SYSCOMMAND, nID, lParam);
		return;
	}
	CFrameWnd::OnSysCommand(nID, lParam);
}

void CSearchDlg::UpdateSearch(CSearchFile* pSearchFile)
{
	if (m_pwndResults)
		m_pwndResults->searchlistctrl.UpdateSearch(pSearchFile);
}

bool CSearchDlg::CanSearchRelatedFiles() const
{
	return m_pwndResults->CanSearchRelatedFiles();
}

//void CSearchDlg::SearchRelatedFiles(CPtrList& listFiles);
void CSearchDlg::SearchRelatedFiles(CPtrList& listFiles, bool bHash) // NEO: MOD - [SearchHash] <-- Xanatos --
{
	//m_pwndResults->SearchRelatedFiles(listFiles);
	m_pwndResults->SearchRelatedFiles(listFiles, bHash); // NEO: MOD - [SearchHash] <-- Xanatos --
}

BOOL CSearchDlg::PreTranslateMessage(MSG* pMsg)
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.RelayEvent(pMsg);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			// UGLY: Because this window is a 'CFrameWnd' (rather than a 'CDialog' like
			// the other eMule main windows) we can not use MFC's message routing. Need
			// to explicitly send that message to the main window.
			theApp.emuledlg->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
		else if (pMsg->wParam == 87 && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			m_pwndResults->DeleteSelectedSearch();
		}
	}
	return CFrameWnd::PreTranslateMessage(pMsg);
}

BOOL CSearchDlg::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	theApp.ShowHelp(eMule_FAQ_GUI_Search);
	return TRUE;
}

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
BOOL CSearchDlg::OnToolTipNotify(UINT /*id*/, NMHDR *pNMH, LRESULT* /*pResult*/)
{
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	switch(control_id)
	{
		case IDC_SEARCHLIST:
		{
			CSearchFile* file;
			if (m_pwndResults->searchlistctrl.GetItemCount() < 1)
				return FALSE;

			int sel = m_pwndResults->searchlistctrl.GetItemUnderMouse();
			if (sel < 0)
				return FALSE;

			file = (CSearchFile*)m_pwndResults->searchlistctrl.GetItemData(sel);
			if (!file)
				return FALSE;

			SHFILEINFO shfi;
			ZeroMemory(&shfi, sizeof(shfi));
			SHGetFileInfo(file->GetFileName(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_ICON|SHGFI_USEFILEATTRIBUTES);

			CString info;
			file->GetTooltipFileInfo(info);

			pNotify->ti->hIcon = shfi.hIcon;
			pNotify->ti->sTooltip = info;

			SetFocus();

			return TRUE;
		}
		default:
			if(pNotify->ti->hIcon)
				pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
			return TRUE;
	}
}

void CSearchDlg::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --