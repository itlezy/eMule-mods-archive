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
#include <share.h>
#include "emule.h"
#include "ServerListCtrl.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "DownloadQueue.h"
#include "ServerList.h"
#include "Server.h"
#include "Sockets.h"
#include "MenuCmds.h"
#include "ServerWnd.h"
#include "Opcodes.h"
#include "Log.h"
#include "ToolTipCtrlX.h"
#include "MemDC.h" // Xman
#include "IPFilter.h" 
#include "HttpDownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const UINT colStrID[]={
	 IDS_SL_SERVERNAME
	,IDS_IP
	,IDS_DESCRIPTION
	,IDS_PING
	,IDS_UUSERS
	,IDS_MAXCLIENT
	,IDS_PW_FILES
	,IDS_PREFERENCE
	,IDS_UFAILED
	,IDS_STATICSERVER
	,IDS_SOFTFILES
	,IDS_HARDFILES
	,IDS_VERSION
	,IDS_IDLOW
	,IDS_OBFUSCATION
};
IMPLEMENT_DYNAMIC(CServerListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CServerListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNmCustomDraw)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	//ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CServerListCtrl::CServerListCtrl()
{
	m_tooltip = new CToolTipCtrlX;
}

void CServerListCtrl::SetGridLine()
{
 if (thePrefs.UseGridlines())
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES); 
   else
     SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

bool CServerListCtrl::Init()
{
	SetPrefsKey(_T("ServerListCtrl"));
	SetGridLine();
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip) {
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		//tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}

	InsertColumn(0, GetResString(IDS_SL_SERVERNAME),LVCFMT_LEFT, 150);
	InsertColumn(1, GetResString(IDS_IP),			LVCFMT_LEFT, 140);
	InsertColumn(2, GetResString(IDS_DESCRIPTION),	LVCFMT_LEFT, 150);
	InsertColumn(3, GetResString(IDS_PING),			LVCFMT_RIGHT, 50);
	InsertColumn(4, GetResString(IDS_UUSERS),		LVCFMT_RIGHT, 60);
	InsertColumn(5, GetResString(IDS_MAXCLIENT),	LVCFMT_RIGHT, 60);
	InsertColumn(6, GetResString(IDS_PW_FILES) ,	LVCFMT_RIGHT, 60);
	InsertColumn(7, GetResString(IDS_PREFERENCE),	LVCFMT_LEFT,  50);
	InsertColumn(8, GetResString(IDS_UFAILED),		LVCFMT_RIGHT, 50);
	InsertColumn(9, GetResString(IDS_STATICSERVER),	LVCFMT_LEFT,  50);
	InsertColumn(10,GetResString(IDS_SOFTFILES),	LVCFMT_RIGHT, 60);
	InsertColumn(11,GetResString(IDS_HARDFILES),	LVCFMT_RIGHT, 60, -1, true);
	InsertColumn(12,GetResString(IDS_VERSION),		LVCFMT_LEFT,  50, -1, true);
	InsertColumn(13,GetResString(IDS_IDLOW),		LVCFMT_RIGHT, 60);
	InsertColumn(14,GetResString(IDS_OBFUSCATION),  LVCFMT_RIGHT, 50);

	//SetAllIcons();
	ShowServerCount();//Localize();// X: [RUL] - [Remove Useless Localize]
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 0x0001)));

	return true;
} 

CServerListCtrl::~CServerListCtrl()
{
	delete m_tooltip;
}
/*
void CServerListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}
*/
/*void CServerListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.Add(CTempIconLoader(_T("Server")));
	HIMAGELIST himl = ApplyImageList(iml.Detach());
	if (himl)
		ImageList_Destroy(himl);
}*/

void CServerListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	size_t iItems = GetItemCount();
	for (size_t i = 0; i < iItems; i++)
		RefreshServer((CServer*)GetItemData(i));
	ShowServerCount();
}

void CServerListCtrl::RemoveServer(const CServer* pServer)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pServer;
	int iItem = FindItem(&find);
	if (iItem != -1) {
		theApp.serverlist->RemoveServer(pServer);
		DeleteItem(iItem); 
		ShowServerCount();
	}
}

void CServerListCtrl::RemoveAllDeadServers()
{
	ShowWindow(SW_HIDE);
	for (POSITION pos = theApp.serverlist->list.GetHeadPosition(); pos != NULL; )
	{
		const CServer* cur_server = theApp.serverlist->list.GetNext(pos);
		if (cur_server->GetFailedCount() >= thePrefs.GetDeadServerRetries())
		{
				//Xman
				// Mighty Knife: Static server handling
				// Static servers can be prevented from being removed from the list.
				if ((!cur_server->IsStaticMember()) || (!thePrefs.GetDontRemoveStaticServers())) {
					RemoveServer(cur_server);
					pos = theApp.serverlist->list.GetHeadPosition();
				} // [end] Mighty Knife
		}
	}
	ShowWindow(SW_SHOW);
}

void CServerListCtrl::RemoveAllFilteredServers()
{
	//if (!thePrefs.GetFilterServerByIP())// X: [CI] - [Code Improvement]
		//return;
	ShowWindow(SW_HIDE);
	for (POSITION pos = theApp.serverlist->list.GetHeadPosition(); pos != NULL; )
	{
		/*const */CServer* cur_server = theApp.serverlist->list.GetNext(pos);
		if (theApp.ipfilter->IsFiltered(cur_server->GetIP()))
		{
			if (thePrefs.GetFilterServerByIP()){
				if (thePrefs.GetLogFilteredIPs())
					AddDebugLogLine(false, _T("IPFilter(Updated): Filtered server \"%s\" (IP=%s) - IP filter"), cur_server->GetListName(), ipstr(cur_server->GetIP()));
				RemoveServer(cur_server);
				pos = theApp.serverlist->list.GetHeadPosition();
			}
			else
				cur_server->SetFiltered(true);// X: [CI] - [Code Improvement]
		}
		else
			cur_server->SetFiltered(false);// X: [CI] - [Code Improvement]
	}
	ShowWindow(SW_SHOW);
}

bool CServerListCtrl::AddServer(const CServer* pServer, bool bAddToList, bool bRandom)
{
	bool bAddTail = !bRandom || ((t_rng->getUInt16() % (1 + theApp.serverlist->GetServerCount())) != 0);
	if (!theApp.serverlist->AddServer(pServer, bAddTail))
		return false;
	if (bAddToList)
	{
		InsertItem(LVIF_TEXT | LVIF_PARAM, bAddTail ? GetItemCount() : 0, pServer->GetListName(), 0, 0, 0, (LPARAM)pServer);
		RefreshServer(pServer);
		ShowServerCount();
	}
	return true;
}

void CServerListCtrl::RefreshServer(const CServer* server)
{
	if (!server || !CemuleDlg::IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)server;
	int itemnr = FindItem(&find);
	if (itemnr == -1)
		return;

	CString temp;
	if(server->HasDynIP()){// X: [UIC] - [UIChange] show Dynip
		if(server->GetIP())
			temp.Format(_T("%s(%s)"), server->GetFullIP(), server->GetDynIP());
		else
			temp.Format(_T("%s"),  server->GetDynIP());
	}
	else
		temp.Format(_T("%s"),  server->GetFullIP());
	temp.Append(_T(" : "));
	//Morph Start - aux Ports, by lugdunummaster
	if (server->GetConnPort() != server->GetPort())
		temp.AppendFormat(_T("%i/%i"), server->GetPort(), server->GetConnPort());
	else
		temp.AppendFormat(_T("%i"), server->GetPort());
	//Morph End - added by [ionix], aux Ports, by lugdunummaster
	SetItemText(itemnr, 1, temp);
	SetItemText(itemnr, 0, server->GetListName());
	SetItemText(itemnr, 2, server->GetDescription());

	// Ping
	if (server->GetPing()) {
		temp.Format(_T("%i"), server->GetPing());
		SetItemText(itemnr, 3, temp);
	}
	else
		SetItemText(itemnr, 3, _T(""));

	// Users
	SetItemText(itemnr, 4, server->GetUsers()?CastItoIShort(server->GetUsers()):_T(""));

	// Max Users
	SetItemText(itemnr, 5, server->GetMaxUsers()?CastItoIShort(server->GetMaxUsers()):_T(""));

	// Files
	SetItemText(itemnr, 6, server->GetFiles()?CastItoIShort(server->GetFiles()):_T(""));

	UINT StrID = IDS_PRIONOPREF;
	switch(server->GetPreference()){
		case SRV_PR_LOW:
			StrID = IDS_PRIOLOW;
			break;
		case SRV_PR_NORMAL:
			StrID = IDS_PRIONORMAL;
			break;
		case SRV_PR_HIGH:
			StrID = IDS_PRIOHIGH;
			break;
	}
	SetItemText(itemnr, 7, GetResString(StrID));
	
	// Failed Count
	temp.Format(_T("%i"), server->GetFailedCount());
	SetItemText(itemnr, 8, temp);

	// Static server
	SetItemText(itemnr, 9, GetResString((server->IsStaticMember())?IDS_YES:IDS_NO)); 

	// Soft Files
	SetItemText(itemnr, 10, server->GetSoftFiles()?CastItoIShort(server->GetSoftFiles()):_T(""));

	// Hard Files
	SetItemText(itemnr, 11, server->GetHardFiles()?CastItoIShort(server->GetHardFiles()):_T(""));

	temp = server->GetVersion();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugServerUDPLevel() > 0) {
		if (server->GetUDPFlags() != 0) {
			if (!temp.IsEmpty())
				temp += _T("; ");
			temp.AppendFormat(_T("ExtUDP=%x"), server->GetUDPFlags());
		}
	}
	if (thePrefs.GetDebugServerTCPLevel() > 0) {
		if (server->GetTCPFlags() != 0) {
			if (!temp.IsEmpty())
				temp += _T("; ");
			temp.AppendFormat(_T("ExtTCP=%x"), server->GetTCPFlags());
		}
	}
#endif
	SetItemText(itemnr, 12, temp);

	// LowID Users
	SetItemText(itemnr, 13, server->GetLowIDUsers()?CastItoIShort(server->GetLowIDUsers()):_T(""));

	// Obfuscation
	SetItemText(itemnr, 14, GetResString((server->SupportsObfuscationTCP() && server->GetObfuscationPortTCP() != 0)?IDS_YES:IDS_NO));
}

void CServerListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{ 
	if (point.x != -1 || point.y != -1) {// X: [BF] - [Bug Fix]
		RECT rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!PtInRect(&rcClient,point)) {
			Default();
			return;
		}
	}
	// get merged settings
	bool bFirstItem = true;
	size_t iSelectedItems = GetSelectedCount();
	size_t iStaticServers = 0;
	UINT uPrioMenuItem = 0;
	POSITION pos = GetFirstSelectedItemPosition();
	sint_ptr iCanConnect = iSelectedItems; // X-Ray :: FiXeS :: Obfuscation-Bugfix :: WiZaRd
	while (pos)
	{
		const CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));

		// X-Ray :: FiXeS :: Obfuscation-Bugfix :: Start :: WiZaRd
		//if we REQUIRE a crypted connection but a selected server does not support obfuscation then we won't be able to connect!
		if(thePrefs.IsClientCryptLayerRequired() && !pServer->SupportsObfuscationTCP())
			iCanConnect--;
		// X-Ray :: FiXeS :: Obfuscation-Bugfix :: End :: WiZaRd

		if(pServer->IsStaticMember())
			++iStaticServers;

		UINT uCurPrioMenuItem = 0;
		if (pServer->GetPreference() == SRV_PR_LOW)
			uCurPrioMenuItem = MP_PRIOLOW;
		else if (pServer->GetPreference() == SRV_PR_NORMAL)
			uCurPrioMenuItem = MP_PRIONORMAL;
		else if (pServer->GetPreference() == SRV_PR_HIGH)
			uCurPrioMenuItem = MP_PRIOHIGH;
		else
			ASSERT(0);

		if (bFirstItem)
			uPrioMenuItem = uCurPrioMenuItem;
		else if (uPrioMenuItem != uCurPrioMenuItem)
			uPrioMenuItem = 0;

		bFirstItem = false;
	}

	CMenu ServerMenu;
	ServerMenu.CreatePopupMenu();

	// X-Ray :: FiXeS :: Obfuscation-Bugfix :: Start :: WiZaRd
	/*
	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_CONNECTTO, GetResString(IDS_CONNECTTHIS), _T("CONNECT"));
	*/
	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 && iCanConnect > 0 ? MF_ENABLED : MF_GRAYED), MP_CONNECTTO, GetResString(IDS_CONNECTTHIS));
	// X-Ray :: FiXeS :: Obfuscation-Bugfix :: End :: WiZaRd
	ServerMenu.SetDefaultItem(iSelectedItems > 0 ? MP_CONNECTTO : -1);
	ServerMenu.AppendMenu(MF_STRING | (theApp.serverconnect->IsConnecting() || theApp.serverconnect->IsConnected() ? MF_ENABLED : MF_GRAYED), MP_DISCONNECT, GetResString(IDS_MAIN_BTN_DISCONNECT));

	CMenu ServerPrioMenu;
	ServerPrioMenu.CreateMenu();
	if (iSelectedItems > 0){
		ServerPrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
		ServerPrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
		ServerPrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
		ServerPrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOHIGH, uPrioMenuItem, 0);
	}
	ServerMenu.AppendMenu(MF_POPUP  | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)ServerPrioMenu.m_hMenu, GetResString(IDS_PRIORITY));

	// enable add/remove from static server list, if there is at least one selected server which can be used for the action
	ServerMenu.AppendMenu(MF_STRING | (iStaticServers < iSelectedItems ? MF_ENABLED : MF_GRAYED), MP_ADDTOSTATIC, GetResString(IDS_ADDTOSTATIC));
	ServerMenu.AppendMenu(MF_STRING | (iStaticServers > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVEFROMSTATIC, GetResString(IDS_REMOVEFROMSTATIC));
	ServerMenu.AppendMenu(MF_SEPARATOR);

	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_GETED2KLINK, GetResString(IDS_DL_LINK1));
	ServerMenu.AppendMenu(MF_STRING | (theApp.IsEd2kServerLinkInClipboard() ? MF_ENABLED : MF_GRAYED), MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD));
	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVE, GetResString(IDS_REMOVETHIS));
	ServerMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVEALL, GetResString(IDS_REMOVEALL));
	ServerMenu.AppendMenu(MF_SEPARATOR);
	ServerMenu.AppendMenu(MF_STRING, MP_UPDATE_SERVER, GetResString(IDS_SV_SERVERLIST) + _T(' ') + GetResString(IDS_SV_UPDATE));

	GetPopupMenuPos(*this, point);
	ServerMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	VERIFY( ServerPrioMenu.DestroyMenu() );
	VERIFY( ServerMenu.DestroyMenu() );
}

BOOL CServerListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
	case MP_CONNECTTO:
	case IDA_ENTER:
      if (GetSelectedCount() > 1)
      {
               theApp.serverconnect->Disconnect();
         POSITION pos = GetFirstSelectedItemPosition();
			while (pos != NULL)
			{
				int iItem = GetNextSelectedItem(pos);
				if (iItem > -1) {
					const CServer* pServer = (CServer*)GetItemData(iItem);

					// X-Ray :: FiXeS :: Obfuscation-Bugfix :: Start :: WiZaRd
					//if we REQUIRE a crypted connection but a selected server does not support obfuscation then we won't be able to connect!
					if(thePrefs.IsClientCryptLayerRequired() && !pServer->SupportsObfuscationTCP())
						continue;
					// X-Ray :: FiXeS :: Obfuscation-Bugfix :: End :: WiZaRd

					theApp.serverlist->MoveServerDown(pServer);
				}
			}
			theApp.serverconnect->ConnectToAnyServer(theApp.serverlist->GetServerCount() - GetSelectedCount(), false, false);
		}
		else
		{
			int iItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
			if (iItem > -1){
				// X-Ray :: FiXeS :: Obfuscation-Bugfix :: Start :: WiZaRd
				/*
				theApp.serverconnect->ConnectToServer((CServer*)GetItemData(iItem));
				*/
				//have to add this because a user might trigger a connection using "ENTER"
				CServer* pServer = (CServer*)GetItemData(iItem);
				//if we REQUIRE a crypted connection but a selected server does not support obfuscation then we won't be able to connect!
				if(!thePrefs.IsClientCryptLayerRequired() || pServer->SupportsObfuscationTCP())
					theApp.serverconnect->ConnectToServer(pServer);
				// X-Ray :: FiXeS :: Obfuscation-Bugfix :: End :: WiZaRd
			}
		}
		theApp.emuledlg->ShowConnectionState();
		return TRUE;

	case MP_DISCONNECT: {
		theApp.serverconnect->Disconnect();
		return TRUE;
	}
	case MP_CUT: {
		CString strURLs = CreateSelectedServersURLs();
		if (!strURLs.IsEmpty())
			theApp.CopyTextToClipboard(strURLs);
		DeleteSelectedServers();
		return TRUE;
	}

	case MP_COPYSELECTED:
	case MP_GETED2KLINK:{
		CString strURLs = CreateSelectedServersURLs();
		if (!strURLs.IsEmpty())
			theApp.CopyTextToClipboard(strURLs);
		return TRUE;
	}

	case MP_PASTE:
		if (theApp.IsEd2kServerLinkInClipboard())
			theApp.emuledlg->serverwnd->PasteServerFromClipboard();
		return TRUE;

	case MP_REMOVE:
	case MPG_DELETE: {
		SetRedraw(FALSE);
		while (GetFirstSelectedItemPosition() != NULL)
		{
			POSITION pos = GetFirstSelectedItemPosition();
			int iItem = GetNextSelectedItem(pos);
			theApp.serverlist->RemoveServer((CServer*)GetItemData(iItem));
			DeleteItem(iItem);
		}
		ShowServerCount();
		SetRedraw(TRUE);
		SetFocus();
		AutoSelectItem();
		return TRUE;
	}

     case MP_UPDATE_SERVER:
	      {
		   CString strURL;
           strURL = thePrefs.GetServerMetUpdateURL();
		  CString strConfirm;
		  strConfirm.Format(GetResString(IDS_CONFIRMSERVERDOWNLOAD), strURL);
		  if(strURL.GetLength() != 0 && AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
		   UpdateServerMetFromURL(strURL);
		return TRUE;
	}

	case MP_REMOVEALL:
		if (AfxMessageBox(GetResString(IDS_REMOVEALLSERVERS), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
			return TRUE;
		if (theApp.serverconnect->IsConnecting()) {
			theApp.downloadqueue->StopUDPRequests();
			theApp.serverconnect->StopConnectionTry();
			theApp.serverconnect->Disconnect();
			theApp.emuledlg->ShowConnectionState();
		}
		ShowWindow(SW_HIDE);
		theApp.serverlist->RemoveAllServers();
		DeleteAllItems();
		ShowWindow(SW_SHOW);
		ShowServerCount();
		return TRUE;

	case MP_ADDTOSTATIC: {
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos != NULL) {
			CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));
			if (!StaticServerFileAppend(pServer))
				return FALSE;
			RefreshServer(pServer);
		}
		return TRUE;
						 }

	case MP_REMOVEFROMSTATIC: {
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos != NULL) {
			CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));
			if (!StaticServerFileRemove(pServer))
				return FALSE;
			RefreshServer(pServer);
		}
		return TRUE;
	}

	case MP_PRIOLOW:
		SetSelectedServersPriority(SRV_PR_LOW);
		return TRUE;

	case MP_PRIONORMAL:
		SetSelectedServersPriority(SRV_PR_NORMAL);
		return TRUE;

	case MP_PRIOHIGH:
		SetSelectedServersPriority(SRV_PR_HIGH);
		return TRUE;
	}
	return FALSE;
}

CString CServerListCtrl::CreateSelectedServersURLs()
{
	POSITION pos = GetFirstSelectedItemPosition();
	CString buffer, link;
	while (pos != NULL) {
		const CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));
		buffer.Format(_T("ed2k://|server|%s|%u|/"), pServer->GetAddress(), pServer->GetPort());
		if (link.GetLength() > 0)
			buffer = _T("\r\n") + buffer;
		link += buffer;
	}
	return link;
}

void CServerListCtrl::DeleteSelectedServers()
{
	//SetRedraw(FALSE);
	while (GetFirstSelectedItemPosition() != NULL)
	{
		POSITION pos = GetFirstSelectedItemPosition();
		int iItem = GetNextSelectedItem(pos);
		theApp.serverlist->RemoveServer((CServer*)GetItemData(iItem));
		DeleteItem(iItem);
	}
	ShowServerCount();
	//SetRedraw(TRUE);
	SetFocus();
	AutoSelectItem();
}

void CServerListCtrl::SetSelectedServersPriority(UINT uPriority)
{
	bool bUpdateStaticServersFile = false;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));
		if (pServer->GetPreference() != uPriority)
		{
			pServer->SetPreference(uPriority);
			if (pServer->IsStaticMember())
				bUpdateStaticServersFile = true;
			RefreshServer(pServer);
		}
	}
	if (bUpdateStaticServersFile)
		theApp.serverlist->SaveStaticServers();
}

void CServerListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		theApp.serverconnect->ConnectToServer((CServer*)GetItemData(iSel));
	   	theApp.emuledlg->ShowConnectionState();
	}
}

bool CServerListCtrl::AddServerMetToList(const CString& strFile)
{
	SetRedraw(FALSE);
	bool bResult = theApp.serverlist->AddServerMetToList(strFile, true);
	RemoveAllDeadServers();
	ShowServerCount();
	SetRedraw(TRUE);
	return bResult;
}

void CServerListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	if (GetSortItem() != pNMListView->iSubItem)
	{
		switch (pNMListView->iSubItem)
		{
			/*case 4: // Users
			case 5: // Max Users
			case 6: // Files
			case 7: // Priority
			case 9: // Static
			case 10: // Soft Files
			case 11: // Hard Files
			case 12: // Version
			case 13: // Low IDs
			case 14: // Obfuscation
				sortAscending = false;
				break;
			default:
				sortAscending = true;
				break;*/
			case 0:
			case 1:
			case 2:
			case 3:
			case 8:
				sortAscending = true;
				break;
			default:
				sortAscending = false;
				break;
		}
	}
	else
		sortAscending = !GetSortAscending();

	// Sort table
	UpdateSortHistory(MAKELONG(pNMListView->iSubItem, (sortAscending ? 0 : 0x0001)));
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, MAKELONG(pNMListView->iSubItem, (sortAscending ? 0 : 0x0001)));
	Invalidate();
	*pResult = 0;
}

int CServerListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CServer* item1 = (CServer*)lParam1;
	const CServer* item2 = (CServer*)lParam2;
	if (item1 == NULL || item2 == NULL)
		return 0;

#define UNDEFINED_STR_AT_BOTTOM(s1, s2) \
	if ((s1).IsEmpty() && (s2).IsEmpty()) \
		return 0;						\
	if ((s1).IsEmpty())					\
		return 1;						\
	if ((s2).IsEmpty())					\
		return -1;						\

#define UNDEFINED_INT_AT_BOTTOM(i1, i2) \
	if ((i1) == (i2))					\
		return 0;						\
	if ((i1) == 0)						\
		return 1;						\
	if ((i2) == 0)						\
		return -1;						\

	int iResult;
	switch (LOWORD(lParamSort))
	{
	  case 0:
		  UNDEFINED_STR_AT_BOTTOM(item1->GetListName(), item2->GetListName());
		  iResult = item1->GetListName().CompareNoCase(item2->GetListName());
		  break;

	  case 1:
		  if (item1->HasDynIP() && item2->HasDynIP())
			  iResult = item1->GetDynIP().CompareNoCase(item2->GetDynIP());
		  else if (item1->HasDynIP())
			  iResult = -1;
		  else if (item2->HasDynIP())
			  iResult = 1;
		  else{
			  uint32 uIP1 = htonl(item1->GetIP());
			  uint32 uIP2 = htonl(item2->GetIP());
			  if (uIP1 < uIP2)
				  iResult = -1;
			  else if (uIP1 > uIP2)
				  iResult = 1;
			  else
				  iResult = CompareUnsigned(item1->GetPort(), item2->GetPort());
		  }
		  break;

	  case 2:
		  UNDEFINED_STR_AT_BOTTOM(item1->GetDescription(), item2->GetDescription());
		  iResult = item1->GetDescription().CompareNoCase(item2->GetDescription());
		  break;

	  case 3:
		  UNDEFINED_INT_AT_BOTTOM(item1->GetPing(), item2->GetPing());
		  iResult = CompareUnsigned(item1->GetPing(), item2->GetPing());
		  break;

	  case 4:
		  UNDEFINED_INT_AT_BOTTOM(item1->GetUsers(), item2->GetUsers());
		  iResult = CompareUnsigned(item1->GetUsers(), item2->GetUsers());
		  break;

	  case 5:
		  UNDEFINED_INT_AT_BOTTOM(item1->GetMaxUsers(), item2->GetMaxUsers());
		  iResult = CompareUnsigned(item1->GetMaxUsers(), item2->GetMaxUsers());
		  break;

	  case 6:
		  UNDEFINED_INT_AT_BOTTOM(item1->GetFiles(), item2->GetFiles());
		  iResult = CompareUnsigned(item1->GetFiles(), item2->GetFiles());
		  break;

	  case 7:
		  if (item2->GetPreference() == item1->GetPreference())
			  return 0;
		  if (item2->GetPreference() == SRV_PR_LOW)
			  iResult = 1;
		  else if (item1->GetPreference() == SRV_PR_LOW)
			  iResult = -1;
		  else if (item2->GetPreference() == SRV_PR_HIGH)
			  iResult = -1;
		  else if (item1->GetPreference() == SRV_PR_HIGH)
			  iResult = 1;
		  else
			  return 0;
		  break;

	  case 8:
		  iResult = CompareUnsigned(item1->GetFailedCount(), item2->GetFailedCount());
		  break;

	  case 9:
		  iResult = (int)item1->IsStaticMember() - (int)item2->IsStaticMember();
		  break;

	  case 10:  
		  UNDEFINED_INT_AT_BOTTOM(item1->GetSoftFiles(), item2->GetSoftFiles());
		  iResult = CompareUnsigned(item1->GetSoftFiles(), item2->GetSoftFiles());
		  break;

	  case 11: 
		  UNDEFINED_INT_AT_BOTTOM(item1->GetHardFiles(), item2->GetHardFiles());
		  iResult = CompareUnsigned(item1->GetHardFiles(), item2->GetHardFiles());
		  break;

	  case 12:
		  UNDEFINED_STR_AT_BOTTOM(item1->GetVersion(), item2->GetVersion());
		  iResult = item1->GetVersion().CompareNoCase(item2->GetVersion());
		  break;

	  case 13:
		  UNDEFINED_INT_AT_BOTTOM(item1->GetLowIDUsers(), item2->GetLowIDUsers());
		  iResult = CompareUnsigned(item1->GetLowIDUsers(), item2->GetLowIDUsers());
		  break;
	  case 14: 
		  iResult = (int)(item1->SupportsObfuscationTCP() && item1->GetObfuscationPortTCP() != 0) - (int)(item2->SupportsObfuscationTCP() && item2->GetObfuscationPortTCP() != 0);
		  break;
	} 
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->serverwnd->serverlistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	if (HIWORD(lParamSort))
		return -iResult;
	return iResult;
}

bool CServerListCtrl::UpdateServerMetFromURL(CString strURL)
{
	if (strURL.IsEmpty() || (strURL.Find(_T("://")) == -1)) {
		// not a valid URL
		LogError(LOG_STATUSBAR, GetResString(IDS_INVALIDURL) );
		return false;
	}

	// add entered URL to LRU list even if it's not yet known whether we can download from this URL (it's just more convenient this way)
	//if (m_pacServerMetURL && m_pacServerMetURL->IsBound())
	//	m_pacServerMetURL->AddItem(strURL, 0);

	CString strTempFilename;
	strTempFilename.Format(_T("%stemp-%d-server.met"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ::GetTickCount());

	// try to download server.met
	Log(GetResString(IDS_DOWNLOADING_SERVERMET_FROM), strURL);
	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_DOWNLOADING_SERVERMET);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = strTempFilename;
	if (dlgDownload.DoModal() != IDOK) {
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FAILEDDOWNLOADMET), strURL);
		return false;
	}

	// add content of server.met to serverlist
	ShowWindow(SW_HIDE);
	AddServerMetToList(strTempFilename);
	ShowWindow(SW_SHOW);
	(void)_tremove(strTempFilename);
	return true;
}

bool CServerListCtrl::StaticServerFileAppend(CServer *server)
{
	bool bResult;
	AddLogLine(false, _T("'%s:%i,%s' %s"), server->GetAddress(), server->GetPort(), server->GetListName(), GetResString(IDS_ADDED2SSF));
	server->SetIsStaticMember(true);
	bResult = theApp.serverlist->SaveStaticServers();
	RefreshServer(server);
	return bResult;
}

bool CServerListCtrl::StaticServerFileRemove(CServer *server)
{
	if (!server->IsStaticMember())
		return true;
	server->SetIsStaticMember(false);
	return theApp.serverlist->SaveStaticServers();
}

void CServerListCtrl::ShowServerCount()
{
	CString text;
	text.AppendFormat(_T("%s (%i)"), GetResString(IDS_SV_SERVERLIST), GetItemCount());
	theApp.emuledlg->serverwnd->SetDlgItemText(IDC_SERVLIST_TEXT, text);
}

void CServerListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		bool bOverMainItem = (SubItemHitTest(&hti) != -1 && hti.iItem == pGetInfoTip->iItem && hti.iSubItem == 0);

		// those tooltips are very nice for debugging/testing but pretty annoying for general usage
		// enable tooltips only if Ctrl is currently pressed
		bool bShowInfoTip = bOverMainItem && (GetSelectedCount() > 1 || (GetKeyState(VK_CONTROL) & 0x8000));
		if (bShowInfoTip && GetSelectedCount() > 1)
		{
			// Don't show the tooltip if the mouse cursor is not over at least one of the selected items
			bool bInfoTipItemIsPartOfMultiSelection = false;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos) {
				if (GetNextSelectedItem(pos) == pGetInfoTip->iItem) {
					bInfoTipItemIsPartOfMultiSelection = true;
					break;
				}
			}
			if (!bInfoTipItemIsPartOfMultiSelection)
				bShowInfoTip = false;
		}

		if (!bShowInfoTip) {
			if (!bOverMainItem) {
				// don't show the default label tip for the main item, if the mouse is not over the main item
				if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
					pGetInfoTip->pszText[0] = _T('\0');
			}
			return;
		}

		if (GetSelectedCount() > 1)
		{
			int iSelected = 0;
			ULONGLONG ulTotalUsers = 0;
			ULONGLONG ulTotalLowIdUsers = 0;
			ULONGLONG ulTotalFiles = 0;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));
				if (pServer)
				{
					iSelected++;
					ulTotalUsers += pServer->GetUsers();
					ulTotalFiles += pServer->GetFiles();
					ulTotalLowIdUsers += pServer->GetLowIDUsers();
				}
			}

			if (iSelected > 0)
			{
				CString strInfo;
				strInfo.Format(_T("%s: %u\r\n%s: %s\r\n%s: %s\r\n%s: %s"), 
					GetResString(IDS_FSTAT_SERVERS), iSelected, 
					GetResString(IDS_UUSERS), CastItoIShort(ulTotalUsers),
					GetResString(IDS_IDLOW), CastItoIShort(ulTotalLowIdUsers),
					GetResString(IDS_PW_FILES), CastItoIShort(ulTotalFiles));
				strInfo += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
				_tcsncpy(pGetInfoTip->pszText, strInfo, pGetInfoTip->cchTextMax);
				pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
			}
		}
	}

	*pResult = 0;
}

void CServerListCtrl::OnNmCustomDraw(NMHDR *pNMHDR, LRESULT *plResult)
{
   LPNMLVCUSTOMDRAW pnmlvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

   if (pnmlvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
   {
      *plResult = CDRF_NOTIFYITEMDRAW;
      return;
   }

   if (pnmlvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
   {
      const CServer* pServer = (const CServer*)pnmlvcd->nmcd.lItemlParam;
		const CServer* pConnectedServer = theApp.serverconnect->GetCurrentServer();
		// the server which we are connected to always has a valid numerical IP member assigned when it is not DynIP,
		// therefor we do not need to call CServer::IsEqual which would be expensive
      //if (pConnectedServer && pConnectedServer->IsEqual(pServer))
		if (pConnectedServer && pConnectedServer->GetPort() == pServer->GetPort()
			&& (pConnectedServer->GetIP()?
				pConnectedServer->GetIP() == pServer->GetIP()
			:
				(pServer->HasDynIP() && pConnectedServer->GetDynIP().Compare(pServer->GetDynIP()) == 0)
			))
         pnmlvcd->clrText = RGB(32,32,255);
		else if (pServer->GetFailedCount() >= thePrefs.GetDeadServerRetries())
         pnmlvcd->clrText = RGB(192,192,192);
		else if (pServer->GetFailedCount() >= 2)
         pnmlvcd->clrText = RGB(128,128,128);
   }

   *plResult = CDRF_DODEFAULT;
}
