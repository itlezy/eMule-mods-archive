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
#include "FileDetailDialog.h"
#include "PartFile.h"
#include "HighColorTab.hpp"
#include "UserMsgs.h"
#include "DownloadListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CFileDetailDialog

LPCTSTR CFileDetailDialog::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CFileDetailDialog, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CFileDetailDialog, CListViewWalkerPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CFileDetailDialog::CFileDetailDialog(const CSimpleArray<CPartFile*>* paFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
	: CListViewWalkerPropertySheet(pListCtrl)
{
	m_uPshInvokePage = uPshInvokePage;
	for (int i = 0; i < paFiles->GetSize(); i++)
		m_aItems.Add((*paFiles)[i]);
	m_psh.dwFlags &= ~PSH_HASHELP;

	m_wndInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndInfo.m_psp.pszIcon = _T("FILEINFO");
	m_wndInfo.SetFiles(&m_aItems);
	AddPage(&m_wndInfo);

	if (m_aItems.GetSize() == 1)
	{
		m_wndName.m_psp.dwFlags &= ~PSP_HASHELP;
		m_wndName.m_psp.dwFlags |= PSP_USEICONID;
		m_wndName.m_psp.pszIcon = _T("RENAME");
		m_wndName.SetFiles(&m_aItems);
		AddPage(&m_wndName);
	}

	if (m_aItems.GetSize() == 1 && thePrefs.IsExtControlsEnabled()){
			m_wndMetaData.m_psp.dwFlags &= ~PSP_HASHELP;
			m_wndMetaData.m_psp.dwFlags |= PSP_USEICONID;
		m_wndMetaData.m_psp.pszIcon = _T("METADATA");
			m_wndMetaData.SetFiles(&m_aItems);
			AddPage(&m_wndMetaData);
		}

	m_wndFileLink.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileLink.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileLink.m_psp.pszIcon = _T("ED2KLINK");
	m_wndFileLink.SetFiles(&m_aItems);
	AddPage(&m_wndFileLink);

	LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}
}

CFileDetailDialog::~CFileDetailDialog()
{
}

void CFileDetailDialog::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CFileDetailDialog::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("FileDetailDialog"), !thePrefs.prefReadonly); // call this after(!) OnInitDialog // X: [ROP] - [ReadOnlyPreference]
	UpdateTitle();

	return bResult;
}

LRESULT CFileDetailDialog::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CFileDetailDialog::UpdateTitle()
{
	CString text = GetResString(IDS_DETAILS);
	if(m_aItems.GetSize() == 1)
		//text.AppendFormat(_T(": %s"), STATIC_DOWNCAST(CAbstractFile, m_aItems[0])->GetFileName());
		text.AppendFormat(_T(": %s"), ((CAbstractFile*)m_aItems[0])->GetFileName());
	SetWindowText(text);
}
