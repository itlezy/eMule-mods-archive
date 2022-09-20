//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "CommentDialogLst.h"
#include "PartFile.h"
#include "UpDownClient.h"
#include "UserMsgs.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/Search.h"
#include "searchlist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CCommentDialogLst, CResizablePage) 

BEGIN_MESSAGE_MAP(CCommentDialogLst, CResizablePage) 
   	ON_BN_CLICKED(IDOK, OnBnClickedApply) 
   	ON_BN_CLICKED(IDC_SEARCHKAD, OnBnClickedSearchKad) 
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP() 

CCommentDialogLst::CCommentDialogLst() 
   : CResizablePage(CCommentDialogLst::IDD, IDS_CMT_READALL) 
{
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_CMT_READALL);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_paFiles = NULL; 
	m_timer = 0;
	bAllowCancel = false;	// Tux: Improvement: Kademlia interface improvement
} 

CCommentDialogLst::~CCommentDialogLst() 
{ 
} 

void CCommentDialogLst::DoDataExchange(CDataExchange* pDX) 
{ 
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LST, m_lstComments);
} 

void CCommentDialogLst::OnBnClickedApply() 
{ 
	CResizablePage::OnOK(); 
} 

void CCommentDialogLst::OnTimer(UINT /*nIDEvent*/)
{
	RefreshData(false);
}

BOOL CCommentDialogLst::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);

	AddAnchor(IDC_LST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_SEARCHKAD,BOTTOM_RIGHT);

	m_lstComments.Init();

	Localize(); 

	// start time for calling 'RefreshData'
	VERIFY( (m_timer = SetTimer(301, 5000, 0)) != NULL );

	return TRUE; 
} 

BOOL CCommentDialogLst::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;
	if (m_bDataChanged)
	{
		RefreshData();
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CCommentDialogLst::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CCommentDialogLst::OnDestroy()
{
	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}
}

void CCommentDialogLst::Localize(void)
{ 
	GetDlgItem(IDC_SEARCHKAD)->SetWindowText(GetResString(IDS_SEARCHKAD));
} 

void CCommentDialogLst::RefreshData(bool deleteOld)
{
	if(deleteOld)
		m_lstComments.DeleteAllItems();

	// Tux: Improvement: Kademlia interface improvement [start]: replaced line
	if(deleteOld == false)
		bAllowCancel = true;
	// Tux: Improvement: Kademlia interface improvement [end]
	for (int i = 0; i < m_paFiles->GetSize(); i++)
    {
		CAbstractFile* file = STATIC_DOWNCAST(CAbstractFile, (*m_paFiles)[i]);
		if(file->IsPartFile())
		{
			for (POSITION pos = ((CPartFile*)file)->srclist.GetHeadPosition(); pos != NULL; )
			{ 
				CUpDownClient* cur_src = ((CPartFile*)file)->srclist.GetNext(pos);
				if (cur_src->HasFileRating() || !cur_src->GetFileComment().IsEmpty())
					m_lstComments.AddItem(cur_src);
			}
		} 
	
		const CTypedPtrList<CPtrList, Kademlia::CEntry*>& list = file->getNotes();
		for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
		{
			Kademlia::CEntry* entry = list.GetNext(pos);
			m_lstComments.AddItem(entry);
		}
		if(file->IsPartFile())
			((CPartFile*)file)->UpdateFileRatingCommentAvail();

		// check if note searches are running for this file(s)
		// Tux: Improvement: Kademlia interface improvement [start]: replaced lines
		if(deleteOld == false)
			bAllowCancel = bAllowCancel && file->GetKadFileSearchID();
		// Tux: Improvement: Kademlia interface improvement [end]
	}

	CWnd* pWndFocus = GetFocus();
	if (Kademlia::CKademlia::IsConnected()) {
		//SetDlgItemText(IDC_SEARCHKAD, kadsearchable ? GetResString(IDS_SEARCHKAD) : GetResString(IDS_KADSEARCHACTIVE));
		GetDlgItem(IDC_SEARCHKAD)->EnableWindow(TRUE);
	}
	else {
		SetDlgItemText(IDC_SEARCHKAD, GetResString(IDS_SEARCHKAD));
		GetDlgItem(IDC_SEARCHKAD)->EnableWindow(FALSE);
	}
	GetDlgItem(IDC_SEARCHKAD)->SetWindowText(GetResString(bAllowCancel ? IDS_CANCEL : IDS_SEARCHKAD));	// Tux: Improvement: Kademlia interface improvement
	if (pWndFocus && pWndFocus->m_hWnd == GetDlgItem(IDC_SEARCHKAD)->m_hWnd)
		m_lstComments.SetFocus();
}

void CCommentDialogLst::OnBnClickedSearchKad()
{
	if (Kademlia::CKademlia::IsConnected())
	{
		bool bSkipped = false;
		int iMaxSearches = min(m_paFiles->GetSize(), KADEMLIATOTALFILE);
	    for (int i = 0; i < iMaxSearches; i++)
	    {
			CAbstractFile* file = STATIC_DOWNCAST(CAbstractFile, (*m_paFiles)[i]);
 			if (file)
			{
				// Tux: Improvement: Kademlia interface improvement [start]: replaced block
				if(bAllowCancel == false)
				{
					Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::NOTES, true, Kademlia::CUInt128(file->GetFileHash()));
					if(pSearch)
					{
						pSearch->SetFileName(file->GetFileName());
						file->SetKadFileSearchID(pSearch->GetSearchID());
						// offi code
					theApp.searchlist->SetNotesSearchStatus(file->GetFileHash(), true);
					file->SetKadCommentSearchRunning(true);
				}
					else
						bSkipped = true;
				}
				else if(file->GetKadFileSearchID())
					Kademlia::CSearchManager::StopSearch(file->GetKadFileSearchID(), false);
				// Tux: Improvement: Kademlia interface improvement [end]
			}
		}
		if (bSkipped)
			AfxMessageBox(GetResString(IDS_KADSEARCHALREADY), MB_OK | MB_ICONINFORMATION);
		// Tux: Improvement: Kademlia interface improvement [start]
		bAllowCancel = !bAllowCancel;
		GetDlgItem(IDC_SEARCHKAD)->SetWindowText(GetResString(bAllowCancel ? IDS_CANCEL : IDS_SEARCHKAD)); 
		// Tux: Improvement: Kademlia interface improvement [end]
	}
	RefreshData();
}
