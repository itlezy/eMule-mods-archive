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

#include "StdAfx.h"
#include "IrcChannelListCtrl.h"
#include "emuleDlg.h"
#include "otherfunctions.h"
#include "MenuCmds.h"
#include "IrcWnd.h"
#include "IrcMain.h"
#include "emule.h"
#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct ChannelName
{
	ChannelName(const CString& sName, UINT uUsers, const CString& sDesc)
		: m_sName(sName), m_uUsers(uUsers), m_sDesc(sDesc)
	{ }
	CString m_sName;
	UINT m_uUsers;
	CString m_sDesc;
};

IMPLEMENT_DYNAMIC(CIrcChannelListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CIrcChannelListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblClk)
END_MESSAGE_MAP()

CIrcChannelListCtrl::CIrcChannelListCtrl()
{
	m_pParent = NULL;
	SetName(_T("IrcChannelListCtrl"));
}

CIrcChannelListCtrl::~CIrcChannelListCtrl()
{
	ResetServerChannelList(true);
}

int CIrcChannelListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const ChannelName* pItem1 = (ChannelName*)lParam1;
	const ChannelName* pItem2 = (ChannelName*)lParam2;
	switch (lParamSort)
	{
		case 0:
			return pItem1->m_sName.CompareNoCase(pItem2->m_sName);
		case 10:
			return pItem2->m_sName.CompareNoCase(pItem1->m_sName);
		case 1:
			return CompareUnsigned(pItem1->m_uUsers, pItem2->m_uUsers);
		case 11:
			return CompareUnsigned(pItem2->m_uUsers, pItem1->m_uUsers);
		case 2:
			return pItem1->m_sDesc.CompareNoCase(pItem2->m_sDesc);
		case 12:
			return pItem2->m_sDesc.CompareNoCase(pItem1->m_sDesc);
		default:
			return 0;
	}
}

void CIrcChannelListCtrl::OnLvnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	bool bSortAscending = (GetSortItem() != pNMLV->iSubItem) ? true : !GetSortAscending();
	SetSortArrow(pNMLV->iSubItem, bSortAscending);
	SortItems(&SortProc, pNMLV->iSubItem + (bSortAscending ? 0 : 10));

	*pResult = 0;
}

void CIrcChannelListCtrl::OnContextMenu(CWnd*, CPoint point)
{
	int iCurSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	CTitleMenu menuChannel;
	menuChannel.CreatePopupMenu();
	menuChannel.AddMenuTitle(GetResString(IDS_IRC_CHANNEL));
	menuChannel.AppendMenu(MF_STRING, Irc_Join, GetResString(IDS_IRC_JOIN));
	if (iCurSel == -1)
		menuChannel.EnableMenuItem(Irc_Join, MF_GRAYED);
	GetPopupMenuPos(*this, point);
	menuChannel.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( menuChannel.DestroyMenu() );
}

void CIrcChannelListCtrl::OnNMDblClk(NMHDR*, LRESULT* pResult)
{
	JoinChannels();
	*pResult = 0;
}

void CIrcChannelListCtrl::ResetServerChannelList(bool bShutDown)
{
	POSITION pos = m_lstChannelNames.GetHeadPosition();
	while (pos)
		delete m_lstChannelNames.GetNext(pos);
	m_lstChannelNames.RemoveAll();
	if (!bShutDown)
		DeleteAllItems();
}

bool CIrcChannelListCtrl::AddChannelToList(const CString& sName, const CString& sUsers, const CString& sDesc)
{
	UINT uUsers = _tstoi(sUsers);
	if (thePrefs.GetIRCUseChannelFilter())
	{
		if (uUsers < thePrefs.GetIRCChannelUserFilter())
			return false;
		// was already filters with "/LIST" command
		//if (!thePrefs.GetIRCChannelFilter().IsEmpty())
		//{
		//	if (stristr(sName, thePrefs.GetIRCChannelFilter()) == NULL)
		//		return false;
		//}
	}

	ChannelName* pChannel = new ChannelName(sName, uUsers, m_pParent->StripMessageOfFontCodes(sDesc));
	m_lstChannelNames.AddTail(pChannel);
	int iItem = InsertItem(LVIF_TEXT | LVIF_PARAM, GetItemCount(), pChannel->m_sName, 0, 0, 0, (LPARAM)pChannel);
	if (iItem < 0)
		return false;
	SetItemText(iItem, 1, 0);
	SetItemText(iItem, 2, 0);
	return true;
}

void CIrcChannelListCtrl::JoinChannels()
{
	if (!m_pParent->IsConnected())
		return;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iIndex = GetNextSelectedItem(pos);
		if (iIndex >= 0)
			m_pParent->m_pIrcMain->SendString(_T("JOIN ") + GetItemText(iIndex, 0));
	}
}

void CIrcChannelListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_IRC_NAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_UUSERS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_DESCRIPTION);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);
}

void CIrcChannelListCtrl::Init()
{
	InsertColumn(0, GetResString(IDS_IRC_NAME), LVCFMT_LEFT, 203);
	InsertColumn(1, GetResString(IDS_UUSERS), LVCFMT_LEFT, 50);
	InsertColumn(2, GetResString(IDS_DESCRIPTION), LVCFMT_LEFT, 350);

	LoadSettings();
	SetSortArrow();
	SortItems(&SortProc, GetSortItem() + (GetSortAscending() ? 0 : 10));
}

BOOL CIrcChannelListCtrl::OnCommand(WPARAM wParam, LPARAM)
{
	switch (wParam)
	{
		case Irc_Join:
			//Pressed the join button.
			JoinChannels();
			return TRUE;
	}
	return TRUE;
}

void CIrcChannelListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	const ChannelName* pChannel = (ChannelName*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE) {
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CString strBuff;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;

	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if (!IsColumnHidden(iColumn))
		{
			int cx = GetColumnWidth(iColumn);
			cur_rec.right += cx;
			switch (iColumn)
			{
				case 0:
					strBuff = pChannel->m_sName;
					break;

				case 1:
					strBuff.Format(_T("%u"), pChannel->m_uUsers);
					break;

				case 2:
					strBuff = pChannel->m_sDesc;
					break;
			}
			dc->DrawText(strBuff, strBuff.GetLength(), &cur_rec, DT_LEFT);
			cur_rec.left += cx;
		}
	}

	//draw rectangle around selected item(s)
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;
		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;
		if (bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}
