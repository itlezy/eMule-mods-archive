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
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "MemDC.h" // Xman

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CServerListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CServerListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	//Xman no need for this because //EastShare - added by AndCycle, IP to Country
	/*
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNmCustomDraw)
	*/
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	//Xman end
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CServerListCtrl::CServerListCtrl()
{
	//Xman IP to country //remark: because of new drawing methodes we can only search first column
	/*
	SetGeneralPurposeFind(true);
	*/
	SetGeneralPurposeFind(true, false);
	//Xman End
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	/*
	m_tooltip = new CToolTipCtrlX;
	*/
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
		m_tooltip = new CToolTipCtrlX;
	else
		m_tooltip = NULL;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
	SetSkinKey(L"ServersLv");
}

bool CServerListCtrl::Init()
{
	SetPrefsKey(_T("ServerListCtrl"));
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	{
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		CToolTipCtrl* tooltip = GetToolTips();
		if (tooltip) {
			m_tooltip->SubclassWindow(*tooltip);
			tooltip->ModifyStyle(0, TTS_NOPREFIX);
			tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
			//tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
		}
	} // Run eMule as NT Service [leuk_he/Stulle] - Stulle

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

	SetAllIcons();
	Localize();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, MAKELONG(GetSortItem(), (GetSortAscending() ? 0 : 0x0001)));

	ShowServerCount();

	return true;
} 

CServerListCtrl::~CServerListCtrl()
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		delete m_tooltip;
}

void CServerListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CServerListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("Server")));
	HIMAGELIST himl = ApplyImageList(iml.Detach());
	if (himl)
		ImageList_Destroy(himl);
	//MORPH START - Changed b SiRoB, CountryFlag Addon , IP to Country
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	imagelist.Add(CTempIconLoader(_T("Server")));
	//MORPH END  - Changed b SiRoB, CountryFlag Addon
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

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		RefreshServer((CServer*)GetItemData(i));

	// ==> Design Settings [eWombat/Stulle] - Stulle
	COLORREF crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_serverwnd);

	if(crTempColor == CLR_DEFAULT)
		crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_default);

	if(crTempColor != CLR_DEFAULT)
		SetBkColor(crTempColor);
	else
		SetBkColor(COLORREF(RGB(255,255,255)));
	// <== Design Settings [eWombat/Stulle] - Stulle
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
			}
			// [end] Mighty Knife
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
				AddDebugLogLine(false, _T("IPFilter(Updated): Filtered server \"%s\" (IP=%s) - IP filter (%s)"), cur_server->GetListName(), ipstr(cur_server->GetIP()), theApp.ipfilter->GetLastHit());
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

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_SVR_OPT))
		return true;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (bAddToList)
	{
		InsertItem(LVIF_TEXT | LVIF_PARAM, bAddTail ? GetItemCount() : 0, pServer->GetListName(), 0, 0, 0, (LPARAM)pServer);
		RefreshServer(pServer);
	}
	ShowServerCount();
	return true;
}

void CServerListCtrl::RefreshServer(const CServer* server)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_SVR_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!server || !theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->serverwnd || IsWindowVisible() == FALSE )
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)server;
	int itemnr = FindItem(&find);
	if (itemnr == -1)
		return;

	//MORPH START - Added by SiRoB,  CountryFlag Addon, IP to Country
	Update(itemnr);
	//MORPH START - Added by SiRoB,  CountryFlag Addon

	//Xman not used because of morph code
	/*
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
	*/
	//Xman end
}

//EastShare Start - added by AndCycle, IP to Country
/* Xman not needed
void CServerListCtrl::RefreshAllServer(){

	for (POSITION pos = theApp.serverlist->list.GetHeadPosition(); pos != NULL; theApp.serverlist->list.GetNext(pos))
	{
		RefreshServer(theApp.serverlist->list.GetAt(pos));
	}
}
*/
//EastShare End - added by AndCycle, IP to Country

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

	CTitleMenu ServerMenu;
	ServerMenu.CreatePopupMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	ServerMenu.AddMenuTitle(GetResString(IDS_EM_SERVER), true);
	*/
	ServerMenu.AddMenuTitle(GetResString(IDS_SV_SERVERLIST), true);
	// <== XP Style Menu [Xanatos] - Stulle

	ServerMenu.AppendMenu(MF_STRING | (iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED), MP_CONNECTTO, GetResString(IDS_CONNECTTHIS), _T("CONNECT"));
	ServerMenu.SetDefaultItem(iSelectedItems > 0 ? MP_CONNECTTO : -1);

	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	CMenu ServerPrioMenu;
	ServerPrioMenu.CreateMenu();
	*/
	CTitleMenu ServerPrioMenu;
	ServerPrioMenu.CreateMenu();
	ServerPrioMenu.AddMenuTitle(GetResString(IDS_PRIORITY), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
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
			case 4: // Users
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

	  default: 
		  iResult = 0;
	} 

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->serverwnd->serverlistctrl.GetNextSortOrder(lParamSort)) != -1)
		iResult = SortProc(lParam1, lParam2, dwNextSort);
	*/
	//Xman end

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
	theApp.emuledlg->serverwnd->GetDlgItem(IDC_SERVLIST_TEXT)->SetWindowText(GetResString(IDS_SV_SERVERLIST) + counter);
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

//Xman no need for this because //EastShare - added by AndCycle, IP to Country
/*
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
		// the server which we are connected to always has a valid numerical IP member assigned,
		// therefor we do not need to call CServer::IsEqual which would be expensive
		//if (pConnectedServer && pConnectedServer->IsEqual(pServer))
		if (pServer && pConnectedServer && pConnectedServer->GetIP() == pServer->GetIP() && pConnectedServer->GetPort() == pServer->GetPort())
			pnmlvcd->clrText = RGB(32,32,255);
		else if (pServer && pServer->GetFailedCount() >= thePrefs.GetDeadServerRetries())
			pnmlvcd->clrText = RGB(192,192,192);
		else if (pServer && pServer->GetFailedCount() >= 2)
			pnmlvcd->clrText = RGB(128,128,128);
	}

	*plResult = CDRF_DODEFAULT;
	
}
*/
//Xman end

//Commander - Added: CountryFlag - Start IP to Country
void CServerListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	// ==> Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	/*
	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	*/
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	// <== Visual Studio 2010 Compatibility [Stulle/Avi-3k/ied] - Stulle
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);

	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);

	const CServer* server = (CServer*)lpDrawItemStruct->itemData;
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	const CServer* cur_srv;

	//Xman our server in blubold
	//+
	//grey out dead servers (BlueSonic/TK4)
	LOGFONT lfFont = {0};
	CFont fontCustom;
	if(theApp.serverconnect->IsConnected()
		&& (cur_srv = theApp.serverconnect->GetCurrentServer()) != NULL
		&& server != NULL
		&& cur_srv->GetPort() == server->GetPort()
		//&& cur_srv->GetConnPort() == server->GetConnPort()//Morph - added by AndCycle, aux Ports, by lugdunummaster
		&& _tcsicmp(cur_srv->GetAddress(), server->GetAddress()) == 0)
	{
		//it's our server
		GetFont()->GetLogFont(&lfFont);
		lfFont.lfWeight = FW_BOLD; 
		fontCustom.CreateFontIndirect(&lfFont);

		dc.SelectObject(&fontCustom);
		dc.SetTextColor(RGB(0,0,192));
	}
	else //TK4 Mod grey out Filtered servers or Dead servers
		if(server && server->GetFailedCount() >= thePrefs.GetDeadServerRetries() || theApp.ipfilter->IsFiltered(server->GetIP()))
		{
			//GetFont()->GetLogFont(&lfFont);
			//fontCustom.CreateFontIndirect(&lfFont);
			//dc.SelectObject(&fontCustom);
			dc.SetTextColor(RGB(192,192,192));
		} else if(server && server->GetFailedCount() >= 2)
		{ //unreliable servers
			//GetFont()->GetLogFont(&lfFont);
			//fontCustom.CreateFontIndirect(&lfFont);
			//dc.SelectObject(&fontCustom);
			dc.SetTextColor(RGB(128,128,128));
		}
	//Xman end
	*/
	// moved InitItemMemDC code here to be able to color connected server
	const CServer* cur_srv = theApp.serverconnect->IsConnected() ? theApp.serverconnect->GetCurrentServer() : NULL;
	bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	int iClientStyle = style_se_default;
	if(thePrefs.GetStyleOnOff(server_styles, style_se_connected)!=0
		&& cur_srv != NULL
		&& cur_srv->GetPort() == server->GetPort()
		&& _tcsicmp(cur_srv->GetAddress(), server->GetAddress()) == 0)
		iClientStyle = style_se_connected;
	else if(thePrefs.GetStyleOnOff(server_styles, style_se_static)!=0
		&& server->IsStaticMember())
		iClientStyle = style_se_static;
	else if(thePrefs.GetStyleOnOff(server_styles, style_se_filtered)!=0
		&& theApp.ipfilter->IsFiltered(server->GetIP()))
		iClientStyle = style_se_filtered;
	else if(thePrefs.GetStyleOnOff(server_styles, style_se_dead)!=0
		&& server->GetFailedCount() >= thePrefs.GetDeadServerRetries())
		iClientStyle = style_se_dead;
	else if(thePrefs.GetStyleOnOff(server_styles, style_se_unreliable)!=0
		&& server->GetFailedCount() >= 2)
		iClientStyle = style_se_unreliable;

	StylesStruct style;
	thePrefs.GetStyle(server_styles, iClientStyle, &style);
	COLORREF crTempColor = GetBkColor();
	if (style.nBackColor != CLR_DEFAULT)
		crTempColor = style.nBackColor;

	if( lpDrawItemStruct->itemState & ODS_SELECTED )
	{
		if(bCtrlFocused)
			dc->FillBackground(__super::m_crHighlight);
		else
			dc->FillBackground(__super::m_crNoHighlight);
	}
	else
	{
		if (crTempColor == CLR_NONE)
		{
			__super::DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
			dc->SetBkMode(TRANSPARENT);
		}
		else
		{
			dc->FillBackground(crTempColor);
		}
	}
	crTempColor = m_crWindowText;
	if(style.nFontColor != CLR_DEFAULT)
		crTempColor = style.nFontColor;

	CFont* pOldFont = dc->SelectObject(theApp.GetFontByStyle(style.nFlags,false));
	COLORREF crOldTextColor = dc->SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : crTempColor);
	// <== Design Settings [eWombat/Stulle] - Stulle

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iLabelOffset;
	cur_rec.left += sm_iIconOffset;

	for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if (!IsColumnHidden(iColumn))
		{
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
			{
				TCHAR szItem[1024];
				GetItemDisplayText(server, iColumn, szItem, _countof(szItem));
				switch (iColumn)
				{
					case 0:
					{
						int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
						POINT point = { cur_rec.left, cur_rec.top + iIconPosY };
						int iIconSize = 16;
						//Draw Country Flag
						if(theApp.ip2country->ShowCountryFlag() ){
							//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, server->GetCountryFlagIndex(), point, CSize(18,16), CPoint(0,0), ILD_NORMAL);
							theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,server->GetCountryFlagIndex(),point));
							iIconSize = 18;
						}
						else
							imagelist.Draw(dc, 0, point, ILD_NORMAL);

						cur_rec.left +=iIconSize + sm_iLabelOffset;
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						cur_rec.left -=iIconSize;
						cur_rec.right -= sm_iSubItemInset;
						break;
					}
					default:
						dc.DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
						break;
				}
			}
			cur_rec.left += iColumnWidth;
		}
	}
	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	fontCustom.DeleteObject(); //Xman
	*/
	dc->SetTextColor(crOldTextColor);
	dc->SelectObject(pOldFont);
	// <== Design Settings [eWombat/Stulle] - Stulle
}

void CServerListCtrl::GetItemDisplayText(const CServer* server, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	switch(iSubItem){
		case 0:
			_tcsncpy(pszText, server->GetListName(), cchTextMax);
			break;
		case 1:
			//DolphinX :: support dynamic ip servers :: Start
			if(server->HasDynIP()){
				if(server->GetIP())
					_sntprintf(pszText, cchTextMax, _T("%s(%s) : %i"), server->GetFullIP(), server->GetDynIP(), server->GetPort());
				else
					_sntprintf(pszText, cchTextMax, _T("%s : %i"),  server->GetDynIP(), server->GetPort());
			}
			else
				_sntprintf(pszText, cchTextMax, _T("%s : %i"),  server->GetAddress(), server->GetPort());
			//DolphinX :: support dynamic ip servers :: End
			break;
		case 2:
			_tcsncpy(pszText, server->GetDescription(), cchTextMax);
			break;
		case 3:
			if(server->GetPing())
				_sntprintf(pszText, cchTextMax, _T("%i"), server->GetPing());
			break;
		case 4:
			if(server->GetUsers())
				_sntprintf(pszText, cchTextMax, _T("%i"), server->GetUsers());
			break;
		case 5:
			if(server->GetMaxUsers())
				_sntprintf(pszText, cchTextMax, _T("%i"), server->GetMaxUsers());
			break;
		case 6:
			if(server->GetFiles())
				_sntprintf(pszText, cchTextMax, _T("%i"), server->GetFiles());
			break;
		case 7:
		{
			CString Sbuffer;
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
			_tcsncpy(pszText, Sbuffer, cchTextMax);
			break;
		}
		case 8:
			_sntprintf(pszText, cchTextMax, _T("%i"), server->GetFailedCount());
			break;
		case 9:
			_tcsncpy(pszText, GetResString( (server->IsStaticMember()) ? IDS_YES : IDS_NO ), cchTextMax);
			break;
		case 10:
			if(server->GetSoftFiles())
				_sntprintf(pszText, cchTextMax, _T("%i"), server->GetSoftFiles());
			break;
		case 11:
			if(server->GetHardFiles())
				_sntprintf(pszText, cchTextMax, _T("%i"), server->GetHardFiles());
			break;
		case 12:
		{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
			CString Sbuffer = server->GetVersion();
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
			_tcsncpy(pszText, Sbuffer, cchTextMax);
#else
			_tcsncpy(pszText, server->GetVersion(), cchTextMax);
#endif
			break;
		}
		//Lowid Column
		case 13:
			if (server->GetLowIDUsers())
				_tcsncpy(pszText, CastItoIShort(server->GetLowIDUsers()), cchTextMax);
			break;
		case 14:
			_tcsncpy(pszText, GetResString( (server->SupportsObfuscationTCP() && server->GetObfuscationPortTCP() != 0) ? IDS_YES : IDS_NO ), cchTextMax);
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CServerListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (theApp.emuledlg->IsRunning()) {
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, do *NOT* put any time consuming code in here.
		//
		// Vista: That callback is used to get the strings for the label tips for the sub(!) items.
		//
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
		if (pDispInfo->item.mask & LVIF_TEXT) {
			const CServer* pServer = reinterpret_cast<CServer*>(pDispInfo->item.lParam);
			if (pServer != NULL)
				GetItemDisplayText(pServer, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
		}
	}
	*pResult = 0;
}
//Commander - Added: CountryFlag - End
