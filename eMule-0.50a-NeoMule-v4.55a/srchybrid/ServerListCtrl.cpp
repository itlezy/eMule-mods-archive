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
#include <io.h>
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
#include "IrcWnd.h"
#include "Opcodes.h"
#include "Log.h"
#include "ToolTipCtrlX.h"
#include "IPFilter.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
#include "Neo/GUI/IP2Country.h"
#include "MemDC.h"
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CServerListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CServerListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblClk)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
#ifndef IP2COUNTRY // NEO: IP2C -- Xanatos -->
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomDraw)
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
END_MESSAGE_MAP()

CServerListCtrl::CServerListCtrl()
{
	SetGeneralPurposeFind(true);
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	m_tooltip = new CToolTipCtrlX;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
}

bool CServerListCtrl::Init()
{
	SetName(_T("ServerListCtrl"));

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip) {
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		//tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	InsertColumn(0, GetResString(IDS_SL_SERVERNAME),LVCFMT_LEFT, 150);
	InsertColumn(1, GetResString(IDS_IP),			LVCFMT_LEFT, 140);
	InsertColumn(2, GetResString(IDS_DESCRIPTION),	LVCFMT_LEFT, 150);
	InsertColumn(3, GetResString(IDS_PING),			LVCFMT_RIGHT, 50);
	InsertColumn(4, GetResString(IDS_UUSERS),		LVCFMT_RIGHT, 50);
	InsertColumn(5, GetResString(IDS_MAXCLIENT),	LVCFMT_RIGHT, 50);
	InsertColumn(6, GetResString(IDS_PW_FILES) ,	LVCFMT_RIGHT, 50);
	InsertColumn(7, GetResString(IDS_PREFERENCE),	LVCFMT_LEFT,  60);
	InsertColumn(8, GetResString(IDS_UFAILED),		LVCFMT_RIGHT, 50);
	InsertColumn(9, GetResString(IDS_STATICSERVER),	LVCFMT_LEFT,  50);
	InsertColumn(10,GetResString(IDS_SOFTFILES),	LVCFMT_RIGHT, 50);
	InsertColumn(11,GetResString(IDS_HARDFILES),	LVCFMT_RIGHT, 50);
	InsertColumn(12,GetResString(IDS_VERSION),		LVCFMT_LEFT,  50);
	InsertColumn(13,GetResString(IDS_IDLOW),		LVCFMT_RIGHT, 50);
	InsertColumn(14,GetResString(IDS_OBFUSCATION)  ,LVCFMT_RIGHT, 50);
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	InsertColumn(15,GetResString(IDS_X_NAT_TRAVERSAL)  ,LVCFMT_RIGHT, 50);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	InsertColumn(16,GetResString(IDS_X_COUNTRY),      LVCFMT_LEFT, 100);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	SetAllIcons();
	Localize();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	PermanentSort(); // NEO: SE - [SortExtension] <-- Xanatos --
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending()? 0 : 0x0001)));

	ShowServerCount();

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	if (NeoPrefs.GetIP2CountryNameMode() == IP2CountryName_DISABLE)
		HideColumn (15);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	return true;
} 

CServerListCtrl::~CServerListCtrl()
{
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos 
	delete m_tooltip;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
}

void CServerListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CServerListCtrl::SetAllIcons()
{
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("Server")));
#else
	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader(_T("Server")));
	HIMAGELIST himl = ApplyImageList(iml.Detach());
	if (himl)
		ImageList_Destroy(himl);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
}

void CServerListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_SL_SERVERNAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_IP);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_DESCRIPTION);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_PING);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

 	strRes = GetResString(IDS_UUSERS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_MAXCLIENT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_PW_FILES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_PREFERENCE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	strRes = GetResString(IDS_UFAILED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);

	strRes = GetResString(IDS_STATICSERVER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);

	strRes = GetResString(IDS_SOFTFILES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);

	strRes = GetResString(IDS_HARDFILES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);

	strRes = GetResString(IDS_VERSION);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);

	strRes = GetResString(IDS_IDLOW);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	strRes = GetResString(IDS_OBFUSCATION);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	strRes = GetResString(IDS_X_NAT_TRAVERSAL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	strRes = GetResString(IDS_X_COUNTRY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		RefreshServer((CServer*)GetItemData(i));
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
		//if (cur_server->GetFailedCount() >= thePrefs.GetDeadServerRetries())
		if (cur_server->GetFailedCount() >= thePrefs.GetDeadServerRetries() 
		 && !(cur_server->IsStaticMember() && NeoPrefs.IsDontRemoveStaticServers()) // NEO: MOD - [DontRemoveStaticServers] <-- Xanatos --
		 || theApp.ipfilter->IsFiltered(cur_server->GetIP())) // NEO: MOD - [ServerIPFilter] <-- Xanatos --
		{
			RemoveServer(cur_server);
			pos = theApp.serverlist->list.GetHeadPosition();
		}
	}
	ShowWindow(SW_SHOW);
}

void CServerListCtrl::RemoveAllFilteredServers()
{
	if (!thePrefs.GetFilterServerByIP())
		return;
	ShowWindow(SW_HIDE);
	for (POSITION pos = theApp.serverlist->list.GetHeadPosition(); pos != NULL; )
	{
		const CServer* cur_server = theApp.serverlist->list.GetNext(pos);
		if (theApp.ipfilter->IsFiltered(cur_server->GetIP()))
		{
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Filtered server \"%s\" (IP=%s) - IP filter (%s)"), cur_server->GetListName(), ipstr(cur_server->GetIP()), theApp.ipfilter->GetLastHit());
			RemoveServer(cur_server);
			pos = theApp.serverlist->list.GetHeadPosition();
		}
	}
	ShowWindow(SW_SHOW);
}

bool CServerListCtrl::AddServer(const CServer* pServer, bool bAddToList, bool bRandom)
{
	bool bAddTail = !bRandom || ((GetRandomUInt16() % (1 + theApp.serverlist->GetServerCount())) != 0);
	if (!theApp.serverlist->AddServer(pServer, bAddTail))
		return false;
	if (bAddToList)
	{
		InsertItem(LVIF_TEXT | LVIF_PARAM, bAddTail ? GetItemCount() : 0, pServer->GetListName(), 0, 0, 1, (LPARAM)pServer);
		RefreshServer(pServer);
	}
	ShowServerCount();
	return true;
}

void CServerListCtrl::RefreshServer(const CServer* server)
{
	if (!server || !theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)server;
	int itemnr = FindItem(&find);
	if (itemnr == -1)
		return;

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	Update(itemnr);
#else // NEO: IP2C END <-- Xanatos --

	CString temp;
	temp.Format(_T("%s : %i"), server->GetAddress(), server->GetPort());
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
	if (server->GetUsers())
		SetItemText(itemnr, 4, CastItoIShort(server->GetUsers()));
	else
		SetItemText(itemnr, 4, _T(""));

	// Max Users
	if (server->GetMaxUsers())
		SetItemText(itemnr, 5, CastItoIShort(server->GetMaxUsers()));
	else
		SetItemText(itemnr, 5, _T(""));

	// Files
	if (server->GetFiles())
		SetItemText(itemnr, 6, CastItoIShort(server->GetFiles()));
	else
		SetItemText(itemnr, 6, _T(""));

	switch (server->GetPreference()) {
		case SRV_PR_LOW:
			SetItemText(itemnr, 7, GetResString(IDS_PRIOLOW));
			break;
		case SRV_PR_NORMAL:
			SetItemText(itemnr, 7, GetResString(IDS_PRIONORMAL));
			break;
		case SRV_PR_HIGH:
			SetItemText(itemnr, 7, GetResString(IDS_PRIOHIGH));
			break;
		default:
			SetItemText(itemnr, 7, GetResString(IDS_PRIONOPREF));
	}
	
	// Failed Count
	temp.Format(_T("%i"), server->GetFailedCount());
	SetItemText(itemnr, 8, temp);

	// Static server
	if (server->IsStaticMember())
		SetItemText(itemnr, 9, GetResString(IDS_YES)); 
	else
		SetItemText(itemnr, 9, GetResString(IDS_NO));

	// Soft Files
	if (server->GetSoftFiles())
		SetItemText(itemnr, 10, CastItoIShort(server->GetSoftFiles()));
	else
		SetItemText(itemnr, 10, _T(""));

	// Hard Files
	if (server->GetHardFiles())
		SetItemText(itemnr, 11, CastItoIShort(server->GetHardFiles()));
	else
		SetItemText(itemnr, 11, _T(""));

	temp = server->GetVersion();
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
	SetItemText(itemnr, 12, temp);

	// LowID Users
	if (server->GetLowIDUsers())
		SetItemText(itemnr, 13, CastItoIShort(server->GetLowIDUsers()));
	else
		SetItemText(itemnr, 13, _T(""));

	// Obfuscation
	if (server->SupportsObfuscationTCP() && server->GetObfuscationPortTCP() != 0)
		SetItemText(itemnr, 14, GetResString(IDS_YES));
	else
		SetItemText(itemnr, 14, GetResString(IDS_NO));

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	// NatTraversal
	if (server->SupportsNatTraversal() && server->SupportsNatTraversal() != 0)
		SetItemText(itemnr, 15, GetResString(IDS_YES));
	else
		SetItemText(itemnr, 15, GetResString(IDS_NO));
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

#endif // IP2COUNTRY // NEO: IP2C <-- Xanatos --
}

void CServerListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{ 
	// get merged settings
	bool bFirstItem = true;
	int iSelectedItems = GetSelectedCount();
	int iStaticServers = 0;
	UINT uPrioMenuItem = 0;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CServer* pServer = (CServer*)GetItemData(GetNextSelectedItem(pos));

		iStaticServers += pServer->IsStaticMember() ? 1 : 0;

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

	//CTitleMenu ServerMenu;
	CMenuXP ServerMenu; // NEO: NMX - [NeoMenuXP] <-- Xanatos --
	ServerMenu.CreatePopupMenu();
	ServerMenu.AddMenuTitle(GetResString(IDS_EM_SERVER), true);

	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_CONNECTTO, GetResString(IDS_CONNECTTHIS), _T("CONNECT"));
	ServerMenu.SetDefaultItem(iSelectedItems > 0 ? MP_CONNECTTO : -1);

	CMenu ServerPrioMenu;
	ServerPrioMenu.CreateMenu();
	if (iSelectedItems > 0){
		ServerPrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
		ServerPrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
		ServerPrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
		ServerPrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOHIGH, uPrioMenuItem, 0);
	}
	ServerMenu.AppendMenu(MF_POPUP  | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)ServerPrioMenu.m_hMenu, GetResString(IDS_PRIORITY), _T("PRIORITY"));

	// enable add/remove from static server list, if there is at least one selected server which can be used for the action
	ServerMenu.AppendMenu(MF_STRING | (iStaticServers < iSelectedItems ? MF_ENABLED : MF_GRAYED), MP_ADDTOSTATIC, GetResString(IDS_ADDTOSTATIC), _T("ListAdd"));
	ServerMenu.AppendMenu(MF_STRING | (iStaticServers > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVEFROMSTATIC, GetResString(IDS_REMOVEFROMSTATIC), _T("ListRemove"));
	ServerMenu.AppendMenu(MF_STRING | (GetSelectedCount() == 1 ? MF_ENABLED : MF_GRAYED),MP_EDITSERVER, GetResString(IDS_X_EDIT_SVR), _T("FileComments")); // NEO: MOD - [EditServer] <-- Xanatos --
	ServerMenu.AppendMenu(MF_SEPARATOR);

	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
	ServerMenu.AppendMenu(MF_STRING | (theApp.IsEd2kServerLinkInClipboard() ? MF_ENABLED : MF_GRAYED), MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD), _T("PASTELINK"));
	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVE, GetResString(IDS_REMOVETHIS), _T("DELETESELECTED"));
	ServerMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVEALL, GetResString(IDS_REMOVEALL), _T("DELETE"));

	ServerMenu.AppendMenu(MF_SEPARATOR);
	ServerMenu.AppendMenu(MF_ENABLED | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

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
					theApp.serverlist->MoveServerDown(pServer);
				}
			}
			theApp.serverconnect->ConnectToAnyServer(theApp.serverlist->GetServerCount() - GetSelectedCount(), false, false);
		}
		else
		{
			int iItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
			if (iItem > -1)
				theApp.serverconnect->ConnectToServer((CServer*)GetItemData(iItem));
		}
		theApp.emuledlg->ShowConnectionState();
		return TRUE;

	case MP_CUT: {
		CString strURLs = CreateSelectedServersURLs();
		if (!strURLs.IsEmpty())
			theApp.CopyTextToClipboard(strURLs);
		DeleteSelectedServers();
		return TRUE;
	}
	
	case MP_COPYSELECTED:
	case MP_GETED2KLINK:
	case Irc_SetSendLink: {
		CString strURLs = CreateSelectedServersURLs();
		if (!strURLs.IsEmpty()) {
			if (wParam == Irc_SetSendLink)
				theApp.emuledlg->ircwnd->SetSendFileString(strURLs);
			else
				theApp.CopyTextToClipboard(strURLs);
		}
		return TRUE;
	}

	case MP_PASTE:
		if (theApp.IsEd2kServerLinkInClipboard())
			theApp.emuledlg->serverwnd->PasteServerFromClipboard();
		return TRUE;
	// NEO: MOD - [EditServer] -- Xanatos -->
	case MP_EDITSERVER: { // [ionix] Avi3k: EditServer
		int iItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
		if (iItem > -1)
		{
			CServer* cur_srv = (CServer*)GetItemData(iItem);
			CString buffer;
			buffer.Format(_T("%i"),cur_srv->GetPort());
			theApp.emuledlg->serverwnd->GetDlgItem(IDC_SPORT)->SetWindowText(buffer);
			theApp.emuledlg->serverwnd->GetDlgItem(IDC_IPADDRESS)->SetWindowText(cur_srv->GetFullIP());
			theApp.emuledlg->serverwnd->GetDlgItem(IDC_SNAME)->SetWindowText(cur_srv->GetListName());
		}
		return TRUE;
	}
	// NEO: MOD END <-- Xanatos --

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

	case MP_FIND:
		OnFindStart();
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

void CServerListCtrl::OnNMDblClk(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
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

void CServerListCtrl::OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult) 
{ 
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR; 

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool bSortAscending = (GetSortItem()!= pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	UpdateSortHistory(MAKELONG(pNMListView->iSubItem, (bSortAscending ? 0 : 0x0001)));
	SetSortArrow(pNMListView->iSubItem, bSortAscending);
	SortItems(SortProc, MAKELONG(pNMListView->iSubItem, (bSortAscending ? 0 : 0x0001)));

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
	switch (LOWORD(lParamSort)){
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
			  iResult = 0;
		  else if (item2->GetPreference() == SRV_PR_LOW)
			  iResult = 1;
		  else if (item1->GetPreference() == SRV_PR_LOW)
			  iResult = -1;
		  else if (item2->GetPreference() == SRV_PR_HIGH)
			  iResult = -1;
		  else if (item1->GetPreference() == SRV_PR_HIGH)
			  iResult = 1;
		  else
			  iResult = 0;
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

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	  case 15: 
		 iResult = (int)(item1->SupportsNatTraversal() && item1->SupportsNatTraversal() != 0) - (int)(item2->SupportsNatTraversal() && item2->SupportsNatTraversal() != 0);
		 break;
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
      case 16:
          UNDEFINED_STR_AT_BOTTOM(item1->GetCountryName(), item2->GetCountryName());
          iResult = item1->GetCountryName().CompareNoCase(item2->GetCountryName());
		  break;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	  default: 
		  iResult = 0;
	} 

	// NEO: SE - [SortExtension] -- Xanatos --
	/*int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->serverwnd->serverlistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}*/

	if (HIWORD(lParamSort))
		iResult = -iResult;
	return iResult;
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
	CString counter;

	counter.Format(_T(" (%i)"), GetItemCount());
	theApp.emuledlg->serverwnd->GetDlgItem(IDC_SERVLIST_TEXT)->SetWindowText(GetResString(IDS_SV_SERVERLIST)+counter  );
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
		// enable tooltips only if Shift+Ctrl is currently pressed
		bool bShowInfoTip = GetSelectedCount() > 1 || ((GetKeyState(VK_SHIFT) & 0x8000) && (GetKeyState(VK_CONTROL) & 0x8000));

		if (!bShowInfoTip) {
			if (!bOverMainItem) {
				// don't show the default label tip for the main item, if the mouse is not over the main item
				if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
					pGetInfoTip->pszText[0] = _T('\0');
			}
			return;
		}

		if (GetSelectedCount() == 1)
		{
			;
		}
		else
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

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
void CServerListCtrl::RefreshAllServer(){

	for(POSITION pos = theApp.serverlist->list.GetHeadPosition(); pos != NULL;){
		RefreshServer(theApp.serverlist->list.GetAt(pos));
		theApp.serverlist->list.GetNext(pos);
	}

}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CServerListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));
	const CServer* server = (CServer*)lpDrawItemStruct->itemData;
	const CServer* pConnectedServer = theApp.serverconnect->GetCurrentServer();
	if( (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED )
	 || (pConnectedServer && pConnectedServer->GetIP() == server->GetIP() && pConnectedServer->GetPort() == server->GetPort())){
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	RECT cur_rec = lpDrawItemStruct->rcItem;
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;

	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);

		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			DrawServerItem(&dc,iColumn,&cur_rec,server);
			cur_rec.left += GetColumnWidth(iColumn);	
		}
	}
	//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;
		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;
		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}


void CServerListCtrl::DrawServerItem(CDC *dc, int iColumn, LPRECT cur_rec, const CServer* server){

	CString Sbuffer;

	switch(iColumn){
		case 0:{
			uint8 image;
			image = 0;
			
			Sbuffer = server->GetListName();

			// NEO: IP2C - [IPtoCountry]
			POINT point2= {cur_rec->left,cur_rec->top+1};
			const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() && theApp.ip2country->ShowCountryFlag()
								&& (NeoPrefs.IsIP2CountryShowFlag() == 2 || IsColumnHidden(15));
			if(bShowFlags){
				theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, server->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
			}else
				imagelist.DrawIndirect(dc, 0, point2, CSize(16,16), CPoint(0,0), ILD_NORMAL);
			// IP2COUNTRY // NEO: IP2C END 

			cur_rec->left +=20;
			dc->DrawText(Sbuffer,Sbuffer.GetLength(),cur_rec,DLC_DT_TEXT);
			cur_rec->left -=20;

			break;
		}
		case 1:{
			Sbuffer.Format(_T("%s : %i"), server->GetAddress(), server->GetPort());
			break;
		}
		case 2:{
			Sbuffer = server->GetDescription();
			break;
		}
		case 3:{
			if(server->GetPing())
				Sbuffer.Format(_T("%i"), server->GetPing());
			else
				Sbuffer = "";
			break;
		}
		case 4:{
			if(server->GetUsers())
				Sbuffer.Format(_T("%s"), CastItoIShort(server->GetUsers()));
			else
				Sbuffer = "";
			break;
		}
		case 5:{
			if(server->GetMaxUsers())
				Sbuffer.Format(_T("%s"), CastItoIShort(server->GetMaxUsers()));
			else
				Sbuffer = "";
			break;
		}
		case 6:{
			if(server->GetFiles())
				Sbuffer.Format(_T("%s"), CastItoIShort(server->GetFiles()));
			else
				Sbuffer = "";
			break;
		}
		case 7:{
			switch(server->GetPreference()){
				case SRV_PR_LOW:
					Sbuffer = GetResString(IDS_PRIOLOW);
					break;
				case SRV_PR_NORMAL:
					Sbuffer = GetResString(IDS_PRIONORMAL);
					break;
				case SRV_PR_HIGH:
					Sbuffer = GetResString(IDS_PRIOHIGH);
					break;
				default:
					Sbuffer = GetResString(IDS_PRIONOPREF);
				}
			break;
			}
		case 8:{
			Sbuffer.Format(_T("%i"), server->GetFailedCount());
			break;
		}
		case 9:{
			if (server->IsStaticMember())
				Sbuffer = GetResString(IDS_YES); 
			else
				Sbuffer = GetResString(IDS_NO);
			break;
		}
		case 10:{
			if(server->GetSoftFiles())
				Sbuffer.Format(_T("%s"), CastItoIShort(server->GetSoftFiles()));
			else
				Sbuffer = "";
			break;
		}
		case 11:{
			if(server->GetHardFiles())
				Sbuffer.Format(_T("%s"), CastItoIShort(server->GetHardFiles()));
			else
				Sbuffer = "";
			break;
		}
		case 12:{
			Sbuffer = server->GetVersion();
			if (thePrefs.GetDebugServerUDPLevel() > 0){
				if (server->GetUDPFlags() != 0){
					if (!Sbuffer.IsEmpty())
						Sbuffer += _T("; ");
					Sbuffer.AppendFormat(_T("ExtUDP=%x"), server->GetUDPFlags());
				}
			}
			if (thePrefs.GetDebugServerTCPLevel() > 0){
				if (server->GetTCPFlags() != 0){
					if (!Sbuffer.IsEmpty())
						Sbuffer += _T("; ");
					Sbuffer.AppendFormat(_T("ExtTCP=%x"), server->GetTCPFlags());
					}
				}
			break;
		}

		case 13:{
			if (server->GetLowIDUsers())
				Sbuffer.Format(_T("%s"), CastItoIShort(server->GetLowIDUsers()));
			else
				Sbuffer = _T("");
			break;
		}

		case 14:{
			if (server->SupportsObfuscationTCP() && server->GetObfuscationPortTCP() != 0)
				Sbuffer = GetResString(IDS_YES);
			else
				Sbuffer = GetResString(IDS_NO);
			break;
		}

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		case 15:{
			if (server->SupportsNatTraversal() && server->SupportsNatTraversal() != 0)
				Sbuffer = GetResString(IDS_YES);
			else
				Sbuffer = GetResString(IDS_NO);
			break;
		}
#endif //NATTUNNELING // NEO: NATT END

		// NEO: IP2C - [IPtoCountry]
		case 16:{
			Sbuffer.Format(_T("%s"), server->GetCountryName());
			const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() == 1 && theApp.ip2country->ShowCountryFlag();
			if(bShowFlags){
				POINT point2= {cur_rec->left,cur_rec->top+1};
				theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, server->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
				cur_rec->left+=20;
			}
			dc->DrawText(Sbuffer,Sbuffer.GetLength(),cur_rec,DLC_DT_TEXT);
			if(bShowFlags)
				cur_rec->left-=20;
			break;
		}
		// IP2COUNTRY // NEO: IP2C END
	}

	if(iColumn != 0 && iColumn != 16) // NEO: IP2C - [IPtoCountry]
	//if(iColumn != 0)
		dc->DrawText(Sbuffer,Sbuffer.GetLength(),cur_rec, DT_LEFT);

}


#else

void CServerListCtrl::OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *plResult)
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
		// the server which we are connected to always has a valid numerical IP member assigned,
		// therefor we do not need to call CServer::IsEqual which would be expensive
		//if (pConnectedServer && pConnectedServer->IsEqual(pServer))
		if (pConnectedServer && pConnectedServer->GetIP() == pServer->GetIP() && pConnectedServer->GetPort() == pServer->GetPort())
			pnmlvcd->clrText = RGB(32,32,255);
		//else if (pServer->GetFailedCount() >= thePrefs.GetDeadServerRetries())
		else if (pServer->GetFailedCount() >= thePrefs.GetDeadServerRetries() || theApp.ipfilter->IsFiltered(pServer->GetIP())) // NEO: MOD - [ServerIPFilter] <-- Xanatos --
			pnmlvcd->clrText = RGB(192,192,192);
		else if (pServer->GetFailedCount() >= 2)
			pnmlvcd->clrText = RGB(128,128,128);
	}

	*plResult = CDRF_DODEFAULT;
}

#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --