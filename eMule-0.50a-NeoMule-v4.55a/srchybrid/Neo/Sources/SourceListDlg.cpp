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
#include "kademlia/kademlia/tag.h"
#include "SourceListDlg.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "MenuCmds.h"
#include "Packets.h"
#include "KnownFile.h"
#include "UserMsgs.h"
#include "Neo/Functions.h"
#include "Neo/Sources/SourceList.h"
#include "Neo/Sources/SourceInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->

#define	PREF_INI_SECTION	_T("SourceListDlg")

IMPLEMENT_DYNAMIC(CSourceListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSourceListCtrl, CMuleListCtrl)
END_MESSAGE_MAP()

CSourceListCtrl::CSourceListCtrl()
	: CListCtrlItemWalk(this)
{
}

CSourceListCtrl::~CSourceListCtrl()
{
}


// CSourceListDlg dialog

IMPLEMENT_DYNAMIC(CSourceListDlg, CDialog)

BEGIN_MESSAGE_MAP(CSourceListDlg, CModResizableDialog) // NEO: MLD - [ModelesDialogs] 
	ON_NOTIFY(NM_DBLCLK, IDC_SOURCEDB, OnNMDblclk)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CSourceListDlg::CSourceListDlg()
	: CModResizableDialog(CSourceListDlg::IDD) // NEO: MLD - [ModelesDialogs]
{
	m_SourceDetailDialog = NULL; // NEO: MLD - [ModelesDialogs]
}

CSourceListDlg::~CSourceListDlg()
{
	ASSERT(m_SourceDetailDialog == NULL); // NEO: MLD - [ModelesDialogs]
}

void CSourceListDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_SOURCEDB, m_SourceList);
}

BOOL CSourceListDlg::OnInitDialog()
{
	CModResizableDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs] 
	InitWindowStyles(this);

	m_SourceList.SetName(_T("SourceList"));
	m_SourceList.ModifyStyle(LVS_SINGLESEL,0);
	m_SourceList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_SourceList.InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, 140); 
	m_SourceList.InsertColumn(1, GetResString(IDS_CD_UHASH), LVCFMT_LEFT, 220); 
	m_SourceList.InsertColumn(2, GetResString(IDS_X_IP_ZONE), LVCFMT_LEFT, 80); 
	m_SourceList.InsertColumn(3, GetResString(IDS_IDLOW), LVCFMT_LEFT, 50); 
	m_SourceList.InsertColumn(4, GetResString(IDS_X_IP_PORT), LVCFMT_LEFT, 120); 
	m_SourceList.InsertColumn(5, GetResString(IDS_CD_CSOFT), LVCFMT_LEFT, 130); 
	m_SourceList.InsertColumn(6, GetResString(IDS_LASTSEEN), LVCFMT_LEFT, 160); 

	m_SourceList.LoadSettings();

	AddAnchor(IDC_STATIC_SRCCOUNT, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_SOURCEDB, TOP_LEFT, BOTTOM_RIGHT);

	EnableSaveRestore(PREF_INI_SECTION);

	ReloadSourceList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSourceListDlg::ReloadSourceList()
{
	CWaitCursor curWait;
	m_SourceList.DeleteAllItems();
	m_SourceList.SetRedraw(FALSE);

	CKnownSource* cur_source;
	CCKey tmpkey(0);
	POSITION pos = theApp.sourcelist->m_mapSources.GetStartPosition();
	while (pos){
		theApp.sourcelist->m_mapSources.GetNextAssoc(pos, tmpkey, cur_source);

		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)cur_source;
		int iItem = m_SourceList.FindItem(&find);
		if (iItem == -1)
			iItem = m_SourceList.InsertItem(LVIF_TEXT|LVIF_PARAM,0,cur_source->GetUserName(),0,0,1,(LPARAM)cur_source);

		CString strBuff;

		strBuff = cur_source->GetUserName();
		m_SourceList.SetItemText(iItem, 0, strBuff); 

		strBuff = md4str(cur_source->GetUserHash());
		m_SourceList.SetItemText(iItem, 1, strBuff); 

		strBuff = ipstr(cur_source->GetIPZone());
		m_SourceList.SetItemText(iItem, 2, strBuff); 

		strBuff = cur_source->HasLowID() ? GetResString(IDS_YES) : GetResString(IDS_NO);
		m_SourceList.SetItemText(iItem, 3, strBuff); 

		strBuff = StrLine(_T("%s:%u"),ipstr(cur_source->GetIP()),cur_source->GetUserPort());
		m_SourceList.SetItemText(iItem, 4, strBuff); 

		strBuff = cur_source->GetClientModVer().IsEmpty() ? cur_source->GetClientSoftVer() : StrLine(_T("%s [%s]"),cur_source->GetClientSoftVer(),cur_source->GetClientModVer());
		m_SourceList.SetItemText(iItem, 5, strBuff); 

		strBuff = CastSecondsToDate(cur_source->GetLastSeen());
		m_SourceList.SetItemText(iItem, 6, strBuff); 

		if (GetAsyncKeyState(VK_ESCAPE) < 0){
			if (AfxMessageBox(GetResString(IDS_X_SRC_LIST_ABORT),MB_YESNO,NULL)==IDYES)
				break;
		}
	}

	GetDlgItem(IDC_STATIC_SRCCOUNT)->SetWindowText(StrLine(GetResString(IDS_X_SOURCES_LIST_COUNT),theApp.sourcelist->m_mapSources.GetCount()));

	m_SourceList.SetRedraw();
}

void CSourceListDlg::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult) {

	POSITION pos = m_SourceList.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		int index = m_SourceList.GetNextSelectedItem(pos);
		if (index >= 0){
			CKnownSource* Item = (CKnownSource*)m_SourceList.GetItemData(index);
			if(!theApp.sourcelist->IsSourcePtrInList(Item)){
				m_SourceList.DeleteItem(index);
				return;
			}
			// NEO: MLD - [ModelesDialogs]
			if(m_SourceDetailDialog)
				m_SourceDetailDialog->DropControl();
			m_SourceDetailDialog = new CSourceDetailDialog(Item,&m_SourceList);
			m_SourceDetailDialog->OpenDialog(FALSE);
			// NEO: MLD END
			//CSourceDetailDialog dlg(Item,&m_SourceList);
			//dlg.DoModal();
		}
	}

	*pResult = 0;
}

void CSourceListDlg::OnDestroy()
{
	// NEO: MLD - [ModelesDialogs]
	if(m_SourceDetailDialog)
		m_SourceDetailDialog->DropControl();
	m_SourceDetailDialog = NULL;
	// NEO: MLD END
	CResizableDialog::OnDestroy(); 
}

BOOL CSourceListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(wParam == MPG_DELETE){
		if (GetSelectedCount() == 0 
		|| IDNO == AfxMessageBox(GetResString(IDS_X_CONFIRM_SRCDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
			return TRUE;
		
		CTypedPtrList<CPtrList, CKnownSource*> selectedList;
		POSITION pos = GetFirstSelectedItemPosition();
		while(pos != NULL)
		{
			int index = GetNextSelectedItem(pos);
			if (index >= 0)
				selectedList.AddTail((CKnownSource*)GetItemData(index));
		}

		int iSkiped = 0;
		while (!selectedList.IsEmpty())
		{
			CKnownSource* Item = selectedList.RemoveHead();
			if(theApp.sourcelist->IsSourcePtrInList(Item)){
				if(!theApp.sourcelist->RemoveSourceFromPtrList(Item)){
					iSkiped++;
					continue;
				}
				delete Item;
			}

			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)Item;
			sint32 result = FindItem(&find);
			if (result != (-1) )
				DeleteItem(result);
		}
		if(iSkiped)
			AfxMessageBox(StrLine(GetResString(IDS_X_SRCDELETE_SKIPPED),iSkiped),MB_ICONWARNING | MB_OK);

		return TRUE;
	}

	return CMuleListCtrl::OnCommand(wParam, lParam);
}

#endif // NEO_CD // NEO: NCD END <-- Xanatos --