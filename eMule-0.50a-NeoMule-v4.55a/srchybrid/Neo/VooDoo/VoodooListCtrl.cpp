//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "VoodooListCtrl.h"
#include "OtherFunctions.h"
#include "MenuCmds.h"
#include "TitleMenu.h"
#include "emule.h"
#include "Neo/VooDoo/Voodoo.h"
#include "Neo/Functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

IMPLEMENT_DYNAMIC(CVoodooListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CVoodooListCtrl, CMuleListCtrl)
	//ON_WM_CONTEXTMENU()
	//ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteItem)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
END_MESSAGE_MAP()

CVoodooListCtrl::CVoodooListCtrl()
	: CListCtrlItemWalk(this)
{
}

CVoodooListCtrl::~CVoodooListCtrl()
{
}

void CVoodooListCtrl::Init(void)
{
	SetName(_T("VoodooListCtrl"));
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(0, GetResString(IDS_X_VOODOO_NAME), LVCFMT_LEFT, 130); 
	InsertColumn(1, GetResString(IDS_X_VOODOO_ADRESS), LVCFMT_LEFT, 120); 
	InsertColumn(2, GetResString(IDS_X_VOODOO_ACTION), LVCFMT_LEFT, 50); 
	InsertColumn(3, GetResString(IDS_X_VOODOO_TYPE), LVCFMT_LEFT, 80); 
	InsertColumn(4, GetResString(IDS_X_VOODOO_PERM), LVCFMT_LEFT, 50); 
	InsertColumn(5, GetResString(IDS_X_VOODOO_STATUS), LVCFMT_LEFT, 100); 
	InsertColumn(6, GetResString(IDS_X_VOODOO_SPEED), LVCFMT_LEFT, 100); 

	LoadSettings();
	SetSortArrow();
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 1)));
}

void CVoodooListCtrl::LoadVoodooItem(CVoodooClient* client)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int iItem = FindItem(&find);
	if (iItem == -1)
		iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,0,client->GetName(),0,0,1,(LPARAM)client);

	CString strBuff;

	strBuff = client->GetName();
	SetItemText(iItem, 0, strBuff); 

	strBuff = client->GetAddress();
	strBuff.AppendFormat(_T(":%u"),client->GetPort());
	in_addr iaDest;	iaDest.s_addr = inet_addr(CT2CA(client->GetAddress()));
	if (iaDest.s_addr == INADDR_NONE && client->GetIP() != 0)
		strBuff.AppendFormat(_T(" (%s)"),ipstr(client->GetIP()));
	SetItemText(iItem, 1, strBuff); 

	strBuff = GetResString((client->GetAction() == VA_PARTNER) ? IDS_X_VOODOO_PARTNER : (client->GetAction() == VA_MASTER) ? IDS_X_VOODOO_MASTER : (client->GetAction() == VA_SLAVE) ? IDS_X_VOODOO_SLAVE : IDS_X_VOODOO_NOTHING);
	SetItemText(iItem, 2, strBuff); 

	strBuff = GetResString((client->GetType() == CT_ED2K) ? IDS_X_VOODOO_ED2K : (client->GetType() == CT_BITTORRENT) ? IDS_X_VOODOO_BITTORRENT : IDS_X_VOODOO_OTHER);
	SetItemText(iItem, 3, strBuff); 

	strBuff = GetResString((client->GetPerm() == VA_PARTNER) ? IDS_X_VOODOO_PARTNER : (client->GetPerm() == VA_MASTER) ? IDS_X_VOODOO_MASTER : (client->GetPerm() == VA_SLAVE) ? IDS_X_VOODOO_SLAVE : IDS_X_VOODOO_NOTHING);
	SetItemText(iItem, 4, strBuff);

	if(client->socket){
		if(client->socket->IsMaster() && client->socket->IsSlave())
			strBuff = GetResString(IDS_X_VOODOO_PARTNER);
		else if(client->socket->IsMaster())
			strBuff = GetResString(IDS_X_VOODOO_MASTER);
		else if(client->socket->IsSlave())
			strBuff = GetResString(IDS_X_VOODOO_SLAVE);
		else
			strBuff = GetResString(IDS_X_VOODOO_NOTHING);
	}else
		strBuff.Empty();
	SetItemText(iItem, 5, strBuff); 

	if(client->socket)
		strBuff.Format(_T("%.2f/%.2f"), (float)client->socket->GetUpDatarate()/1000, (float)client->socket->GetDownDatarate()/1000);
	else
		strBuff.Empty();
	SetItemText(iItem, 6, strBuff); 
}

int CVoodooListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CVoodooClient* item1 = (CVoodooClient*)lParam1;
	const CVoodooClient* item2 = (CVoodooClient*)lParam2;
	if (item1 == NULL || item2 == NULL)
		return 0;

	if(LOWORD(lParamSort) > 4)
	{
		if (item1->socket == NULL || item2->socket == NULL)
		{
			if(item1->socket != NULL)
				return HIWORD(lParamSort) ? -1 : 1;
			else if(item2->socket != NULL)
				return HIWORD(lParamSort) ? 1 : -1;
			else
				return 0;
		}
	}

	int iResult;
	switch (LOWORD(lParamSort))
	{
		case 0:
			iResult = StrCmp(item1->GetName(),item2->GetName());
			break;
		case 1:
			iResult = StrCmp(item1->GetAddress(),item2->GetAddress());
			if(iResult == 0)
				iResult = CompareUnsigned(item1->GetPort(), item2->GetPort());
			if(iResult == 0 && item1->GetIP() && item2->GetIP())
			{
				if (item1->GetIP() < item2->GetIP())
					iResult = -1;
				else if (item1->GetIP() > item2->GetIP())
					iResult = 1;
				else
					iResult = 0;
			}
			break;
		case 2:
			if (item1->GetAction() < item2->GetAction())
				iResult = -1;
			else if (item1->GetAction() > item2->GetAction())
				iResult = 1;
			else
				iResult = 0;
			break;
		case 3:
			if (item1->GetType() < item2->GetType())
				iResult = -1;
			else if (item1->GetType() > item2->GetType())
				iResult = 1;
			else
				iResult = 0;
			break;
		case 4:
			if (item1->GetPerm() && !item2->GetPerm())
				iResult = -1;
			else if (!item1->GetPerm() && item2->GetPerm())
				iResult = 1;
			else
				iResult = 0;
			break;
		case 5:
			{
				int State1;
				if(item1->socket->IsMaster() && item1->socket->IsSlave())
					State1 = 2;
				else if(item1->socket->IsMaster())
					State1 = 3;
				else if(item1->socket->IsSlave())
					State1 = 1;
				else
					State1 = 0;

				int State2;
				if(item2->socket->IsMaster() && item2->socket->IsSlave())
					State2 = 2;
				else if(item2->socket->IsMaster())
					State2 = 3;
				else if(item2->socket->IsSlave())
					State2 = 1;
				else
					State2 = 0;

				if (State1 < State2)
					iResult = -1;
				else if (State1 > State2)
					iResult = 1;
				else
					iResult = 0;
			}
			break;
		case 6:
			{
				uint32 dr1 = item1->socket->GetUpDatarate() + item1->socket->GetDownDatarate();
				uint32 dr2 = item2->socket->GetUpDatarate() + item2->socket->GetDownDatarate();
				iResult = CompareUnsigned(dr1, dr2);
			}
			break;

		default:
			ASSERT(0);
			return 0;
	}
	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
}

void CVoodooListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMLV->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMLV->iSubItem;

	// Sort table
	UpdateSortHistory(MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

/*void CVoodooListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	UINT flag = MF_STRING;
	if (GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED) == -1)
		flag = MF_GRAYED;

	CTitleMenu popupMenu;
	popupMenu.CreatePopupMenu();
	popupMenu.AppendMenu(MF_STRING | flag, MP_COPYSELECTED, GetResString(IDS_COPY));

	GetPopupMenuPos(*this, point);
	popupMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( popupMenu.DestroyMenu() );
}

BOOL CVoodooListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case MP_COPYSELECTED: {
			CString strText;
			POSITION posItem = GetFirstSelectedItemPosition();
			while (posItem) {
				int iItem = GetNextSelectedItem(posItem);
				if (iItem >= 0) {
					CString strComment = GetItemText(iItem, colComment);
					if (!strComment.IsEmpty()) {
						if (!strText.IsEmpty())
							strText += _T("\r\n");
						strText += strComment;
					}
				}
			}
			theApp.CopyTextToClipboard(strText);
			break;
		}
	}
	return CMuleListCtrl::OnCommand(wParam, lParam);
}*/

/*void CVoodooListCtrl::OnLvnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	delete (SComment*)pNMLV->lParam;
	*pResult = 0;
}*/

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --