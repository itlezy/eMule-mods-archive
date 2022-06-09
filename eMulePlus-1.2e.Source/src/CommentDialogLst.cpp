//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "updownclient.h"
#include "CommentDialogLst.h"
#include "PastComment.h"
#include "Details\ClientDetails.h"
#include "TitleMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define DRAWTEXTFMT		(DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP)


IMPLEMENT_DYNAMIC(CCommentDialogLst, CDialog)
CCommentDialogLst::CCommentDialogLst(CPartFile* file)
	: CResizableDialog(CCommentDialogLst::IDD, 0)
{
	m_file = file;
}

CCommentDialogLst::~CCommentDialogLst()
{
}

void CCommentDialogLst::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LST, pmyListCtrl);
}

BEGIN_MESSAGE_MAP(CCommentDialogLst, CResizableDialog)
	ON_BN_CLICKED(IDC_CMT_OK, OnBnClickedApply)
	ON_BN_CLICKED(IDC_CMT_REF, OnBnClickedRefresh)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_LST, OnNMRclickList)
END_MESSAGE_MAP()



void CCommentDialogLst::OnBnClickedApply()
{
	CDialog::OnOK();
}

void CCommentDialogLst::OnBnClickedRefresh()
{
	CompleteList();
}

BOOL CCommentDialogLst::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	AddAnchor(IDC_LST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_CMT_OK,BOTTOM_RIGHT);
	AddAnchor(IDC_CMT_REF,BOTTOM_RIGHT);
	AddAnchor(IDC_CMSTATUS,BOTTOM_LEFT);

	pmyListCtrl.InsertColumn(0, GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, 100, -1);
	pmyListCtrl.InsertColumn(1, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 150, -1);
	pmyListCtrl.InsertColumn(2, GetResString(IDS_RATING), LVCFMT_LEFT, 80, 1);
	pmyListCtrl.InsertColumn(3, GetResString(IDS_COMMENT), LVCFMT_LEFT, 315, 1);

	Localize();
	CompleteList();

	return TRUE;
}

void CCommentDialogLst::Localize(void)
{
	static const int s_aiResTbl[][2] =
	{
		{ IDC_CMT_OK, IDS_CW_CLOSE },
		{ IDC_CMT_REF, IDS_CMT_REFRESH }
	};
	CString	strBuff;

	for (unsigned ui = 0; ui < ARRSIZE(s_aiResTbl); ui++)
		SetDlgItemText(s_aiResTbl[ui][0], GetResString(static_cast<UINT>(s_aiResTbl[ui][1])));

	strBuff.Format(_T("%s (%s)"), GetResString(IDS_CMT_READALL), m_file->GetFileName());
	SetWindowText(strBuff);
}

void CCommentDialogLst::CompleteList()
{
	POSITION	pos;
	ClientList	clientListCopy;
	int			iCount = 0;
	const TCHAR	*pcTmp;

	pmyListCtrl.DeleteAllItems();

	m_file->GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		CUpDownClient	*pClient = *cIt;

		if ((pClient->GetFileRating() != PF_RATING_NONE) || !pClient->IsFileCommentEmpty())
		{
			pmyListCtrl.InsertItem(LVIF_TEXT | LVIF_PARAM, iCount, pClient->GetUserName(), 0, 0, 0, reinterpret_cast<LPARAM>(pClient));
			pmyListCtrl.SetItemText(iCount, 1, pClient->GetClientFilename());
			pmyListCtrl.SetItemText(iCount, 2, GetRatingString(pClient->GetFileRating()));
			pmyListCtrl.SetItemText(iCount, 3, pClient->GetFileComment());
			iCount++;
		}
	}

	CString				strTmp = m_file->GetFileComment();
	EnumPartFileRating	eRated = m_file->GetFileRating();

	if (!strTmp.IsEmpty() || (eRated != PF_RATING_NONE))
	{
		pmyListCtrl.InsertItem(iCount, g_App.m_pPrefs->GetUserNick());
		pmyListCtrl.SetItemText(iCount, 1, m_file->GetFileName());
		pmyListCtrl.SetItemText(iCount, 2, GetRatingString(eRated));
		pmyListCtrl.SetItemText(iCount, 3, strTmp);
		iCount++;
	}

	CPastCommentList	   &pclist = m_file->GetPastCommentList();

	for (pos = pclist.GetHeadPosition(); pos != NULL;)
	{
		CPastComment &pc = pclist.GetNext(pos);

		pmyListCtrl.InsertItem(LVIF_TEXT | LVIF_STATE, iCount, pc.GetClientName(), LVIS_CUT, LVIS_CUT, 0, NULL);
		pmyListCtrl.SetItemText(iCount, 1, pc.GetFileName());
		pmyListCtrl.SetItemText(iCount, 2, GetRatingString(pc.GetRating()));
		pmyListCtrl.SetItemText(iCount, 3, pc.GetComment());
		iCount++;
	}

	pcTmp = _T("");
	if (iCount == 0)
	{
		strTmp.Format(_T("(%s)"), GetResString(IDS_CMT_NONE));
		pcTmp = strTmp.GetString();
	}
	SetDlgItemText(IDC_CMSTATUS, pcTmp);
	m_file->UpdateFileRatingCommentAvail();
}

void CCommentDialogLst::OnDestroy()
{
	CResizableDialog::OnDestroy();
}

void CCommentDialogLst::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	if ((pmyListCtrl.GetSelectionMark() != (-1)) && (pmyListCtrl.GetSelectedCount() == 1))
	{
		CTitleMenu			menuCmt;
		CTitleMenu			menuClient;
		POINT				point;
		POSITION			posSelClient = pmyListCtrl.GetFirstSelectedItemPosition();
		CUpDownClient		*pClient = reinterpret_cast<CUpDownClient*>(pmyListCtrl.GetItemData(pmyListCtrl.GetNextSelectedItem(posSelClient)));
		UINT				dwMenuFlags = MF_STRING | MF_GRAYED;

		::GetCursorPos(&point);
		menuClient.CreateMenu();

		if (pClient != NULL)
		{
			UINT_PTR			dwRes = MP_ADDFRIEND;
			UINT				dwResStrId = IDS_ADDFRIEND;

			dwMenuFlags = MF_STRING | MF_ENABLED;
			if (pClient->IsFriend())
			{
				dwRes = MP_REMOVEFRIEND;
				dwResStrId = IDS_REMOVEFRIEND;
			}
			menuClient.AddMenuTitle(GetResString(IDS_CLIENT));
			menuClient.AppendMenu(dwMenuFlags, MP_DETAIL, GetResString(IDS_SHOWDETAILS));
			menuClient.AppendMenu(dwMenuFlags, dwRes, GetResString(dwResStrId));
			menuClient.AppendMenu(dwMenuFlags, MP_MESSAGE, GetResString(IDS_SEND_MSG));
			menuClient.AppendMenu( dwMenuFlags | ((pClient->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED),
				MP_SHOWLIST, GetResString(IDS_VIEWFILES) );
		}

		menuCmt.CreatePopupMenu();
		menuCmt.AddMenuTitle(GetResString(IDS_COMMENTS));
		menuCmt.AppendMenu(MF_STRING, MP_COPYSELECTED, GetResString(IDS_COPY));
		menuCmt.AppendMenu(MF_SEPARATOR);
		menuCmt.AppendMenu(dwMenuFlags|MF_POPUP,(UINT_PTR)menuClient.m_hMenu, GetResString(IDS_CLIENT));

		menuCmt.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
	}
	*pResult = 0;
}

BOOL CCommentDialogLst::OnCommand(WPARAM wParam,LPARAM lParam )
{
	POSITION			posSelClient;

	if ( (pmyListCtrl.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED) != -1) &&
		((posSelClient = pmyListCtrl.GetFirstSelectedItemPosition()) != NULL) )
	{
		int				iIdx = pmyListCtrl.GetNextSelectedItem(posSelClient);
		CUpDownClient	*pClient = reinterpret_cast<CUpDownClient*>(pmyListCtrl.GetItemData(iIdx));

		switch (wParam)
		{
			case MP_COPYSELECTED:
			{
				g_App.CopyTextToClipboard(pmyListCtrl.GetItemText(iIdx, 3));
				return true;
			}
			case MP_SHOWLIST:
			{
				pClient->RequestSharedFileList();
				return true;
			}
			case MP_MESSAGE:
			{
				g_App.m_pMDlg->m_wndChat.StartSession(pClient);
				return true;
			}
			case MP_ADDFRIEND:
			{
				g_App.m_pFriendList->AddFriend(pClient);
				return true;
			}
			case MP_REMOVEFRIEND:
			{
				g_App.m_pFriendList->RemoveFriend(pClient);
				return true;
			}
			case MP_DETAIL:
			{
				CClientDetails dialog(IDS_CD_TITLE, pClient, this, 0);
				dialog.DoModal();
				return true;
			}
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
