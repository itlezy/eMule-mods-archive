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

#include "stdafx.h"
#include "FDSrcNames.h"
#include "filedetails.h"
#include "..\SharedFileList.h"
#include "..\updownclient.h"
#include "..\TitleMenu.h"
#include "..\emule.h"

IMPLEMENT_DYNCREATE(CFDSrcNames, CPropertyPage)

CFDSrcNames::CFDSrcNames() : CPropertyPage(CFDSrcNames::IDD)
{
	m_pFile = NULL;
	m_bSortAscending[FDSRCCOL_FILENAME] = true;
	m_bSortAscending[FDSRCCOL_SOURCES] = false;
}

CFDSrcNames::~CFDSrcNames()
{
}

void CFDSrcNames::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCENAMES, pmyListCtrl);
	DDX_Control(pDX, IDC_FDS_TAKEOVER, m_ctrlTakeOver);
	DDX_Control(pDX, IDC_CLEANUP, m_ctrlCleanUp);
	DDX_Control(pDX, IDC_RENAME, m_ctrlRename);
	DDX_Control(pDX, IDC_FILENAME, m_ctrlFilename);
}


BEGIN_MESSAGE_MAP(CFDSrcNames, CPropertyPage)
	ON_BN_CLICKED(IDC_RENAME, OnBnClickedRename)
	ON_BN_CLICKED(IDC_CLEANUP, OnBnClickedCleanup)
	ON_BN_CLICKED(IDC_FDS_TAKEOVER, OnBnClickedFdsTakeover)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SOURCENAMES, OnLvnColumnclick)
	ON_NOTIFY(NM_DBLCLK, IDC_SOURCENAMES, OnNMDblclkList)
	ON_NOTIFY(NM_RCLICK, IDC_SOURCENAMES, OnNMRclickList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFDSrcNames message handlers
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::Update()
{
	EMULE_TRY

	if (m_pFile == NULL)
		return;

	if (!::IsWindow(GetSafeHwnd()))
		return;

	EnumPartFileStatuses	eFileStatus = m_pFile->GetStatus();
	bool					bEnable = ((eFileStatus == PS_COMPLETE) || (eFileStatus == PS_COMPLETING)) ? false : true;

	m_ctrlRename.EnableWindow(bEnable);
	m_ctrlCleanUp.EnableWindow(bEnable);
	m_ctrlTakeOver.EnableWindow(bEnable);
	m_ctrlFilename.EnableWindow(bEnable);

	FillSourcenameList();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::FillSourcenameList()
{
	EMULE_TRY

	LVFINDINFO			info;
	int					iItemPos, iNewIndex;
	CString				strText;
	FCtrlItem_Struct	*pItem;
	ClientList			clientListCopy;

	info.flags = LVFI_STRING;

// Reset
	for (int i = 0; i < pmyListCtrl.GetItemCount(); i++)
	{
		pItem = (FCtrlItem_Struct*)pmyListCtrl.GetItemData(i);
		pItem->iCount = 0;
	}

// Update
	m_pFile->GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		CUpDownClient	*pClient = *cIt;

		if ((pClient->m_pReqPartFile != m_pFile) || pClient->IsClientFilenameEmpty())
			continue;

		strText = pClient->GetClientFilename();
		info.psz = strText;
		if ((iItemPos = pmyListCtrl.FindItem(&info, -1)) == -1)
		{ 
			pItem = new FCtrlItem_Struct();
			pItem->iCount = 1;
			pItem->strFileName = strText;
			iNewIndex = pmyListCtrl.InsertItem(LVIF_TEXT | LVIF_PARAM, pmyListCtrl.GetItemCount(), strText, 0, 0, 0, (LPARAM)pItem);
			pmyListCtrl.SetItemText(iNewIndex, 1, _T("1"));
		}
		else
		{
			pItem = (FCtrlItem_Struct*)pmyListCtrl.GetItemData(iItemPos);
			strText.Format(_T("%u"), ++pItem->iCount);
			pmyListCtrl.SetItemText(iItemPos, 1, strText);
		} 
	}

// Remove zeros
	for (int i = 0; i < pmyListCtrl.GetItemCount(); i++)
	{
		pItem = (FCtrlItem_Struct*)pmyListCtrl.GetItemData(i);
		if (pItem != NULL && pItem->iCount == 0)
		{
			delete pItem;
			pmyListCtrl.DeleteItem(i);
			i = 0;
		}
	}

	pmyListCtrl.SortItems(CompareListNameItems, pmyListCtrl.GetSortParam());

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnBnClickedRename()
{
	EMULE_TRY

	CString			strNewFileName;

	m_ctrlFilename.GetWindowText(strNewFileName);

	if (strNewFileName != m_pFile->GetFileName())	// update only when file name changed
	{
		m_pFile->SetFileName(strNewFileName);
		m_pFile->SavePartFile();

		CFileDetails	*pParent = (CFileDetails*)GetParent();

		if (pParent != NULL)
			pParent->UpdateData();

	//	Update the lists
		m_pFile->UpdateDisplayedInfo();
		g_App.m_pSharedFilesList->UpdateItem((CKnownFile*)m_pFile);
		g_App.m_pMDlg->m_wndTransfer.UpdateInfoHeader();
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnBnClickedCleanup()
{
	CString			strFilename;
	int				iStart, iEnd;

	m_ctrlFilename.GetSel(iStart, iEnd);

	if (m_ctrlFilename.GetWindowTextLength() != iEnd - iStart)
		m_ctrlFilename.ReplaceSel(strFilename.GetBuffer());

	m_ctrlFilename.GetWindowText(strFilename);
	m_ctrlFilename.SetWindowText(CleanupFilename(strFilename));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnBnClickedFdsTakeover()
{
	if (pmyListCtrl.GetSelectedCount() > 0)
	{
		POSITION	pos = pmyListCtrl.GetFirstSelectedItemPosition();

		SetDlgItemText(IDC_FILENAME, pmyListCtrl.GetItemText(pmyListCtrl.GetNextSelectedItem(pos), 0));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	EMULE_TRY

	NM_LISTVIEW	*pNMListView = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	int			iSubItem = pNMListView->iSubItem;
	bool		bSortOrder = m_bSortAscending[iSubItem];

// Reverse sorting direction for the same column and keep the same if column was changed
	if ((pmyListCtrl.GetSortParam() & MLC_COLUMNMASK) == static_cast<uint32>(iSubItem))
		m_bSortAscending[iSubItem] = bSortOrder = !bSortOrder;

	pmyListCtrl.SetSortArrow(iSubItem, bSortOrder);
	pmyListCtrl.SortItems(CompareListNameItems, iSubItem + ((bSortOrder) ? MLC_SORTASC : MLC_SORTDESC));

	*pResult = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	OnBnClickedFdsTakeover();
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	POINT			point;
	CTitleMenu		popupMenu;
	NOPRM(pNMHDR);

	::GetCursorPos(&point);

	popupMenu.CreatePopupMenu();
	popupMenu.AppendMenu( MF_STRING |
		((pmyListCtrl.GetSelectionMark() == -1) ? MF_GRAYED : 0),
		MP_MESSAGE, GetResString(IDS_TAKEOVER) );
	popupMenu.AppendMenu(MF_STRING, MP_RESTORE, GetResString(IDS_SV_UPDATE));
	popupMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

//	Local Menu objects are destroyed in their destructor

	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFDSrcNames::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (pmyListCtrl.GetSelectionMark() != -1)
	{
		switch (wParam)
		{
			case MP_MESSAGE:
				OnBnClickedFdsTakeover();
				return true;

			case MP_RESTORE:
				FillSourcenameList();
				return true;
		}
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CALLBACK CFDSrcNames::CompareListNameItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FCtrlItem_Struct	*pItem1 = (FCtrlItem_Struct*) lParam1;
	FCtrlItem_Struct	*pItem2 = (FCtrlItem_Struct*) lParam2;

	switch(lParamSort)
	{
		case 0:
			return (pItem1->strFileName.CompareNoCase(pItem2->strFileName));
			break;
		case 0+MLC_SORTDESC:
			return (pItem2->strFileName.CompareNoCase(pItem1->strFileName));
			break;
		case 1:
			return (pItem1->iCount - pItem2->iCount);
			break;
		case 1+MLC_SORTDESC:
			return (pItem2->iCount - pItem1->iCount);
			break;
		default:
			return 0;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFDSrcNames::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Localize();

	pmyListCtrl.InsertColumn(0, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 240, -1);
	pmyListCtrl.InsertColumn(1, GetResString(IDS_DL_SOURCES), LVCFMT_RIGHT, 75, 1);
	pmyListCtrl.SetSortArrow(FDSRCCOL_SOURCES, m_bSortAscending[FDSRCCOL_SOURCES]);
	pmyListCtrl.SortItems(CompareListNameItems, FDSRCCOL_SOURCES + ((m_bSortAscending[FDSRCCOL_SOURCES]) ? MLC_SORTASC : MLC_SORTDESC));
	pmyListCtrl.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_INFOTIP, LVS_EX_INFOTIP);

	Update();

	m_ctrlFilename.SetWindowText(m_pFile->GetFileName());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_FDS_TAKEOVER, IDS_TAKEOVER },
		{ IDC_CLEANUP, IDS_CLEANUP },
		{ IDC_RENAME, IDS_RENAME }
	};

	if (GetSafeHwnd())
	{
		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
			SetDlgItemText(s_auResTbl[i][0], GetResString(static_cast<UINT>(s_auResTbl[i][1])));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnOK()
{
	OnBnClickedRename();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFDSrcNames::OnDestroy()
{
	FCtrlItem_Struct	*pItem;

	for (int i = 0; i < pmyListCtrl.GetItemCount(); ++i)
	{
		pItem = (FCtrlItem_Struct*)pmyListCtrl.GetItemData(i);
		delete pItem;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
