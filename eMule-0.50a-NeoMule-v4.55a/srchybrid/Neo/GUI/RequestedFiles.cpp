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

#include "stdafx.h" 
#include "emule.h"
#include "RequestedFiles.h"
#include "OtherFunctions.h"
#include "ClientList.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "UserMsgs.h"
#include "SharedFileList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NEO: RFL - [RequestFileList] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CReqFilesPage

IMPLEMENT_DYNAMIC(CReqFilesPage, CResizablePage)

BEGIN_MESSAGE_MAP(CReqFilesPage, CResizablePage)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnBnClickedApply)
	//ON_BN_CLICKED(IDC_REFRESH, OnBnClickedRefresh)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CReqFilesPage::CReqFilesPage()
   : CResizablePage(CReqFilesPage::IDD, 0)
{
	m_paClients = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_X_REQUESTED_FILES);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;

	m_lstReqFiles.SetDataType(KNOWN_TYPE);
	m_lstReqFilesUp.SetDataType(STRUCT_TYPE);

	m_timer = 0;
}

CReqFilesPage::~CReqFilesPage()
{
}

void CReqFilesPage::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LST, m_lstReqFiles);
	DDX_Control(pDX, IDC_LST_UP, m_lstReqFilesUp);
}

void CReqFilesPage::OnBnClickedApply()
{
	CResizablePage::OnOK(); 
}

//void CReqFilesPage::OnBnClickedRefresh()
//{
//	RefreshData();
//}

BOOL CReqFilesPage::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);

	AddAnchor(IDC_LST,TOP_LEFT,MIDDLE_RIGHT);
	AddAnchor(IDC_LST_UP,MIDDLE_LEFT,BOTTOM_RIGHT);
	//AddAnchor(IDC_REFRESH,BOTTOM_RIGHT);
	AddAnchor(IDC_CMSTATUS,MIDDLE_LEFT);
	AddAnchor(IDC_CMSTATUS_UP,BOTTOM_LEFT);

	m_lstReqFiles.Init();
	m_lstReqFilesUp.Init();

	Localize();

	VERIFY( (m_timer = SetTimer(301, 5000, 0)) != NULL );

	return TRUE; 
} 

void CReqFilesPage::OnDestroy()
{
	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}

	CPropertyPage::OnDestroy();
}

BOOL CReqFilesPage::OnSetActive()
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

LRESULT CReqFilesPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CReqFilesPage::Localize(void)
{ 
	//GetDlgItem(IDC_REFRESH)->SetWindowText(GetResString(IDS_CMT_REFRESH));
} 

void CReqFilesPage::OnTimer(UINT /*nIDEvent*/)
{
	RefreshData(false);
}

void CReqFilesPage::RefreshData(bool deleteOld)
{
	if(deleteOld){
		m_lstReqFiles.DeleteAllItems();
		m_lstReqFilesUp.DeleteAllItems();
	}

	if (!theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
		return;

	CUpDownClient* client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);

	int count=0;
	m_lstReqFiles.SetClient(client);
	if (theApp.downloadqueue->IsPartFile(client->GetRequestFile()))
	{
		m_lstReqFiles.AddFile(client->GetRequestFile(),false,false);
		count++;
	}
	for(POSITION pos = client->m_OtherRequests_list.GetHeadPosition(); pos; client->m_OtherRequests_list.GetNext(pos))
	{
		m_lstReqFiles.AddFile(client->m_OtherRequests_list.GetAt(pos),false,true);
		count++;
	}
	for(POSITION pos = client->m_OtherNoNeeded_list.GetHeadPosition(); pos; client->m_OtherNoNeeded_list.GetNext(pos))
	{
		m_lstReqFiles.AddFile(client->m_OtherNoNeeded_list.GetAt(pos),true,false);
		count++;
	}

	CString info;
	if (count==0) 
		info=GetResString(IDS_X_DOWNFILES_NONE);
	else
		info.Format(GetResString(IDS_X_DOWNFILES_COUNT), count);
	GetDlgItem(IDC_CMSTATUS)->SetWindowText(info);


	int countUp=0;
	m_lstReqFilesUp.SetClient(client);
	for(POSITION pos = client->GetRequestedFilesList()->GetHeadPosition();pos!=NULL;client->GetRequestedFilesList()->GetNext(pos))
	{
		Requested_File_Struct* requpfile = client->GetRequestedFilesList()->GetAt(pos);
		m_lstReqFilesUp.AddFile(requpfile,md4cmp(client->GetUploadFileID(),requpfile->fileid) ? true : false);
		countUp++;
	}

	CString infoUp;
	if (countUp==0) 
		infoUp=GetResString(IDS_X_UPFILES_NONE);
	else
		infoUp.Format(GetResString(IDS_X_UPFILES_COUNT), countUp);
	GetDlgItem(IDC_CMSTATUS_UP)->SetWindowText(infoUp);
}

// NEO: RFL END <-- Xanatos --