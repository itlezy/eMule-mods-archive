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
#include "emule.h"
#include "DownloadListCtrl.h"
#include "otherfunctions.h" 
#include "updownclient.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "FileDetailDialog.h"
#include "commentdialoglst.h"
#include "MetaDataDlg.h"
#include "InputBox.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "MemDC.h"
#include "ChatWnd.h"
#include "TransferWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/net/KademliaUDPListener.h"
#include "WebServices.h"
#include "Preview.h"
#include "StringConversion.h"
#include "AddSourceDlg.h"
#include "ToolTipCtrlX.h"
#include "CollectionViewDialog.h"
#include "SearchDlg.h"
#include "SharedFileList.h"
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/CP/FilePreferencesDialog.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h"// NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/Functions.h"// NEO: MOD <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList]-- Xanatos -->
#include "Neo/Sources/SourceList.h"
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CDownloadListCtrl

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
#define DLC_BARUPDATE 512

#define	FILE_ITEM_MARGIN_X	4
#define RATING_ICON_WIDTH	16


IMPLEMENT_DYNAMIC(CtrlItem_Struct, CObject)

IMPLEMENT_DYNAMIC(CDownloadListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnItemActivate)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnListModified)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClickDownloadlist) // NEO: NTS - [NeoTreeStyle] <-- Xanatos --
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadlist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
END_MESSAGE_MAP()

CDownloadListCtrl::CDownloadListCtrl()
	: CDownloadListListCtrlItemWalk(this)
{
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	m_tooltip = new CToolTipCtrlX;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	SetGeneralPurposeFind(true, false);
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
	// NEO: MCM - [ManualClientManagement] -- Xanatos -->
	if (m_A4AFMenu)
		VERIFY( m_A4AFMenu.DestroyMenu() );
	// NEO: MCM END <-- Xanatos --
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	if (m_PWProtMenu) 
		VERIFY( m_PWProtMenu.DestroyMenu() ); 
	// NEO: PP END <-- Xanatos --
	// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
	if (m_CollectMenu) 
		VERIFY( m_CollectMenu.DestroyMenu() );
	// NEO: MSR END <-- Xanatos --
    if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	// NEO: MSD - [ManualSourcesDrop] -- Xanatos -->
	if (m_DropMenu) 
		VERIFY( m_DropMenu.DestroyMenu() );
	// NEO: MSD END <-- Xanatos --
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	if (m_TempDirMenu) 
		VERIFY( m_TempDirMenu.DestroyMenu() ); 
	// NEO: MTD END <-- Xanatos --
		
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );
	
	while (m_ListItems.empty() == false) {
		delete m_ListItems.begin()->second; // second = CtrlItem_Struct*
		m_ListItems.erase(m_ListItems.begin());
	}
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos 
	delete m_tooltip;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
}

void CDownloadListCtrl::Init()
{
	SetName(_T("DownloadListCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy, theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetStyle();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	EnableDual(); // NEO: SEa - [SortAltExtension] <-- Xanatos --
	
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SetFileIconToolTip(true);
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	InsertColumn(0,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 260);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT, 60);
	InsertColumn(2,GetResString(IDS_DL_TRANSF),LVCFMT_LEFT, 65);
	InsertColumn(3,GetResString(IDS_DL_TRANSFCOMPL),LVCFMT_LEFT, 65);
	InsertColumn(4,GetResString(IDS_DL_SPEED),LVCFMT_LEFT, 65);
	InsertColumn(5,GetResString(IDS_DL_PROGRESS),LVCFMT_LEFT, 170);
	InsertColumn(6,GetResString(IDS_DL_SOURCES),LVCFMT_LEFT, 50);
	InsertColumn(7,GetResString(IDS_PRIORITY),LVCFMT_LEFT, 55);
	InsertColumn(8,GetResString(IDS_STATUS),LVCFMT_LEFT, 70);
	InsertColumn(9,GetResString(IDS_DL_REMAINS),LVCFMT_LEFT, 110);
	CString lsctitle=GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(_T(':'));
	InsertColumn(10, lsctitle,LVCFMT_LEFT, 220);
	lsctitle=GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(_T(':'));
	InsertColumn(11, lsctitle,LVCFMT_LEFT, 220);
	InsertColumn(12, GetResString(IDS_CAT) ,LVCFMT_LEFT, 100);
	InsertColumn(13, GetResString(IDS_X_SOURCE_LIMITS) ,LVCFMT_LEFT, 100); // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	InsertColumn(14, GetResString(IDS_X_CAT_COLORDER),LVCFMT_LEFT,60); // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
	InsertColumn(15,GetResString(IDS_X_DL_FOLDER),LVCFMT_LEFT, 260); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --

	SetAllIcons();
	Localize();
	LoadSettings();
	curTab=0;

	//if (thePrefs.GetShowActiveDownloadsBold()) // NEO: FIX - [ShowActiveDownloadsBold] <-- Xanatos --
	//{
	CFont* pFont = GetFont();
	LOGFONT lfFont = {0};
	pFont->GetLogFont(&lfFont);
	lfFont.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect(&lfFont);
	//}

	// NEO: MOD - [Percentage] -- Xanatos -->
	lfFont.lfHeight = 11;
	lfFont.lfWeight = FW_NORMAL;
	m_fontSmall.CreateFontIndirect(&lfFont);
	// NEO: MOD END <-- Xanatos --

	// Barry - Use preferred sort order from preferences
	m_bRemainSort=thePrefs.TransferlistRemainSortStyle();

	uint8 adder=0;
	//if (GetSortItem()!=9 || !m_bRemainSort)
	if ((GetSortItem()!=9 && GetSortItem()!=2 && (GetSortItem()!=3 || !IsAlternate())) || !m_bRemainSort ) // MOD - [SessionDL] <-- Xanatos --
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending()?arrowDoubleUp : arrowDoubleDown);
		// MOD - [SessionDL] -- Xanatos -->
		if(GetSortItem()==2)
			adder=18;
		else if(GetSortItem()==3 && IsAlternate())
			adder=27;
		else
		// MOD END <-- Xanatos --
		adder=81;
	}
	
	SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100) + adder);

}

void CDownloadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

// NEO: NCI - [NewClientIcons] -- Xanatos -->
enum EnumDownloadListIcons
{
	DL_BLANK = 7,

	DL_SRC_DOWNLOADING,
	DL_SRC_ON_QUEUE,
	DL_SRC_CONNECTING,
	DL_SRC_NNPFQ,
	DL_SRC_UNKNOWN,

	DL_CLIENT_CDONKEY,
	DL_CLIENT_EDONKEY,
	DL_CLIENT_EMULE,
	DL_CLIENT_HYBRID,
	DL_CLIENT_MLDONKEY,
	DL_CLIENT_OLDEMULE,
	DL_CLIENT_SHAREAZA,
	DL_CLIENT_UNKNOWN,
	DL_CLIENT_XMULE,
	DL_CLIENT_AMULE,
	DL_CLIENT_LPHANT,
	DL_CLIENT_EMULEPLUS,
	DL_CLIENT_TRUSTYFILES,
	DL_CLIENT_HYDRANODE,
	DL_SERVER,

	DL_CLIENT_MOD,
	DL_CLIENT_MOD_NEO,
	DL_CLIENT_MOD_MORPH,
	DL_CLIENT_MOD_SCARANGEL,
	DL_CLIENT_MOD_STULLE,
	DL_CLIENT_MOD_MAXMOD,
	DL_CLIENT_MOD_XTREME,
	DL_CLIENT_MOD_EASTSHARE,
	DL_CLIENT_MOD_IONIX,
	DL_CLIENT_MOD_CYREX,
	DL_CLIENT_MOD_NEXTEMF,
	
	DL_CREDIT_UP,
	DL_CREDIT_DOWN,
	DL_CREDIT_UP_DOWN,

	DL_FRIEND_CLIENT,
	DL_FRIEND_SLOT_CLIENT,

	DL_ARGOS_BANNED,

	DL_SHOW_SHARED,
	DL_LOW_ID_CLIENT,
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	DL_LANCAST_CLIENT,
#endif //LANCAST // NEO: NLC END

	DL_SECUREHASH_BLUE,
	DL_SECUREHASH_GREEN,
	DL_SECUREHASH_YELLOW,
	DL_SECUREHASH_RED,

	DL_ICON_OBFU,
};
// NEO: NI END <-- Xanatos --

void CDownloadListCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	//m_ImageList.Add(CTempIconLoader(_T("SrcDownloading")));
	//m_ImageList.Add(CTempIconLoader(_T("SrcOnQueue")));
	//m_ImageList.Add(CTempIconLoader(_T("SrcConnecting")));
	//m_ImageList.Add(CTempIconLoader(_T("SrcNNPQF")));
	//m_ImageList.Add(CTempIconLoader(_T("SrcUnknown")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	//m_ImageList.Add(CTempIconLoader(_T("Friend")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	//m_ImageList.Add(CTempIconLoader(_T("Server")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	//m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	//m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	//m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	//m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);

	// NEO: NCI - [NewClientIcons] - [NewIcons] -- Xanatos -->
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("SrcDownloading")));
	m_ImageList.Add(CTempIconLoader(_T("SrcOnQueue")));
	m_ImageList.Add(CTempIconLoader(_T("SrcConnecting")));
	m_ImageList.Add(CTempIconLoader(_T("SrcNNPQF")));
	m_ImageList.Add(CTempIconLoader(_T("SrcUnknown")));

	m_ImageList.Add(CTempIconLoader(_T("CLIENT_CDONKEY")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EDONKEY")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_HYBRID")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MLDONKEY")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_OLDEMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_SHAREAZA")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_UNKNOWN")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_XMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_AMULE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_LPHANT")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EMULEPLUS")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_TRUSTYFILES")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_HYDRANODE")));
	m_ImageList.Add(CTempIconLoader(_T("SERVER")));

	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MOD")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_NEO")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MORPH")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_SCARANGEL")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_STULLE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_MAXMOD")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_XTREME")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_EASTSHARE")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_IONIX")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_CYREX")));
	m_ImageList.Add(CTempIconLoader(_T("CLIENT_NEXTEMF")));
	
	m_ImageList.Add(CTempIconLoader(_T("CREDIT_UP")));
	m_ImageList.Add(CTempIconLoader(_T("CREDIT_DOWN")));
	m_ImageList.Add(CTempIconLoader(_T("CREDIT_UP_DOWN")));

	m_ImageList.Add(CTempIconLoader(_T("FRIEND_CLIENT")));
	m_ImageList.Add(CTempIconLoader(_T("FRIEND_SLOT_CLIENT")));

	m_ImageList.Add(CTempIconLoader(_T("ARGOS_BANNED")));

	m_ImageList.Add(CTempIconLoader(_T("SHOW_SHARED")));
	m_ImageList.Add(CTempIconLoader(_T("LOW_ID_CLIENT")));
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_ImageList.Add(CTempIconLoader(_T("LANCAST_CLIENT")));
#endif //LANCAST // NEO: NLC END

	m_ImageList.Add(CTempIconLoader(_T("SECUREHASH_BLUE")));
	m_ImageList.Add(CTempIconLoader(_T("SECUREHASH_GREEN")));
	m_ImageList.Add(CTempIconLoader(_T("SECUREHASH_YELLOW")));
	m_ImageList.Add(CTempIconLoader(_T("SECUREHASH_RED")));

	m_ImageList.Add(CTempIconLoader(_T("OverlayObfu")));
	// NEO: NI END <-- Xanatos --
}

void CDownloadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_DL_TRANSF);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_DL_TRANSFCOMPL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_DL_PROGRESS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_DL_SOURCES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);

	strRes = GetResString(IDS_DL_REMAINS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);

	strRes = GetResString(IDS_LASTSEENCOMPL);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);

	strRes = GetResString(IDS_FD_LASTCHANGE);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);

	strRes = GetResString(IDS_CAT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);

	// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
	strRes = GetResString(IDS_X_SOURCE_LIMITS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	// NEO: SRT END <-- Xanatos --

	// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
	strRes = GetResString(IDS_X_CAT_COLORDER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
	// NEO: NXC END <-- Xanatos --

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	strRes = GetResString(IDS_X_DL_FOLDER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);
	// NEO: MTD END <-- Xanatos --

	CreateMenues();
	ShowFilesCount();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    int itemnr = GetItemCount();
    newitem->owner = NULL;
    newitem->type = FILE_TYPE;
    newitem->value = toadd;
    newitem->parent = NULL;
	newitem->dwUpdated = 0; 

	// The same file shall be added only once
	ASSERT(m_ListItems.find(toadd) == m_ListItems.end());
	m_ListItems.insert(ListItemsPair(toadd, newitem));

	if (toadd->CheckShowItemInGivenCat(curTab))
		InsertItem(LVIF_PARAM|LVIF_TEXT,itemnr,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);

	ShowFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    newitem->owner = owner;
    newitem->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE;
    newitem->value = source;
	newitem->dwUpdated = 0; 

	// Update cross link to the owner
	ListItems::const_iterator ownerIt = m_ListItems.find(owner);
	ASSERT(ownerIt != m_ListItems.end());
	CtrlItem_Struct* ownerItem = ownerIt->second;
	ASSERT(ownerItem->value == owner);
	newitem->parent = ownerItem;

	// The same source could be added a few time but only one time per file 
	{
		// Update the other instances of this source
		bool bFound = false;
		std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(source);
		for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
			CtrlItem_Struct* cur_item = it->second;

			// Check if this source has been already added to this file => to be sure
			if(cur_item->owner == owner){
				// Update this instance with its new setting
				cur_item->type = newitem->type;
				cur_item->dwUpdated = 0;
				bFound = true;
			}
			else if(notavailable == false){
				// The state 'Available' is exclusive
				cur_item->type = UNAVAILABLE_SOURCE;
				cur_item->dwUpdated = 0;
			}
		}

		if(bFound == true){
			delete newitem; 
			return;
		}
	}
	m_ListItems.insert(ListItemsPair(source, newitem));

	if (owner->srcarevisible) {
		// find parent from the CListCtrl to add source
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)ownerItem;
		int result = FindItem(&find);
		if (result != -1)
			InsertItem(LVIF_PARAM|LVIF_TEXT,result+1,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);
	}
}

void CDownloadListCtrl::RemoveSource(CUpDownClient* source, CPartFile* owner)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::iterator, ListItems::iterator> rangeIt = m_ListItems.equal_range(source);
	for(ListItems::iterator it = rangeIt.first; it != rangeIt.second; ){
		CtrlItem_Struct* delItem  = it->second;
		if(owner == NULL || owner == delItem->owner){
			// Remove it from the m_ListItems			
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
 			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			int result = FindItem(&find);
			if (result != -1)
				DeleteItem(result);

			// finally it could be delete
			delete delItem;
		}
		else{
			it++;
		}
	}
}

bool CDownloadListCtrl::RemoveFile(const CPartFile* toremove)
{
	bool bResult = false;
	if (!theApp.emuledlg->IsRunning())
		return bResult;
	// Retrieve all entries matching the File or linked to the file
	// Remark: The 'asked another files' clients must be removed from here
	ASSERT(toremove != NULL);
	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	if(toremove->IsPartFile() && toremove->srcarevisible==true)
		SetAlternate(FALSE);
	// NEO: SEa END <-- Xanatos --
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* delItem = it->second;
		if(delItem->owner == toremove || delItem->value == (void*)toremove){
			// Remove it from the m_ListItems
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			int result = FindItem(&find);
			if (result != -1)
				DeleteItem(result);

			// finally it could be delete
			delete delItem;
			bResult = true;
		}
		else {
			it++;
		}
	}
	ShowFilesCount();
	return bResult;
}

void CDownloadListCtrl::UpdateItem(void* toupdate)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toupdate);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result != -1){
			updateItem->dwUpdated = 0;
			Update(result);
		}
	}
}

void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem)
{
	if(lpRect->left < lpRect->right)
	{
		CString buffer;
		/*const*/ CPartFile *lpPartFile = (CPartFile*)lpCtrlItem->value;
		switch(nColumn)
		{
		case 0:{	// file name
			// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
			// Show collor only when more categorys are shown together
			//if (!g_bLowColorDesktop && !thePrefs.GetCategory(curTab)->viewfilters.nFromCats) {
			if (!g_bLowColorDesktop && thePrefs.GetCategory(curTab)->viewfilters.nFromCats < 2) { // Lit Cat Filter
				DWORD dwCatColor = thePrefs.GetCatColor(lpPartFile->GetCategory());
				if (dwCatColor > 0)
					dc->SetTextColor(dwCatColor);
			}
			// NEO: NXC END <-- Xanatos --

			CRect rcDraw(lpRect);

			// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
			if(NeoPrefs.UseTreeStyle())
				rcDraw.left += 14;
			// NEO: NTS END <-- Xanatos --

			int iImage = theApp.GetFileTypeSystemImageIdx(lpPartFile->GetFileName());
			if (theApp.GetSystemImageList() != NULL)
				::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), rcDraw.left, rcDraw.top, ILD_NORMAL|ILD_TRANSPARENT);
			rcDraw.left += theApp.GetSmallSytemIconSize().cx;

			if (thePrefs.ShowRatingIndicator() && (lpPartFile->HasComment() || lpPartFile->HasRating() || lpPartFile->IsKadCommentSearchRunning())){
				//m_ImageList.Draw(dc, lpPartFile->UserRating(true)+14, rcDraw.TopLeft(), ILD_NORMAL);
				m_ImageList.Draw(dc, lpPartFile->UserRating(true), rcDraw.TopLeft(), ILD_NORMAL); // NEO: NCI - [NewClientIcons] <-- Xanatos --
				rcDraw.left += RATING_ICON_WIDTH;
			}

			rcDraw.left += 3;
			dc->DrawText(lpPartFile->GetFileName(), lpPartFile->GetFileName().GetLength(), &rcDraw, DLC_DT_TEXT);
			// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
			if(NeoPrefs.UseTreeStyle())
				rcDraw.left -= 14;
			// NEO: NTS END <-- Xanatos --
			break;
		}

		case 1:		// size
			buffer = CastItoXBytes(lpPartFile->GetFileSize(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 2:		// transferred
			//buffer = CastItoXBytes(lpPartFile->GetTransferred(), false, false);
			buffer = StrLine(_T("%s (%s)"), CastItoXBytes(lpPartFile->GetTransferred(), false, false),  CastItoXBytes(lpPartFile->GetTransferredSession(), false, false)); // MOD - [SessionDL] <-- Xanatos --
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 3:		// transferred complete
			buffer = CastItoXBytes(lpPartFile->GetCompletedSize(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 4:		// speed
			if (lpPartFile->GetTransferringSrcCount()){
				buffer.Format(_T("%s"), CastItoXBytes(lpPartFile->GetDatarate(), false, true));
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
				if(lpPartFile->GetLanDatarate() > 0)
					buffer.AppendFormat(_T(" + (%s)"), CastItoXBytes(lpPartFile->GetLanDatarate(), false, true));
#endif //LANCAST // NEO: NLC END <-- Xanatos --
			}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
			if(lpPartFile->GetVoodooDatarate() > 0)
				buffer.AppendFormat(_T(" + [%s]"), CastItoXBytes(lpPartFile->GetVoodooDatarate(), false, true));
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 5:		// progress
			{
				CRect rcDraw(*lpRect);
				rcDraw.bottom--;
				rcDraw.top++;

				// added
				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL));
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx; 
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !lpCtrlItem->dwUpdated) {
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
				//added end

				if (thePrefs.GetUseDwlPercentage()) {
					// NEO: MOD - [Percentage] -- Xanatos -->
					COLORREF oldclr = dc->SetTextColor(RGB(0,0,0));
					int iOMode = dc->SetBkMode(TRANSPARENT);
					if(thePrefs.GetUseDwlPercentage() == TRUE)
						buffer.Format(_T("%.1f%%"), lpPartFile->GetPercentCompleted());
					else
						buffer.Format(_T("%.1f%% / %.1f%%"), lpPartFile->GetPercentCompleted(), lpPartFile->GetPercentCompleted() - lpPartFile->GetPercentCompletedInitial());
					CFont *pOldFont = dc->SelectObject(&m_fontSmall);
					rcDraw.OffsetRect(-1,0);
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					rcDraw.OffsetRect(2,0);
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					rcDraw.OffsetRect(-1,-1);
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					rcDraw.OffsetRect(0,2);
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					rcDraw.OffsetRect(0,-1);
					dc->SetTextColor(RGB(255,255,255));
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);

					dc->SelectObject(pOldFont);
					dc->SetBkMode(iOMode);
					dc->SetTextColor(oldclr);
					// NEO: MOD END <-- Xanatos --

					// HoaX_69: BEGIN Display percent in progress bar
					//COLORREF oldclr = dc->SetTextColor(RGB(255,255,255));
					//int iOMode = dc->SetBkMode(TRANSPARENT);
					//buffer.Format(_T("%.1f%%"), lpPartFile->GetPercentCompleted());
					//dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					//dc->SetBkMode(iOMode);
					//dc->SetTextColor(oldclr);
					// HoaX_69: END
				}
			}
			break;

		case 6:		// sources
			{
				//UINT sc = lpPartFile->GetSourceCount();
				UINT sc = (lpPartFile->GetSourceCount() - lpPartFile->GetSrcStatisticsValue(DS_CACHED)); // NEO: XSC - [ExtremeSourceCache] <-- Xanatos --
				UINT ncsc = lpPartFile->GetNotCurrentSourcesCount();
// ZZ:DownloadManager -->
                if(!(lpPartFile->GetStatus() == PS_PAUSED && sc == 0) && lpPartFile->GetStatus() != PS_COMPLETE) {
                    buffer.Format(_T("%i"), sc-ncsc);
				    if(ncsc>0) buffer.AppendFormat(_T("/%i"), sc);
					if(lpPartFile->GetSrcStatisticsValue(DS_CACHED) > 0)  buffer.AppendFormat(_T("[%i]"), lpPartFile->GetSrcStatisticsValue(DS_CACHED)); // NEO: XSC - [ExtremeSourceCache] <-- Xanatos --
                    if(thePrefs.IsExtControlsEnabled() && lpPartFile->GetSrcA4AFCount() > 0) buffer.AppendFormat(_T("+%i"), lpPartFile->GetSrcA4AFCount());
				    if(lpPartFile->GetTransferringSrcCount() > 0) buffer.AppendFormat(_T(" (%i)"), lpPartFile->GetTransferringSrcCount());
                } else {
                    buffer = _T("");
				}
// <-- ZZ:DownloadManager
				// NEO: SRT - [SourceRequestTweaks] -- Xanatos --
				//if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				//	buffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 7:		// prio
			switch(lpPartFile->GetDownPriority()) {
			case PR_LOW:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTOLOW),GetResString(IDS_PRIOAUTOLOW).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIOLOW),GetResString(IDS_PRIOLOW).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			case PR_NORMAL:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTONORMAL),GetResString(IDS_PRIOAUTONORMAL).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIONORMAL),GetResString(IDS_PRIONORMAL).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			case PR_HIGH:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTOHIGH),GetResString(IDS_PRIOAUTOHIGH).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIOHIGH),GetResString(IDS_PRIOHIGH).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			}
			break;

		case 8:		// <<--9/21/02
			buffer = lpPartFile->getPartfileStatus();
			// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
			if(lpPartFile->GetPartsHashing())
				buffer.AppendFormat(GetResString(IDS_X_ISHASHING),lpPartFile->GetPartsHashing());
			// NEO: SSH END <-- Xanatos --
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;

		case 9:		// remaining time & size
			{
				if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE ){
					// time 
					time_t restTime;
					if (!thePrefs.UseSimpleTimeRemainingComputation())
						restTime = lpPartFile->getTimeRemaining();
					else
						restTime = lpPartFile->getTimeRemainingSimple();

					buffer.Format(_T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
				}
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 10: // last seen complete
			{
				CString tempbuffer;
				if (lpPartFile->m_nCompleteSourcesCountLo == 0)
				{
					tempbuffer.Format(_T("< %u"), lpPartFile->m_nCompleteSourcesCountHi);
				}
				else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
				{
					tempbuffer.Format(_T("%u"), lpPartFile->m_nCompleteSourcesCountLo);
				}
				else
				{
					tempbuffer.Format(_T("%u - %u"), lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);
				}
				if (lpPartFile->lastseencomplete==NULL)
					buffer.Format(_T("%s (%s)"),GetResString(IDS_NEVER),tempbuffer);
				else
					buffer.Format(_T("%s (%s)"),lpPartFile->lastseencomplete.Format( thePrefs.GetDateTimeFormat()),tempbuffer);
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 11: // last receive
			if (!IsColumnHidden(11)) {
				if(lpPartFile->GetFileDate()!=NULL && lpPartFile->GetCompletedSize() > (uint64)0)
					buffer=lpPartFile->GetCFileDate().Format( thePrefs.GetDateTimeFormat());
				else
					buffer.Format(_T("%s"),GetResString(IDS_NEVER));

				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 12: // cat
			if (!IsColumnHidden(12)) {
				//buffer=(lpPartFile->GetCategory()!=0)?
				//	thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle:_T("");
				// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
				if (!NeoPrefs.ShowCatNameInDownList())
					buffer.Format(_T("%u"), lpPartFile->GetCategory());
				else
					buffer.Format(_T("%s"), thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle);
				// NEO: NXC END <-- Xanatos --
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
		case 13: // source limits
			if (!IsColumnHidden(13)) {
				buffer.Empty();
				// NEO: AHL - [AutoHardLimit]
				if(lpPartFile->PartPrefs->UseAutoHardLimit()) 
					buffer.AppendFormat(_T("A"));
				// NEO: AHL END
				// NEO: CSL - [CategorySourceLimit]
				if(lpPartFile->PartPrefs->UseCategorySourceLimit()) 
					buffer.AppendFormat(_T("C"));
				// NEO: CSL END
				// NEO: GSL - [GlobalSourceLimit]
				if(lpPartFile->PartPrefs->UseGlobalSourceLimit()) 
					buffer.AppendFormat(_T("G"));
				// NEO: GSL END
				if(!buffer.IsEmpty())
					buffer.Append(_T(":"));

				buffer.AppendFormat(_T("%i;%i"),lpPartFile->GetMaxSources(),lpPartFile->GetSrcStatisticsValue(DS_ONQUEUE) + lpPartFile->GetSrcStatisticsValue(DS_DOWNLOADING));

				// NEO: ASL - [AutoSoftLock]
				if (UINT toMany = lpPartFile->GetSrcStatisticsValue(DS_TOOMANYCONNS) + lpPartFile->GetSrcStatisticsValue(DS_TOOMANYCONNSKAD))
					buffer.AppendFormat(_T("/%i"),toMany);
				if(lpPartFile->PartPrefs->UseAutoSoftLock() && lpPartFile->CheckSoftLock()) 
					buffer.AppendFormat(_T("L"));
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				// NEO: ASL END
			}
			break;
		// NEO: SRT END <-- Xanatos --
		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		case 14:
			buffer.Format(_T("%u"), lpPartFile->GetCatResumeOrder());
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;
		// NEO: NXC END <-- Xanatos --
		// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
		case 15:
			if (!IsColumnHidden(13)) {
				buffer=lpPartFile->IsPartFile()?
					lpPartFile->GetPath():_T("");
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		// NEO: MTD END <-- Xanatos --
		}
	}
}

void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem) {
	if(lpRect->left < lpRect->right) { 

		CString buffer;
		CUpDownClient *lpUpDownClient = (CUpDownClient*)lpCtrlItem->value;
		switch(nColumn) {

		case 0:		// icon, name, status
			{
				RECT cur_rec = *lpRect;
				// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
				if(NeoPrefs.UseTreeStyle())
					cur_rec.left += 14;
				// NEO: NTS END <-- Xanatos --
				POINT point = {cur_rec.left, cur_rec.top+1};
				//if (lpCtrlItem->type == AVAILABLE_SOURCE){
				//	switch (lpUpDownClient->GetDownloadState()) {
				//	case DS_CONNECTING:
				//		m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
				//		break;
				//	case DS_CONNECTED:
				//		m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
				//		break;
				//	case DS_WAITCALLBACKKAD:
//#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
//					case DS_WAITCALLBACKXS:
//#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
				//	case DS_WAITCALLBACK:
				//		m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
				//		break;
				//	//case DS_ONQUEUE:
				//	//	if(lpUpDownClient->IsRemoteQueueFull())
				//	//		m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				//	//	else
				//	//		m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
				//	//	break;
				//	// NEO: FIX - [SourceCount] -- Xanatos -->
				//	case DS_ONQUEUE:
				//		m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
				//		break;
				//	case DS_REMOTEQUEUEFULL: 
				//		m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				//		break;
				//	// NEO: FIX END <-- Xanatos --
				//	case DS_DOWNLOADING:
				//		m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
				//		break;
				//	case DS_REQHASHSET:
				//		m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
				//		break;
				//	case DS_NONEEDEDPARTS:
				//		m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				//		break;
				//	case DS_ERROR:
				//		m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				//		break;
				//	case DS_TOOMANYCONNS:
				//	case DS_TOOMANYCONNSKAD:
				//		m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
				//		break;
				//	default:
				//		m_ImageList.Draw(dc, 4, point, ILD_NORMAL);
				//	}
				//}
				//else {
				//	m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				//}
				//cur_rec.left += 20;
				//UINT uOvlImg = 0;
				//if ((lpUpDownClient->Credits() && lpUpDownClient->Credits()->GetCurrentIdentState(lpUpDownClient->GetIP()) == IS_IDENTIFIED))
				//	uOvlImg |= 1;
				//if (lpUpDownClient->IsObfuscatedConnectionEstablished())
				//	uOvlImg |= 2;
				//
				//POINT point2= {cur_rec.left,cur_rec.top+1};
				//if (lpUpDownClient->IsFriend())
				//	m_ImageList.Draw(dc, 6, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->GetClientSoft() == SO_EDONKEYHYBRID)
				//	m_ImageList.Draw(dc, 9, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->GetClientSoft() == SO_MLDONKEY)
				//	m_ImageList.Draw(dc, 8, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->GetClientSoft() == SO_SHAREAZA)
				//	m_ImageList.Draw(dc, 10, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->GetClientSoft() == SO_URL)
				//	m_ImageList.Draw(dc, 11, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->GetClientSoft() == SO_AMULE)
				//	m_ImageList.Draw(dc, 12, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->GetClientSoft() == SO_LPHANT)
				//	m_ImageList.Draw(dc, 13, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else if (lpUpDownClient->ExtProtocolAvailable())
				//	m_ImageList.Draw(dc, 5, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//else
				//	m_ImageList.Draw(dc, 7, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				//cur_rec.left += 20;

				// NEO: NCI - [NewClientIcons]
				if (lpCtrlItem->type == AVAILABLE_SOURCE){
					switch (lpUpDownClient->GetDownloadState()) {
					case DS_CONNECTIONRETRY: // NEO: TCR - [TCPConnectionRetry]
					case DS_CONNECTING:
					case DS_HALTED: // NEO: SD - [StandByDL]
					case DS_CONNECTED:
					case DS_WAITCALLBACKKAD:
					case DS_WAITCALLBACK:
					case DS_TOOMANYCONNS:
					case DS_TOOMANYCONNSKAD:
						m_ImageList.Draw(dc, DL_SRC_CONNECTING, point, ILD_NORMAL);		break;
					// NEO: FIX - [SourceCount]
					case DS_ONQUEUE:
						m_ImageList.Draw(dc, DL_SRC_ON_QUEUE, point, ILD_NORMAL);		break;
					case DS_REMOTEQUEUEFULL: 
						m_ImageList.Draw(dc, DL_SRC_NNPFQ, point, ILD_NORMAL);			break;
					// NEO: FIX END
					case DS_DOWNLOADING:
					case DS_REQHASHSET:
						m_ImageList.Draw(dc, DL_SRC_DOWNLOADING, point, ILD_NORMAL);	break;
					case DS_NONEEDEDPARTS:
					case DS_ERROR:
						m_ImageList.Draw(dc, DL_SRC_NNPFQ, point, ILD_NORMAL);			break;
					case DS_CACHED: // NEO: XSC - [ExtremeSourceCache]
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
					case DS_LOADED:
#endif // NEO_SS // NEO: NSS END
					default:
						m_ImageList.Draw(dc, DL_SRC_UNKNOWN, point, ILD_NORMAL);
					}
				}
				else {
					m_ImageList.Draw(dc, DL_SRC_NNPFQ, point, ILD_NORMAL);
				}
				cur_rec.left += 20;
				// NEO: NCI END
			
				POINT point2= {cur_rec.left,cur_rec.top+1};
				switch (lpUpDownClient->GetClientSoft())
				{
					case SO_URL:		m_ImageList.Draw(dc, DL_SERVER, point2, ILD_NORMAL);			break;
					case SO_CDONKEY:	m_ImageList.Draw(dc, DL_CLIENT_CDONKEY,point2, ILD_NORMAL);		break;
					case SO_AMULE: 		m_ImageList.Draw(dc, DL_CLIENT_AMULE, point2, ILD_NORMAL);		break;
					case SO_LPHANT:		m_ImageList.Draw(dc, DL_CLIENT_LPHANT, point2, ILD_NORMAL);		break;
					case SO_EMULEPLUS:	m_ImageList.Draw(dc, DL_CLIENT_EMULEPLUS, point2, ILD_NORMAL);	break;
					case SO_XMULE:		m_ImageList.Draw(dc, DL_CLIENT_XMULE, point2, ILD_NORMAL);		break;
					case SO_HYDRANODE:	m_ImageList.Draw(dc, DL_CLIENT_HYDRANODE, point2, ILD_NORMAL);	break;
					case SO_TRUSTYFILES:m_ImageList.Draw(dc, DL_CLIENT_TRUSTYFILES, point2, ILD_NORMAL);break;
					case SO_SHAREAZA:	m_ImageList.Draw(dc, DL_CLIENT_SHAREAZA, point2, ILD_NORMAL);	break;
					case SO_EDONKEYHYBRID:	m_ImageList.Draw(dc, DL_CLIENT_HYBRID, point2, ILD_NORMAL);	break;
					case SO_MLDONKEY:	m_ImageList.Draw(dc, DL_CLIENT_MLDONKEY, point2, ILD_NORMAL);	break;
					case SO_EMULE:
						if(const EModClient Mod = lpUpDownClient->GetMod())
							m_ImageList.Draw(dc, DL_CLIENT_MOD + ((int)Mod - 1), point2, ILD_NORMAL);
						else
							m_ImageList.Draw(dc, DL_CLIENT_EMULE, point2, ILD_NORMAL);
						break;
					case SO_OLDEMULE:	m_ImageList.Draw(dc, DL_CLIENT_OLDEMULE, point2, ILD_NORMAL);	break;
					case SO_EDONKEY:	m_ImageList.Draw(dc, DL_CLIENT_EDONKEY, point2, ILD_NORMAL);	break;
					case SO_UNKNOWN:	m_ImageList.Draw(dc, DL_CLIENT_UNKNOWN, point2, ILD_NORMAL);	break;
					default:			m_ImageList.Draw(dc, DL_CLIENT_UNKNOWN, point2, ILD_NORMAL);
				}

				if (lpUpDownClient->IsBanned()) 
					m_ImageList.Draw(dc, DL_ARGOS_BANNED, point2, ILD_TRANSPARENT);
				else if (lpUpDownClient->GetFriendSlot())
					m_ImageList.Draw(dc, DL_FRIEND_SLOT_CLIENT, point2, ILD_TRANSPARENT);
				else if (lpUpDownClient->IsFriend())
					m_ImageList.Draw(dc, DL_FRIEND_CLIENT, point2, ILD_TRANSPARENT);
				else if (lpUpDownClient->Credits()){ 
					if (lpUpDownClient->credits->GetScoreRatio(lpUpDownClient->GetIP()) > 1 && lpUpDownClient->credits->GetRemoteScoreRatio() > 1)
						m_ImageList.Draw(dc, DL_CREDIT_UP_DOWN, point2, ILD_TRANSPARENT);
					else if (lpUpDownClient->credits->GetScoreRatio(lpUpDownClient->GetIP()) > 1)
						m_ImageList.Draw(dc, DL_CREDIT_UP, point2, ILD_TRANSPARENT);
					else if (lpUpDownClient->credits->GetRemoteScoreRatio() > 1)
						m_ImageList.Draw(dc, DL_CREDIT_DOWN, point2, ILD_TRANSPARENT);
				}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
				if(lpUpDownClient->IsLanClient()) 
					m_ImageList.Draw(dc, DL_LANCAST_CLIENT, point2, ILD_TRANSPARENT);
				else // Lan Client's dont need SUI
#endif //LANCAST // NEO: NLC END ENC
					if(theApp.clientcredits->CryptoAvailable() && lpUpDownClient->Credits())
						switch(lpUpDownClient->Credits()->GetCurrentIdentState(lpUpDownClient->GetIP())){
							case IS_IDBADGUY:
							case IS_IDFAILED:		m_ImageList.Draw(dc, DL_SECUREHASH_RED, point2, ILD_TRANSPARENT);		break;
							case IS_IDNEEDED:		m_ImageList.Draw(dc, DL_SECUREHASH_YELLOW, point2, ILD_TRANSPARENT); 	break;
							case IS_IDENTIFIED:		m_ImageList.Draw(dc, DL_SECUREHASH_GREEN, point2, ILD_TRANSPARENT); 	break;
							case IS_NOTAVAILABLE:	m_ImageList.Draw(dc, DL_SECUREHASH_BLUE, point2, ILD_TRANSPARENT); 	break;
						}

				if(lpUpDownClient->GetViewSharedFilesSupport() && lpUpDownClient->GetClientSoft() == SO_EMULE)
					m_ImageList.Draw(dc, DL_SHOW_SHARED, point2, ILD_TRANSPARENT);

				if(lpUpDownClient->HasLowID())
					m_ImageList.Draw(dc, DL_LOW_ID_CLIENT, point2, ILD_TRANSPARENT);

				if(lpUpDownClient->IsObfuscatedConnectionEstablished())
					m_ImageList.Draw(dc, DL_ICON_OBFU, point2, ILD_TRANSPARENT);

				cur_rec.left += 20;
				// NEO: NCI EMD <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
				const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() && theApp.ip2country->ShowCountryFlag()
									&& (NeoPrefs.IsIP2CountryShowFlag() == 2 || IsColumnHidden(2));
				if(bShowFlags){
					POINT point3= {cur_rec.left,cur_rec.top+1};
					theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, lpUpDownClient->GetCountryFlagIndex(), point3, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					cur_rec.left+=20;
				}
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

				if (!lpUpDownClient->GetUserName())
					buffer = _T("?");
				else
					buffer = lpUpDownClient->GetUserName();
				dc->DrawText(buffer,buffer.GetLength(),&cur_rec, DLC_DT_TEXT);
			}
			break;

		case 1:		// size
			switch(lpUpDownClient->GetSourceFrom()){
				case SF_SERVER:
					buffer = _T("eD2K Server");
					break;
				case SF_KADEMLIA:
					buffer = GetResString(IDS_KADEMLIA);
					break;
				case SF_SOURCE_EXCHANGE:
					buffer = GetResString(IDS_SE);
					break;
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
				case SF_STORAGE:
					buffer = GetResString(IDS_X_STORAGE);
					break;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
				case SF_PASSIVE:
					buffer = GetResString(IDS_PASSIVE);
					break;
				case SF_LINK:
					buffer = GetResString(IDS_SW_LINK);
					break;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
				case SF_LANCAST:
					buffer = GetResString(IDS_X_LANCAST);
					break;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange] -- Xanatos -->
				case SF_VOODOO:
					buffer = GetResString(IDS_X_VOODOO);
					break;
#endif // VOODOO // NEO: VOODOOx END <-- Xanatos --
			}
			dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;

		case 2:		// transferred
			// MOD - [SessionDL] -- Xanatos -->
			buffer = StrLine(_T("%s (%s)"), lpUpDownClient->Credits() ? CastItoXBytes(lpUpDownClient->Credits()->GetDownloadedTotal(), false, false) : _T("???"),  CastItoXBytes(lpUpDownClient->GetTransferredDown(), false, false));
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;
			// MOD END <-- Xanatos --
		case 3:		// completed
			// MOD - [SessionDL] -- Xanatos -->
			buffer = StrLine(_T("%s (%s)"), lpUpDownClient->Credits() ? CastItoXBytes(lpUpDownClient->Credits()->GetUploadedTotal(), false, false) : _T("???"),  CastItoXBytes(lpUpDownClient->GetTransferredUp(), false, false));
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;
			// MOD END <-- Xanatos --
		case 4:		// speed
			if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetDownloadDatarate()){
				if (lpUpDownClient->GetDownloadDatarate())
					buffer.Format(_T("%s"), CastItoXBytes(lpUpDownClient->GetDownloadDatarate(), false, true));
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 5:		// file info
			{
				CRect rcDraw(*lpRect);
				rcDraw.bottom--; 
				rcDraw.top++; 

				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx;
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !lpCtrlItem->dwUpdated) { 
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					//lpUpDownClient->DrawStatusBar(&cdcStatus,  &rec_status,(lpCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 
					lpUpDownClient->DrawStatusBar(&cdcStatus,  &rec_status, (CPartFile*)lpCtrlItem->owner, thePrefs.UseFlatBar());  // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
			}
			break;

		case 6:		// sources
		{
			//buffer = lpUpDownClient->GetClientSoftVer();
			buffer = lpUpDownClient->DbgGetFullClientSoftVer(); // NEO: MIDI - [ModIDInfo] <-- Xanatos --
			if (buffer.IsEmpty())
				buffer = GetResString(IDS_UNKNOWN);
			CRect rc(lpRect);
			dc->DrawText(buffer, buffer.GetLength(), &rc, DLC_DT_TEXT);
			break;
		}

		case 7:		// prio
			if (lpUpDownClient->GetDownloadState()==DS_ONQUEUE){
				// NEO: FIX - [SourceCount] -- Xanatos --
				/*if (lpUpDownClient->IsRemoteQueueFull()){
					buffer = GetResString(IDS_QUEUEFULL);
					dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				}
				else*/
				{
					if (lpUpDownClient->GetRemoteQueueRank()){
						// NEO: CQR - [CollorQueueRank] -- Xanatos -->
						COLORREF old = (COLORREF)RGB(255,255,255);
						if(lpUpDownClient->GetRemoteQueueRankOld()){
							if(lpUpDownClient->GetRemoteQueueRankOld() > lpUpDownClient->GetRemoteQueueRank()){
								buffer.Format(_T("QR: %u (+%i)"), lpUpDownClient->GetRemoteQueueRank(), lpUpDownClient->GetRemoteQueueRankOld() - lpUpDownClient->GetRemoteQueueRank());
								old = dc->SetTextColor((COLORREF)RGB(10,160,70)); // Green
							}else if (lpUpDownClient->GetRemoteQueueRankOld() < lpUpDownClient->GetRemoteQueueRank()){
								buffer.Format(_T("QR: %u (-%i)"), lpUpDownClient->GetRemoteQueueRank(), lpUpDownClient->GetRemoteQueueRank() - lpUpDownClient->GetRemoteQueueRankOld());
								old = dc->SetTextColor((COLORREF)RGB(190,60,60)); // Red
							}else{
								buffer.Format(_T("QR: %u"), lpUpDownClient->GetRemoteQueueRank());
								old = dc->SetTextColor((COLORREF)RGB(190,190,60)); // Yelow
							}
						}else
							buffer.Format(_T("QR: %u"), lpUpDownClient->GetRemoteQueueRank()); 
						dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
						if(old != (COLORREF)RGB(255,255,255))
							dc->SetTextColor(old);
						// NEO: CQR END <-- Xanatos --
						//buffer.Format(_T("QR: %u"), lpUpDownClient->GetRemoteQueueRank()); 
						//dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
					}
					//else{
					//	dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
					//}
				}
			}
			// NEO: FIX - [SourceCount] -- Xanatos -->
			else if (lpUpDownClient->GetDownloadState()==DS_REMOTEQUEUEFULL){
				buffer = GetResString(IDS_QUEUEFULL);
				dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			// NEO: FIX - [SourceCount] <-- Xanatos --
			else{
				dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;

		case 8:	{	// status
			if (lpCtrlItem->type == AVAILABLE_SOURCE){
				buffer = lpUpDownClient->GetDownloadStateDisplayString();
			}
			else {
				buffer = GetResString(IDS_ASKED4ANOTHERFILE);

// ZZ:DownloadManager -->
                if(thePrefs.IsExtControlsEnabled()) {
                    if(lpUpDownClient->IsInNoNeededList(lpCtrlItem->owner)) {
                        buffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(")");
                    } else if(lpUpDownClient->GetDownloadState() == DS_DOWNLOADING) {
                        buffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(")");
                    } else if(lpUpDownClient->IsSwapSuspended(lpUpDownClient->GetRequestFile())) {
                        buffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(")");
                    }

                    if (lpUpDownClient && lpUpDownClient->GetRequestFile() && lpUpDownClient->GetRequestFile()->GetFileName()){
                        buffer.AppendFormat(_T(": \"%s\""),lpUpDownClient->GetRequestFile()->GetFileName());
                    }
                }
			}

            if(thePrefs.IsExtControlsEnabled() && !lpUpDownClient->m_OtherRequests_list.IsEmpty()) {
                buffer.Append(_T("*"));
            }
// ZZ:DownloadManager <--

			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;
		}
		case 9:		// remaining time & size
			break;
		case 10:	// last seen complete
			break;
		case 11:	// last received
			break;
		case 12:	// category
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
			{
				RECT cur_rec = *lpRect;
				const bool bShowFlags = NeoPrefs.IsIP2CountryShowFlag() == 1 && theApp.ip2country->ShowCountryFlag();
				if(bShowFlags){
					POINT point3= {cur_rec.left,cur_rec.top+1};
					theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, lpUpDownClient->GetCountryFlagIndex(), point3, CSize(18,16), CPoint(0,0), ILD_NORMAL);
					cur_rec.left+=20;
				}
				buffer.Format(_T("%s"), lpUpDownClient->GetCountryName());
				dc->DrawText(buffer,buffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			}
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
			break;
		}
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	CtrlItem_Struct* content = (CtrlItem_Struct*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont;
	//if (m_fontBold.m_hObject){
	if (thePrefs.GetShowActiveDownloadsBold() && m_fontBold.m_hObject){ // NEO: FIX - [ShowActiveDownloadsBold] <-- Xanatos --
		if (content->type == FILE_TYPE){
			if (((const CPartFile*)content->value)->GetTransferringSrcCount())
				pOldFont = dc.SelectObject(&m_fontBold);
			else
				pOldFont = dc.SelectObject(GetFont());
		}
		else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE){
			if (((const CUpDownClient*)content->value)->GetDownloadState() == DS_DOWNLOADING)
				pOldFont = dc.SelectObject(&m_fontBold);
			else
				pOldFont = dc.SelectObject(GetFont());
		}
		else
			pOldFont = dc.SelectObject(GetFont());
	}
	else
		pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	BOOL notLast = lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start=0;
	int tree_end=0;

	int iTreeOffset = 6;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left;
	cur_rec.right -= FILE_ITEM_MARGIN_X;
	cur_rec.left += FILE_ITEM_MARGIN_X;

	if (content->type == FILE_TYPE)
	{
		// NEO: NXC - [NewExtendedCategories] -- Xanatos --
		//if (!g_bLowColorDesktop || (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) {
		//	DWORD dwCatColor = thePrefs.GetCatColor(((/*const*/ CPartFile*)content->value)->GetCategory());
		//	if (dwCatColor > 0)
		//		dc.SetTextColor(dwCatColor);
		//}

		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);
			// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
			if (iColumn == 0 && NeoPrefs.UseTreeStyle()){
				tree_start = cur_rec.left - 1;
				tree_end = cur_rec.left + 1;
			}
			if(iColumn == 5 && !NeoPrefs.UseTreeStyle()) {
			// NEO: NTS END <-- Xanatos --
			//if (iColumn == 5) {
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iTreeOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iTreeOffset;
				DrawFileItem(dc, 5, &cur_rec, content);
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawFileItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}
		}
	}
	else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE)
	{
		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);
			// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
			if (iColumn == 0 && NeoPrefs.UseTreeStyle()){ 
				tree_start = cur_rec.left - 1;
				tree_end = cur_rec.left + 1;
			}
			if(iColumn == 5 && !NeoPrefs.UseTreeStyle()) {
			// NEO: NTS END <-- Xanatos --
			//if (iColumn == 5) {
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iTreeOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iTreeOffset;
				DrawSourceItem(dc, 5, &cur_rec, content);
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawSourceItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}
		}
	}

	//draw rectangle around selected item(s)
	if (content->type == FILE_TYPE && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(notFirst && (GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))) {
			CtrlItem_Struct* prev = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID - 1);
			if(prev->type == FILE_TYPE)
				outline_rec.top--;
		} 

		if(notLast && (GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))) {
			CtrlItem_Struct* next = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1);
			if(next->type == FILE_TYPE)
				outline_rec.bottom++;
		} 

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	//draw focus rectangle around non-highlightable items when they have the focus
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT focus_rec;
		focus_rec.top    = lpDrawItemStruct->rcItem.top;
		focus_rec.bottom = lpDrawItemStruct->rcItem.bottom;
		focus_rec.left   = lpDrawItemStruct->rcItem.left + 1;
		focus_rec.right  = lpDrawItemStruct->rcItem.right - 1;
		dc.FrameRect(&focus_rec, &CBrush(m_crNoFocusLine));
	}

	//draw tree last so it draws over selected and focus (looks better)
	// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
	if(NeoPrefs.UseTreeStyle()){
		if(tree_start < tree_end)
		{
			//set new bounds
			RECT tree_rect;
			tree_rect.top    = lpDrawItemStruct->rcItem.top;
			tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
			tree_rect.left   = tree_start;
			tree_rect.right  = tree_end;
			dc->SetBoundsRect(&tree_rect, DCB_DISABLE);

			//gather some information
			BOOL hasNext = notLast &&
				((CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1))->type != FILE_TYPE;
			BOOL isOpenRoot = hasNext && content->type == FILE_TYPE;
			BOOL isChild = content->type != FILE_TYPE;

			//might as well calculate these now
			int treeCenter = tree_start + 4;
			int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

			//set up a new pen for drawing the tree
			CPen pn, *oldpn;
			pn.CreatePen(PS_SOLID, 1, RGB(128,128,128)/*m_crWindowText*/);
			oldpn = dc->SelectObject(&pn);

			if(isChild)
			{
				//draw the line to the status bar
				dc->MoveTo(tree_end+10, middle);
				dc->LineTo(tree_start + 4, middle);

				//draw the line to the child node
				if(hasNext)
				{
					dc->MoveTo(treeCenter, middle);
					dc->LineTo(treeCenter, cur_rec.bottom + 1);
				}
			}
			else if (isOpenRoot || (content->type == FILE_TYPE /* && itemdata->parts > 1*/))
			{
				//draw box
				RECT circle_rec;
				circle_rec.top    = middle - 5;
				circle_rec.bottom = middle + 4;
				circle_rec.left   = treeCenter - 4;
				circle_rec.right  = treeCenter + 5;
				dc->FrameRect(&circle_rec, &CBrush(RGB(128,128,128)/*m_crWindowText*/));
				CPen penBlack;
				penBlack.CreatePen(PS_SOLID, 1, m_crWindowText);
				CPen* pOldPen2;
				pOldPen2 = dc->SelectObject(&penBlack);


				CPartFile *lpPartFile = (CPartFile*)content->value;

				if ( lpPartFile->GetSourceCount() > 0 || lpPartFile->GetSrcA4AFCount() > 0
					&&  !(lpPartFile->GetStatus() == PS_COMPLETING || lpPartFile->GetStatus() == PS_COMPLETE || lpPartFile->IsStopped()) )
				{
					dc->MoveTo(treeCenter-2,middle - 1);
					dc->LineTo(treeCenter+3,middle - 1);

					if (!lpPartFile->srcarevisible)
					{
						dc->MoveTo(treeCenter,middle-3);
						dc->LineTo(treeCenter,middle+2);
					}
				}
				dc->SelectObject(pOldPen2);
				//draw the line to the child node
				if (hasNext)
				{
					dc->MoveTo(treeCenter, middle + 4);
					dc->LineTo(treeCenter, cur_rec.bottom + 1);
				}
			}

			//draw the line back up to parent node
			if (notFirst && isChild)
			{
				dc->MoveTo(treeCenter, middle);
				dc->LineTo(treeCenter, cur_rec.top - 1);
			}

			//put the old pen back
			dc->SelectObject(oldpn);
			pn.DeleteObject();
		}
	}
	else
	// NEO: NTS END <-- Xanatos --
	if(tree_start < tree_end) {
		//set new bounds
		RECT tree_rect;
		tree_rect.top    = lpDrawItemStruct->rcItem.top;
		tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
		tree_rect.left   = tree_start;
		tree_rect.right  = tree_end;
		dc.SetBoundsRect(&tree_rect, DCB_DISABLE);

		//gather some information
		BOOL hasNext = notLast &&
			((CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1))->type != FILE_TYPE;
		BOOL isOpenRoot = hasNext && content->type == FILE_TYPE;
		BOOL isChild = content->type != FILE_TYPE;
		//BOOL isExpandable = !isChild && ((CPartFile*)content->value)->GetSourceCount() > 0;
		//might as well calculate these now
		int treeCenter = tree_start + 3;
		int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

		//set up a new pen for drawing the tree
		CPen pn, *oldpn;
		pn.CreatePen(PS_SOLID, 1, m_crWindowText);
		oldpn = dc.SelectObject(&pn);

		if(isChild) {
			//draw the line to the status bar
			dc.MoveTo(tree_end, middle);
			dc.LineTo(tree_start + 3, middle);

			//draw the line to the child node
			if(hasNext) {
				dc.MoveTo(treeCenter, middle);
				dc.LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} else if(isOpenRoot) {
			//draw circle
			RECT circle_rec;
			COLORREF crBk = dc.GetBkColor();
			circle_rec.top    = middle - 2;
			circle_rec.bottom = middle + 3;
			circle_rec.left   = treeCenter - 2;
			circle_rec.right  = treeCenter + 3;
			dc.FrameRect(&circle_rec, &CBrush(m_crWindowText));
			dc.SetPixelV(circle_rec.left,      circle_rec.top,    crBk);
			dc.SetPixelV(circle_rec.right - 1, circle_rec.top,    crBk);
			dc.SetPixelV(circle_rec.left,      circle_rec.bottom - 1, crBk);
			dc.SetPixelV(circle_rec.right - 1, circle_rec.bottom - 1, crBk);
			//draw the line to the child node
			if(hasNext) {
				dc.MoveTo(treeCenter, middle + 3);
				dc.LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} /*else if(isExpandable) {
			//draw a + sign
			dc.MoveTo(treeCenter, middle - 2);
			dc.LineTo(treeCenter, middle + 3);
			dc.MoveTo(treeCenter - 2, middle);
			dc.LineTo(treeCenter + 3, middle);
		}*/

		//draw the line back up to parent node
		if(notFirst && isChild) {
			dc.MoveTo(treeCenter, middle);
			dc.LineTo(treeCenter, cur_rec.top - 1);
		}

		//put the old pen back
		dc.SelectObject(oldpn);
		pn.DeleteObject();
	}

	//put the original objects back
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CDownloadListCtrl::HideSources(CPartFile* toCollapse)
{
	SetRedraw(false);
	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	if(toCollapse->srcarevisible==true)
		SetAlternate(FALSE);
	// NEO: SEa END <-- Xanatos --
	int pre = 0;
	int post = 0;
	for (int i = 0; i < GetItemCount(); i++)
	{
		CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData(i);
		if (item->owner == toCollapse)
		{
			pre++;
			item->dwUpdated = 0;
			item->status.DeleteObject();
			DeleteItem(i--);
			post++;
		}
	}
	if (pre - post == 0)
		toCollapse->srcarevisible = false;
	SetRedraw(true);
}

void CDownloadListCtrl::ExpandCollapseItem(int iItem, int iAction, bool bCollapseSource)
{
	if (iItem == -1)
		return;
	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iItem);

	// to collapse/expand files when one of its source is selected
	if (bCollapseSource && content->parent != NULL)
	{
		content=content->parent;
		
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)content;
		iItem = FindItem(&find);
		if (iItem == -1)
			return;
	}

	if (!content || content->type != FILE_TYPE)
		return;
	
	CPartFile* partfile = reinterpret_cast<CPartFile*>(content->value);
	if (!partfile)
		return;

	if (partfile->CanOpenFile()) {
		partfile->OpenFile();
		return;
	}

	// Check if the source branch is disable
	if (!partfile->srcarevisible)
	{
		if (iAction > COLLAPSE_ONLY)
		{
			SetRedraw(false);
			
			// Go throught the whole list to find out the sources for this file
			// Remark: don't use GetSourceCount() => UNAVAILABLE_SOURCE
			for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
			{
				const CtrlItem_Struct* cur_item = it->second;
				if (cur_item->owner == partfile)
				{
					partfile->srcarevisible = true;
					InsertItem(LVIF_PARAM|LVIF_TEXT, iItem+1, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)cur_item);
				}
			}

			SetRedraw(true);
		}
	}
	else {
		if (iAction == EXPAND_COLLAPSE || iAction == COLLAPSE_ONLY)
		{
			if (GetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED) != (LVIS_SELECTED | LVIS_FOCUSED))
			{
				SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetSelectionMark(iItem);
			}
			HideSources(partfile);
		}
	}

	// NEO: SE - [SortExtension] -- Xanatos -->
	if(partfile->srcarevisible == true)
		SetAlternate(TRUE); // NEO: SEa - [SortAltExtension]

	AlterSortArrow();

	uint8 adder=0;
	//if ((GetSortItem()!=9 || !IsAlternate()) || !m_bRemainSort )
	if ((GetSortItem()!=9 && GetSortItem()!=2 && (GetSortItem()!=3 || !IsAlternate())) || !m_bRemainSort ) // MOD - [SessionDL]
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending()?arrowDoubleUp : arrowDoubleDown);
		// MOD - [SessionDL]
		if(GetSortItem()==2)
			adder=18;
		else if(GetSortItem()==3 && IsAlternate())
			adder=27;
		else
		// MOD END
		adder=81;
	}

	if (NeoPrefs.DisableAutoSort() != 1 && IsAlternate())
		SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100) + 2000 + adder); // NEO: SEa - [SortAltExtension]
	// NEO: SE END <-- Xanatos --
}

void CDownloadListCtrl::OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (thePrefs.IsDoubleClickEnabled() || pNMIA->iSubItem > 0)
		ExpandCollapseItem(pNMIA->iItem, EXPAND_COLLAPSE);
	*pResult = 0;
}

void CDownloadListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			// get merged settings
			bool bFirstItem = true;
			int iSelectedItems = 0;
			int iFilesNotDone = 0;
			int iFilesToPause = 0;
			int iFilesToStop = 0;
			int iFilesToResume = 0;
			int iFilesToOpen = 0;
            int iFilesGetPreviewParts = 0;
            int iFilesPreviewType = 0;
			int iFilesToPreview = 0;
			int iFilesToCancel = 0;
			UINT uPrioMenuItem = 0;
			int iFilesProtected = 0; // NEO: PP - [PasswordProtection] <-- Xanatos --
			const CPartFile* file1 = NULL;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData->type != FILE_TYPE)
					continue;
				const CPartFile* pFile = (CPartFile*)pItemData->value;
				if (bFirstItem)
					file1 = pFile;
				iSelectedItems++;

				bool bFileDone = (pFile->GetStatus()==PS_COMPLETE || pFile->GetStatus()==PS_COMPLETING);
				iFilesToCancel += pFile->GetStatus() != PS_COMPLETING ? 1 : 0;
				iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
                iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
                iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
				iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;

				UINT uCurPrioMenuItem = 0;
				if (pFile->IsAutoDownPriority())
					uCurPrioMenuItem = MP_PRIOAUTO;
				else if (pFile->GetDownPriority() == PR_HIGH)
					uCurPrioMenuItem = MP_PRIOHIGH;
				else if (pFile->GetDownPriority() == PR_NORMAL)
					uCurPrioMenuItem = MP_PRIONORMAL;
				else if (pFile->GetDownPriority() == PR_LOW)
					uCurPrioMenuItem = MP_PRIOLOW;
				else
					ASSERT(0);

                if (bFirstItem)
					uPrioMenuItem = uCurPrioMenuItem;
                else if (uPrioMenuItem != uCurPrioMenuItem)
					uPrioMenuItem = 0;

				iFilesProtected += (pFile->IsPWProt()); // NEO: PP - [PasswordProtection] <-- Xanatos --

				bFirstItem = false;
			}

			m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
			m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

			m_FileMenu.EnableMenuItem((UINT_PTR)m_A4AFMenu.m_hMenu, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED); // NEO: MCM - [ManualClientManagement] <-- Xanatos --

			// enable commands if there is at least one item which can be used for the action
			m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesToCancel > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);
			
			// NEO: OCF - [OnlyCompleetFiles] -- Xanatos -->
			m_FileMenu.EnableMenuItem(MP_FORCE, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			//m_FileMenu.CheckMenuItem(MP_FORCE, file1->GetForced() ? MF_CHECKED : MF_UNCHECKED);
			m_FileMenu.SetMenuBitmap(MF_STRING,MP_FORCE, GetResString(IDS_X_DL_FORCE), file1->GetForced() ? _T("FORCE2") : _T("FORCE"));
			// NEO: OCF END <-- Xanatos --

			// NEO: SD - [StandByDL] -- Xanatos -->
			m_FileMenu.EnableMenuItem(MP_STANDBY, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			//m_FileMenu.CheckMenuItem(MP_STANDBY, file1->GetStandBy() ? MF_CHECKED : MF_UNCHECKED);
			m_FileMenu.SetMenuBitmap(MF_STRING,MP_STANDBY, GetResString(IDS_X_DL_STANDBY), file1->GetStandBy() ? _T("STANDBY2") : _T("STANDBY"));
			// NEO: SD END <-- Xanatos --

			// NEO: SC - [SuspendCollecting] -- Xanatos -->
			m_FileMenu.EnableMenuItem(MP_SUSPEND, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			//m_FileMenu.CheckMenuItem(MP_SUSPEND, file1->GetSuspend() ? MF_CHECKED : MF_UNCHECKED);
			m_FileMenu.SetMenuBitmap(MF_STRING,MP_SUSPEND, GetResString(IDS_X_DL_SUSPEND), file1->GetSuspend() ? _T("SUSPEND2") : _T("SUSPEND"));
			// NEO: SC END <-- Xanatos --

			// NEO: PP - [PasswordProtection] -- Xanatos -->
			m_PWProtMenu.EnableMenuItem(MP_PWPROT_SET, (iSelectedItems > 1 || (iSelectedItems == 1 && !iFilesProtected)) ? MF_ENABLED : MF_GRAYED);
			m_PWProtMenu.EnableMenuItem(MP_PWPROT_CHANGE, (iFilesProtected > 0) ? MF_ENABLED : MF_GRAYED);
			m_PWProtMenu.EnableMenuItem(MP_PWPROT_UNSET, (iFilesProtected > 0) ? MF_ENABLED : MF_GRAYED);
			// NEO: PP END <-- Xanatos --

			bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
			m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);
            if(thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio()) {
			    m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesPreviewType == 1 && iFilesToPreview == 0 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
			    m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesGetPreviewParts == 1) ? MF_CHECKED : MF_UNCHECKED);
            }
			m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
			CMenu PreviewMenu;
			PreviewMenu.CreateMenu();
			int iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(PreviewMenu, (iSelectedItems == 1) ? file1 : NULL);
			if (iPreviewMenuEntries)
				m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewMenu.m_hMenu, GetResString(IDS_DL_PREVIEW));

			bool bDetailsEnabled = (iSelectedItems > 0);
			m_FileMenu.EnableMenuItem(MP_METINFO, bDetailsEnabled ? MF_ENABLED : MF_GRAYED);
			if (thePrefs.IsDoubleClickEnabled() && bOpenEnabled)
				m_FileMenu.SetDefaultItem(MP_OPEN);
			else if (!thePrefs.IsDoubleClickEnabled() && bDetailsEnabled)
				m_FileMenu.SetDefaultItem(MP_METINFO);
			else
				m_FileMenu.SetDefaultItem((UINT)-1);
			m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, (iSelectedItems >= 1 /*&& iFilesNotDone == 1*/) ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_TWEAKS, (iSelectedItems > 0 /*iFilesNotDone > 0*/) ? MF_ENABLED : MF_GRAYED); // NEO: FCFG - [FileConfiguration] <-- Xanatos --


			m_FileMenu.EnableMenuItem((UINT_PTR)m_CollectMenu.m_hMenu, (iFilesToStop >= 1) ? MF_ENABLED : MF_GRAYED); // NEO: MSR - [ManualSourceRequest] <-- Xanatos --
			m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, (iFilesToStop >= 1) ? MF_ENABLED : MF_GRAYED); // NEO: MOD - [SourcesMenu] <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
			m_SourcesMenu.EnableMenuItem(MP_LOADSOURCE, (iFilesToStop >= 1) ? MF_ENABLED : MF_GRAYED);
			m_SourcesMenu.EnableMenuItem(MP_SAVESOURCE, (iFilesToStop >= 1) ? MF_ENABLED : MF_GRAYED);
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
			m_SourcesMenu.EnableMenuItem(MP_FINDSOURCES, (iFilesToStop >= 1) ? MF_ENABLED : MF_GRAYED);
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
			m_FileMenu.EnableMenuItem((UINT_PTR)m_DropMenu.m_hMenu, (iFilesToStop >= 1) ? MF_ENABLED : MF_GRAYED); // NEO: MSD - [ManualSourcesDrop] <-- Xanatos --

			// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
			m_A4AFMenu.EnableMenuItem(MP_FORCEA4AF, NeoPrefs.UseSmartA4AFSwapping() && iSelectedItems == 1 && iFilesNotDone == 1? MF_ENABLED : MF_GRAYED);
			//m_A4AFMenu.CheckMenuItem(MP_FORCEA4AF, iSelectedItems == 1 && (theApp.downloadqueue->forcea4af_file == pSingleSelFile) ? MF_CHECKED : MF_UNCHECKED);
			m_FileMenu.SetMenuBitmap(MF_STRING,MP_FORCEA4AF, GetResString(IDS_X_A4AF_FORCEALL), (theApp.downloadqueue->forcea4af_file == file1) ? _T("ADVA4AFALL2") : _T("ADVA4AFALL"));
			// NEO: NXC END <-- Xanatos --

			m_FileMenu.EnableMenuItem(MP_IMPORT, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED); // NEO: PIX - [PartImportExport] <-- Xanatos --
			// NEO: MOD - [SpaceAllocate] -- Xanatos -->
			if(thePrefs.GetAllocCompleteMode() == false)
				m_FileMenu.EnableMenuItem(MP_PREALOCATE, (iSelectedItems > 0 && const_cast<CPartFile*>(file1)->IncompleteAllocateSpace()) ? MF_ENABLED : MF_GRAYED); 
			// NEO: MOD END -- Xanatos --
			m_FileMenu.EnableMenuItem((UINT_PTR)m_TempDirMenu.m_hMenu, (iFilesNotDone > 0) ? MF_ENABLED : MF_GRAYED); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --

			int total;
			m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab, total) > 0 ? MF_ENABLED : MF_GRAYED);

			// NEO: MOD - [SourcesMenu] -- Xanatos --
			//if (m_SourcesMenu && thePrefs.IsExtControlsEnabled()) {
			//	m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_ENABLED);
			//	m_SourcesMenu.EnableMenuItem(MP_ADDSOURCE, (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED);
				//m_SourcesMenu.EnableMenuItem(MP_SETSOURCELIMIT, (iFilesNotDone == iSelectedItems) ? MF_ENABLED : MF_GRAYED); // NEO: SRT - [SourceRequestTweaks] -- Xanatos --
			//}

			m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

			CTitleMenu WebMenu;
			WebMenu.CreateMenu();
			WebMenu.AddMenuTitle(NULL, true);
			int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
			UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

			// create cat-submenue
			CTitleMenu CatsMenu; // NEO: MOD - [CTitleMenu] <-- Xanatos --
			CatsMenu.CreateMenu();
			UpdateCatMenu(CatsMenu,file1 ? file1->GetCategory() : 0); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
			flag = (thePrefs.GetCatCount() == 1) ? MF_GRAYED : MF_ENABLED;
			/*CString label;
			if (thePrefs.GetCatCount()>1) {
				for (int i = 0; i < thePrefs.GetCatCount(); i++){
					if (i>0) {
						label=thePrefs.GetCategory(i)->strTitle;
						label.Replace(_T("&"), _T("&&") );
					}
					CatsMenu.AppendMenu(MF_STRING,MP_ASSIGNCAT+i, (i==0)?GetResString(IDS_CAT_UNASSIGN):label);
				}
			}*/
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT), _T("CATEGORY"));

			// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
			CTitleMenu mnuOrder;
			if (this->GetSelectedCount() > 1) {
				mnuOrder.CreatePopupMenu();
				mnuOrder.AddMenuTitle(NULL, true);
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERAUTOINC, GetResString(IDS_X_CAT_MNUAUTOINC), _T("FILELINEARPRIOAUTO"));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERSTEPTHRU, GetResString(IDS_X_CAT_MNUSTEPTHRU), _T("FILELINEARPRIOMANUAL"));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERALLSAME, GetResString(IDS_X_CAT_MNUALLSAME), _T("FILELINEARPRIOSAME"));
				m_FileMenu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)mnuOrder.m_hMenu, GetResString(IDS_X_CAT_SETORDER), _T("FILELINEARPRIO"));
			}
			else {
				m_FileMenu.AppendMenu(MF_STRING, MP_CAT_SETRESUMEORDER, GetResString(IDS_X_CAT_SETORDER), _T("FILELINEARPRIO"));
			}
			// NEO: NXC END <-- Xanatos --

			// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
			CTitleMenu TempMoveMenu;
			TempMoveMenu.CreateMenu();
			TempMoveMenu.AddMenuTitle(NULL, true);
			if(file1)
				UpdateMTDMenu(TempMoveMenu,file1->GetPath(),true);
			m_TempDirMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)TempMoveMenu.m_hMenu, GetResString(IDS_X_MTD_MOVE), _T("MTD_MOVE"));
			// NEO: MTD END <-- Xanatos --

			GetPopupMenuPos(*this, point);
			m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_TempDirMenu.RemoveMenu(m_TempDirMenu.GetMenuItemCount() - 1, MF_BYPOSITION) ); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
			if (iPreviewMenuEntries)
				VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewMenu.m_hMenu, MF_BYCOMMAND) );
			// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
			m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount()-1,MF_BYPOSITION);
			if (mnuOrder)
				VERIFY( mnuOrder.DestroyMenu() );
			// NEO: NXC END <-- Xanatos --
			VERIFY( WebMenu.DestroyMenu() );
			VERIFY( CatsMenu.DestroyMenu() );
			VERIFY( PreviewMenu.DestroyMenu() );
			VERIFY( TempMoveMenu.DestroyMenu() ); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		}
		else{
			// NEO: MOD - [ClientMenu] -- Xanatos -->
			CPartFile* pSingleSelFile = NULL;
			CUpDownClient* client = NULL;
			int iSelectedItems = 0;
			int iThisFileItems = 0;
			int iOtherFileItems = 0;
			int iLoadingItems = 0;
			bool bFirstItem = true;

			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData->type == FILE_TYPE)
					continue;
				const CUpDownClient* pClient = (CUpDownClient*)pItemData->value;
				if (bFirstItem)
					client = const_cast <CUpDownClient*> (pClient);
				if (bFirstItem)
					pSingleSelFile = (CPartFile*)pItemData->owner;
				iSelectedItems++;

				if(pSingleSelFile == pClient->GetRequestFile())
					iThisFileItems++;
				else
					iOtherFileItems++;

				if(pClient->GetDownloadState() == DS_DOWNLOADING)
					iLoadingItems++;
				bFirstItem = false;
			}
			// NEO: MOD END <-- Xanatos --

			//const CUpDownClient* client = (CUpDownClient*)content->value;
			//CTitleMenu ClientMenu;
			CMenuXP ClientMenu;// NEO: NMX - [NeoMenuXP] <-- Xanatos --
			ClientMenu.CreatePopupMenu();
			ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
			ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
			ClientMenu.SetDefaultItem(MP_DETAIL);
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
			// NEO: MCM - [ManualClientManagement] -- Xanatos -->
			//CTitleMenu A4AFMenu;
			CMenuXP A4AFMenu; // NEO: NMX - [NeoMenuXP]
			A4AFMenu.CreateMenu();
			A4AFMenu.AddMenuTitle(GetResString(IDS_X_SRC_MANAGEMENT), true);
			if (thePrefs.IsExtControlsEnabled()) {
				A4AFMenu.AppendMenu(MF_STRING | ((iOtherFileItems > 0 && client && client->IsEd2kClient() && content->type == UNAVAILABLE_SOURCE) ? MF_ENABLED : MF_GRAYED),MP_SWAP_TO_CLIENT,GetResString(IDS_X_SWAP_TO_CLIENT), _T("SWAPTO"));
				A4AFMenu.AppendMenu(MF_STRING | ((iThisFileItems > 0 && client && client->IsEd2kClient() && content->type == AVAILABLE_SOURCE) ? MF_ENABLED : MF_GRAYED),MP_SWAP_FROM_CLIENT,GetResString(IDS_X_SWAP_FROM_CLIENT), _T("SWAPFROM"));
				A4AFMenu.AppendMenu(MF_STRING | ((iThisFileItems > 0 && client && client->IsEd2kClient() && (content->type == AVAILABLE_SOURCE || client->IsSwapingDisabled()) ) ? MF_ENABLED : MF_GRAYED),MP_LOCK_CLIENT,GetResString((client && client->IsSwapingDisabled()) ? (content->owner == client->GetRequestFile() ? IDS_X_MP_UNLOCK_CLIENT : IDS_X_MP_UNLOCKFROM_CLIENT) : IDS_X_MP_LOCK_CLIENT), (client && client->IsSwapingDisabled()) ? (content->owner == client->GetRequestFile() ? _T("UNLOCK") : _T("UNLOCKFROM")) : _T("LOCKTO"));
				A4AFMenu.AppendMenu(MF_STRING | ((iSelectedItems > 0 && client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED),MP_DROP_CLIENT, GetResString(IDS_X_DROP_CLIENT), _T("DROPCLIENT"));
				A4AFMenu.AppendMenu(MF_STRING | ((iLoadingItems > 0 && client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED),MP_STOP_CLIENT, GetResString(IDS_X_STOP_CLIENT), _T("CANCEL"));
				
				ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_X_SRC_MANAGEMENT), _T("MANAGECLIENT"));
			}
			// NEO: MCM END <-- Xanatos --
			if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
				ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
			ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

			// NEO: MCM - [ManualClientManagement] -- Xanatos --
			/*CMenu A4AFMenu;
			A4AFMenu.CreateMenu();
			if (thePrefs.IsExtControlsEnabled()) {
// ZZ:DownloadManager -->
#ifdef _DEBUG
                if (content->type == UNAVAILABLE_SOURCE) {
                    A4AFMenu.AppendMenu(MF_STRING,MP_A4AF_CHECK_THIS_NOW,GetResString(IDS_A4AF_CHECK_THIS_NOW));
                }
# endif
// <-- ZZ:DownloadManager
				if (A4AFMenu.GetMenuItemCount()>0)
					ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_A4AF));
			}*/

			GetPopupMenuPos(*this, point);
			ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			
			VERIFY( A4AFMenu.DestroyMenu() );
			VERIFY( ClientMenu.DestroyMenu() );
		}
	}
	else{	// nothing selected
		int total;
		m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem((UINT_PTR)m_A4AFMenu.m_hMenu, MF_GRAYED); // NEO: MCM - [ManualClientManagement] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_CANCEL, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_FORCE,MF_GRAYED); // NEO: OCF - [OnlyCompleetFiles] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_STANDBY,MF_GRAYED); // NEO: SD - [StandByDL] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_SUSPEND,MF_GRAYED); // NEO: SC - [SuspendCollecting] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);

		// NEO: PP - [PasswordProtection] -- Xanatos -->
		m_PWProtMenu.EnableMenuItem(MP_PWPROT_SET, MF_GRAYED);
		m_PWProtMenu.EnableMenuItem(MP_PWPROT_CHANGE, MF_GRAYED);
		m_PWProtMenu.EnableMenuItem(MP_PWPROT_UNSET, MF_GRAYED);
		// NEO: PP END <-- Xanatos --

		if (thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio()) {
			m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_GRAYED);
			m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_UNCHECKED);
        }
		m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_TWEAKS, MF_GRAYED); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_IMPORT, MF_GRAYED); // NEO: PIX - [PartImportExport] <-- Xanatos --
		// NEO: MOD - [SpaceAllocate] -- Xanatos -->
		if(thePrefs.GetAllocCompleteMode() == false)
			m_FileMenu.EnableMenuItem(MP_PREALOCATE, MF_GRAYED);
		// NEO: MOD END <-- Xanatos --
		m_FileMenu.EnableMenuItem((UINT_PTR)m_TempDirMenu.m_hMenu, MF_GRAYED); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab,total) > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);
		m_FileMenu.EnableMenuItem((UINT_PTR)m_CollectMenu.m_hMenu, MF_GRAYED); // NEO: MSR - [ManualSourceRequest] <-- Xanatos --
		//if (m_SourcesMenu) // NEO: MOD - [SourcesMenu] <-- Xanatos --
		m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem((UINT_PTR)m_DropMenu.m_hMenu, MF_GRAYED); // NEO: MSD - [ManualSourcesDrop] <-- Xanatos --
		m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

		// also show the "Web Services" entry, even if its disabled and therefore not useable, it though looks a little 
		// less confusing this way.
		CTitleMenu WebMenu;
		WebMenu.CreateMenu();
		WebMenu.AddMenuTitle(NULL, true);
		theWebServices.GetFileMenuEntries(&WebMenu);
		m_FileMenu.AppendMenu(MF_POPUP | MF_GRAYED, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

		GetPopupMenuPos(*this, point);
		m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
		VERIFY( WebMenu.DestroyMenu() );
	}
}

BOOL CDownloadListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
		case MP_PASTE:
			if (theApp.IsEd2kFileLinkInClipboard())
				theApp.PasteClipboard(curTab);
			return TRUE;
		case MP_FIND:
			OnFindStart();
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel == -1)
		iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			//for multiple selections 
			UINT selectedCount = 0;
			CTypedPtrList<CPtrList, CPartFile*> selectedList; 
			POSITION pos = GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = GetNextSelectedItem(pos);
				if(index > -1) 
				{
					if (((const CtrlItem_Struct*)GetItemData(index))->type == FILE_TYPE)
					{
						selectedCount++;
						selectedList.AddTail((CPartFile*)((const CtrlItem_Struct*)GetItemData(index))->value);
					}
				} 
			} 

			CPartFile* file = (CPartFile*)content->value;
			switch (wParam)
			{
				case MP_MTD_UNLOAD: // NEO: MTD - [MultiTempDirectories]
				case MP_CANCEL:
				case MPG_DELETE: // keyboard del will continue to remove completed files from the screen while cancel will now also be available for complete files
				{
					if (selectedCount > 0)
					{
						SetRedraw(false);
						CString fileList;
						bool validdelete = false;
						bool removecompl = false;
						int cFiles = 0;
						const int iMaxDisplayFiles = 10;
						for (pos = selectedList.GetHeadPosition(); pos != 0; )
						{
							CPartFile* cur_file = selectedList.GetNext(pos);
							if (cur_file->GetStatus() != PS_COMPLETING && (cur_file->GetStatus() != PS_COMPLETE || wParam == MP_CANCEL)){
								validdelete = true;
								cFiles++;
								if (cFiles < iMaxDisplayFiles)
									fileList.Append(_T("\n") + CString(cur_file->GetFileName()));
								else if(cFiles == iMaxDisplayFiles && pos != NULL)
									fileList.Append(_T("\n..."));
							}
							else if (cur_file->GetStatus() == PS_COMPLETE)
								removecompl = true;
						}
						CString quest;
						// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
						bool bUnloadOnly = false;
						if(wParam == MP_MTD_UNLOAD){
							bUnloadOnly = true;
							if (selectedCount == 1)
								quest = GetResString(IDS_X_Q_UNLOADDL2);
							else
								quest = GetResString(IDS_X_Q_UNLOADDL);
						}else
						// NEO: MTD END <-- Xanatos --
						if (selectedCount == 1)
							quest = GetResString(IDS_Q_CANCELDL2);
						else
							quest = GetResString(IDS_Q_CANCELDL);
						if ((removecompl && !validdelete) || (validdelete && AfxMessageBox(quest + fileList, MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDYES))
						{
							bool bRemovedItems = false;
							while (!selectedList.IsEmpty())
							{
								HideSources(selectedList.GetHead());
								switch (selectedList.GetHead()->GetStatus())
								{
									// NEO: SSH - [SlugFillerSafeHash] -- Xanatos --
									//case PS_WAITINGFORHASH:
									//case PS_HASHING:
									case PS_IMPORTING: // NEO: PIX - [PartImportExport] <-- Xanatos --
									case PS_MOVING: // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
									case PS_COMPLETING:
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									case PS_COMPLETE:
										if (wParam == MP_CANCEL){
											BOOL delsucc = FALSE;
											if (!PathFileExists(selectedList.GetHead()->GetFilePath()))
												delsucc = TRUE;
											else{
												// Delete
												if (!thePrefs.GetRemoveToBin()){
													delsucc = DeleteFile(selectedList.GetHead()->GetFilePath());
												}
												else{
													// delete to recycle bin :(
													TCHAR todel[MAX_PATH+1];
													memset(todel, 0, sizeof todel);
													_tcsncpy(todel, selectedList.GetHead()->GetFilePath(), ARRSIZE(todel)-2);

													SHFILEOPSTRUCT fp = {0};
													fp.wFunc = FO_DELETE;
													fp.hwnd = theApp.emuledlg->m_hWnd;
													fp.pFrom = todel;
													fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
													delsucc = (SHFileOperation(&fp) == 0);
												}
											}
											if (delsucc){
												theApp.sharedfiles->RemoveFile(selectedList.GetHead());
											}
											else{
												CString strError;
												strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), selectedList.GetHead()->GetFilePath(), GetErrorMessage(GetLastError()));
												AfxMessageBox(strError);
											}
										}
										RemoveFile(selectedList.GetHead());
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									case PS_PAUSED:
										//selectedList.GetHead()->DeleteFile();
										selectedList.GetHead()->DeleteFile(bUnloadOnly); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									default:
										if (selectedList.GetHead()->GetCategory())
											theApp.downloadqueue->StartNextFileIfPrefs(selectedList.GetHead()->GetCategory());
										//selectedList.GetHead()->DeleteFile();
										selectedList.GetHead()->DeleteFile(bUnloadOnly); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
										selectedList.RemoveHead();
										bRemovedItems = true;
								}
							}
							if (bRemovedItems)
							{
								AutoSelectItem();
								theApp.emuledlg->transferwnd->UpdateCatTabTitles();
							}
						}
						SetRedraw(true);
					}
					break;
				}
				// NEO: CI#3 - [CodeImprovement]  -- Xanatos -->
				case MP_PRIOLOW:
				case MP_PRIONORMAL:
				case MP_PRIOHIGH:
				case MP_PRIOAUTO:
				{
					uint8 uPriority = PR_NORMAL;
					switch (wParam) {
						case MP_PRIOLOW:		uPriority = PR_LOW;		break;
						case MP_PRIONORMAL:		uPriority = PR_NORMAL;	break;
						case MP_PRIOHIGH:		uPriority = PR_HIGH;	break;
						case MP_PRIOAUTO:		uPriority = PR_AUTO;	break;
					}
					SetRedraw(false);
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CPartFile* file = selectedList.GetNext(pos);
						if(uPriority == PR_AUTO){
							file->SetAutoDownPriority(true);
							file->SetDownPriority(PR_HIGH);
						}else{
							file->SetAutoDownPriority(false);
							file->SetDownPriority(uPriority);
						}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
						if(!file->IsVoodooFile() && file->KnownPrefs->IsEnableVoodoo())
							theApp.voodoo->ManifestDownloadInstruction(file,INST_DL_PRIO,uPriority);
#endif // VOODOO // NEO: VOODOO END
					}
					SetRedraw(true);
					break;
				}
				// NEO: CI#3 EMD <-- Xanatos --
				case MP_PAUSE:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanPauseFile())
							partfile->PauseFile();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_RESUME:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanResumeFile()){
							if (partfile->GetStatus() == PS_INSUFFICIENT)
								partfile->ResumeFileInsufficient();
							else
								partfile->ResumeFile();
						}
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				// NEO: OCF - [OnlyCompleetFiles] -- Xanatos -->
				case MP_FORCE:{
					SetRedraw(false);
					bool bNewState = !file->GetForced();
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->SetForced(bNewState);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				}
				// NEO: END <-- Xanatos --
				// NEO: SD - [StandByDL] -- Xanatos -->
				case MP_STANDBY:{
					SetRedraw(false);
					bool bNewState = !file->GetStandBy();
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->SetStandBy(bNewState);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				}
				// NEO: SD END <-- Xanatos --
				// NEO: SC - [SuspendCollecting] -- Xanatos -->
				case MP_SUSPEND:{
					SetRedraw(false);
					bool bNewState = !file->GetSuspend();
					while (!selectedList.IsEmpty()){
						selectedList.GetHead()->SetSuspend(bNewState);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				}
				// NEO: SC END <-- Xanatos --
				case MP_STOP:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.GetHead();
						if (partfile->CanStopFile()){
							HideSources(partfile);
							partfile->StopFile();
						}
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					break;
				case MP_CLEARCOMPLETED:
					SetRedraw(false);
					ClearCompleted();
					SetRedraw(true);
					break;
				case MPG_F2:
					if (GetAsyncKeyState(VK_CONTROL) < 0 || selectedCount > 1) {
						// when ctrl is pressed -> filename cleanup
						if (IDYES==AfxMessageBox(GetResString(IDS_MANUAL_FILENAMECLEANUP),MB_YESNO))
							while (!selectedList.IsEmpty()){
								CPartFile *partfile = selectedList.GetHead();
								if (partfile->IsPartFile()) {
									partfile->SetFileName(CleanupFilename(partfile->GetFileName()));
								}
								selectedList.RemoveHead();
							}
					} else {
						if (file->GetStatus() != PS_COMPLETE && file->GetStatus() != PS_COMPLETING)
						{
							InputBox inputbox;
							CString title = GetResString(IDS_RENAME);
							title.Remove(_T('&'));
							inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
							inputbox.SetEditFilenameMode();
							if (inputbox.DoModal()==IDOK && !inputbox.GetInput().IsEmpty() && IsValidEd2kString(inputbox.GetInput()))
							{
								file->SetFileName(inputbox.GetInput(), true);
								file->UpdateDisplayedInfo();
								file->SavePartFile();
							}
						}
						else
							MessageBeep(MB_OK);
					}
					break;
				case MP_METINFO:
				case MPG_ALTENTER:
					ShowFileDialog(0);
					break;
				case MP_COPYSELECTED:
				case MP_GETED2KLINK:{
					CString str;
					while (!selectedList.IsEmpty()){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += CreateED2kLink(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					theApp.CopyTextToClipboard(str);
					break;
				}
				case MP_SEARCHRELATED:
					theApp.emuledlg->searchwnd->SearchRelatedFiles(selectedList);
					theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
					break;
				// NEO: PIX - [PartImportExport] -- Xanatos -->
				case MP_IMPORT:
					if(selectedCount > 1)
						break;
					file->ImportParts();
					break;
				// NEO: PIX <-- Xanatos --
				// NEO: MOD - [SpaceAllocate] -- Xanatos -->
				case MP_PREALOCATE:
					while (!selectedList.IsEmpty()){
						if(selectedList.GetHead()->IncompleteAllocateSpace())
							selectedList.GetHead()->AllocateNeededSpace();
						selectedList.RemoveHead();
					}
					break;
				// NEO: MOD END <-- Xanatos --
				case MP_OPEN:
				case IDA_ENTER:
					if (selectedCount > 1)
						break;
					if (file->CanOpenFile())
						file->OpenFile();
					break;
				case MP_TRY_TO_GET_PREVIEW_PARTS:
					if (selectedCount > 1)
						break;
                    file->SetPreviewPrio(!file->GetPreviewPrio());
                    break;
				case MP_PREVIEW:
					if (selectedCount > 1)
						break;
					file->PreviewFile();
					break;
				case MP_VIEWFILECOMMENTS:
					ShowFileDialog(IDD_COMMENTLST);
					break;
				// NEO: FCFG - [FileConfiguration] -- Xanatos -->
				case MP_TWEAKS:
					ShowFileDialog(NULL, TRUE);
					break;
				// NEO: FCFG END <-- Xanatos --
				case MP_SHOWED2KLINK:
					ShowFileDialog(IDD_ED2KLINK);
					break;
				// NEO: MCM - [ManualClientManagement] -- Xanatos -->
				case MP_SWAP_TO_A4AF:
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->CollectAllA4AF();
						selectedList.RemoveHead();
					}
					break;
				case MP_SWAP_FROM_A4AF:
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->ReleaseAllA4AF();
						selectedList.RemoveHead();
					}
					break;
				// NEO: MCM END <-- Xanatos --
				// NEO: SRT - [SourceRequestTweaks] -- Xanatos --
				/*case MP_SETSOURCELIMIT: {
					CString temp;
					temp.Format(_T("%u"),file->GetPrivateMaxSources());
					InputBox inputbox;
					CString title = GetResString(IDS_SETPFSLIMIT);
					inputbox.SetLabels(title, GetResString(IDS_SETPFSLIMITEXPLAINED), temp );

					if (inputbox.DoModal() == IDOK)
					{
						temp = inputbox.GetInput();
						int newlimit = _tstoi(temp);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetPrivateMaxSources(newlimit);
							selectedList.RemoveHead();
							partfile->UpdateDisplayedInfo(true);
						}
					}
					break;
				}*/
				// NEO: PP - [PasswordProtection] -- Xanatos -->
				case MP_PWPROT_SET:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_OTHER,INST_OTHER_PROTECT_SET);
					break;
				case MP_PWPROT_CHANGE:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_OTHER,INST_OTHER_PROTECT_CHANGE);
					break;
				case MP_PWPROT_UNSET:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_OTHER,INST_OTHER_PROTECT_UNSET);
					break;
				// NEO: PP END <-- Xanatos --
				// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
				// This is only called when there is a single selection, so we'll handle it thusly.
				case MP_CAT_SETRESUMEORDER: {
					InputBox	inputOrder;
					CString		currOrder;

					currOrder.Format(_T("%u"), file->GetCatResumeOrder());
					inputOrder.SetLabels(GetResString(IDS_X_CAT_SETORDER), GetResString(IDS_X_CAT_ORDER), currOrder);
					inputOrder.SetNumber(true);
					if (inputOrder.DoModal() == IDOK)
					{
					int newOrder = inputOrder.GetInputInt();
						if  (newOrder < 0 || newOrder == file->GetCatResumeOrder()) break;

					file->SetCatResumeOrder(newOrder);
					Invalidate(); // Display the new category.
					}
					break;
				}
				// These next three are only called when there are multiple selections.
				case MP_CAT_ORDERAUTOINC: {
					// This option asks the user for a starting point, and then increments each selected item
					// automatically.  It uses whatever order they appear in the list, from top to bottom.
					InputBox	inputOrder;
					if (selectedCount <= 1) break;
						
					inputOrder.SetLabels(GetResString(IDS_X_CAT_SETORDER), GetResString(IDS_X_CAT_EXPAUTOINC), _T("0"));
					inputOrder.SetNumber(true);
                    if (inputOrder.DoModal() == IDOK)
					{
					int newOrder = inputOrder.GetInputInt();
					if  (newOrder < 0) break;

					while (!selectedList.IsEmpty()) {
						selectedList.GetHead()->SetCatResumeOrder(newOrder);
						newOrder++;
						selectedList.RemoveHead();
					}
					Invalidate();
					}
					break;
				}
				case MP_CAT_ORDERSTEPTHRU: {
					// This option asks the user for a different resume modifier for each file.  It
					// displays the filename in the inputbox so that they don't get confused about
					// which one they're setting at any given moment.
					InputBox	inputOrder;
					CString		currOrder;
					CString		currFile;
					CString		currInstructions;
					int			newOrder = 0;

					if (selectedCount <= 1) break;
					inputOrder.SetNumber(true);

					while (!selectedList.IsEmpty()) {
						currOrder.Format(_T("%u"), selectedList.GetHead()->GetCatResumeOrder());
						currFile = selectedList.GetHead()->GetFileName();
                        if (currFile.GetLength() > 50) currFile = currFile.Mid(0,47) + _T("...");
						currInstructions.Format(_T("%s %s"), GetResString(IDS_X_CAT_EXPSTEPTHRU), currFile);
						inputOrder.SetLabels(GetResString(IDS_X_CAT_SETORDER), currInstructions, currOrder);

						if (inputOrder.DoModal() == IDCANCEL) {
							if (MessageBox(GetResString(IDS_X_CAT_ABORTSTEPTHRU), GetResString(IDS_X_ABORT), MB_YESNO) == IDYES) {
								break;
							}
							else {
								selectedList.RemoveHead();
								continue;
							}
						}

						newOrder = inputOrder.GetInputInt();
						selectedList.GetHead()->SetCatResumeOrder(newOrder);
						selectedList.RemoveHead();
					}
					RedrawItems(0, GetItemCount() - 1);
					break;
				}
				case MP_CAT_ORDERALLSAME: {
					// This option asks the user for a single resume modifier and applies it to
					// all the selected files.
					InputBox	inputOrder;
					CString		currOrder;

					if (selectedCount <= 1) break;

					inputOrder.SetLabels(GetResString(IDS_X_CAT_SETORDER), GetResString(IDS_X_CAT_EXPALLSAME), _T("0"));
					inputOrder.SetNumber(true);
					if (inputOrder.DoModal() == IDCANCEL)
						break;

					int newOrder = inputOrder.GetInputInt();
					if  (newOrder < 0) break;

					while (!selectedList.IsEmpty()) {
						selectedList.GetHead()->SetCatResumeOrder(newOrder);
						selectedList.RemoveHead();
					}
					RedrawItems(0, GetItemCount() - 1);
					break;
				}
				case MP_FORCEA4AF: {
					if (file && theApp.downloadqueue->forcea4af_file != file)
						theApp.downloadqueue->forcea4af_file = file;
					else if (file && theApp.downloadqueue->forcea4af_file == file)
						theApp.downloadqueue->forcea4af_file = NULL;
					break;
				}
				// NEO: NXC END <-- Xanatos --
				// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
				case MP_COLLECT_ALL_SOURCES: 
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_COLLECT_ALL_SOURCES);
					break;
				case MP_COLLECT_XS_SOURCES:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_COLLECT_XS_SOURCES);
					break;
				case MP_COLLECT_SVR_SOURCES:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_COLLECT_SVR_SOURCES);
					break;
				case MP_COLLECT_KAD_SOURCES:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_COLLECT_KAD_SOURCES);
					break;
				case MP_COLLECT_UDP_SOURCES:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_COLLECT_UDP_SOURCES);
					break;
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
				case MP_COLLECT_VOODOO_SOURCES:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_COLLECT_VOODOO_SOURCES);
#endif // VOODOO // NEO: VOODOOx END
				//
				case MP_AHL_INCREASE:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_AHL_INCREASE); 
					break;
				case MP_AHL_DECREASE:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_COLLECT,INST_AHL_DECREASE); 
					break;
				// NEO: MSH END <-- Xanatos --
				case MP_ADDSOURCE: {
					if (selectedCount > 1)
						break;
					// NEO: MLD - [ModelesDialogs] -- Xanatos -->
					CAddSourceDlg* dlg = new CAddSourceDlg();
					dlg->SetFile(file);
					dlg->OpenDialog(); 
					// NEO: MLD END <-- Xanatos --
					//CAddSourceDlg as;
					//as.SetFile(file);
					//as.DoModal();
					break;
				}
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
				case MP_LOADSOURCE:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_STORAGE,INST_STORAGE_LOAD); 
					break;
				case MP_SAVESOURCE:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_STORAGE,INST_STORAGE_SAVE);
					break;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
				case MP_FINDSOURCES:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_STORAGE,INST_STORAGE_FIND);
					break;
#endif // NEO_CD // NEO: SFL END <-- Xanatos --

				// NEO: MSD - [ManualSourcesDrop] -- Xanatos -->
				case MP_DROP_NNP:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_NNP);
					break;
				case MP_DROP_FULLQ:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_FULLQ);
					break;
				case MP_DROP_HIGHQ:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_HIGHQ);
					break;
				// NEO: TCR - [TCPConnectionRetry]
				case MP_DROP_WAITINGRETRY:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_WAITINGRETRY);
					break;
				// NEO: TCR END
				// NEO: XSC - [ExtremeSourceCache]
				case MP_DROP_CACHED:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_CACHED);
					break;
				// NEO: XSC END
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				case MP_DROP_LOADED:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_LOADED);
					break;
#endif // NEO_SS // NEO: NSS END
				case MP_DROP_TOMANY:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_TOMANY);
					break;
				case MP_DROP_LOW2LOW:
					theApp.downloadqueue->ExecuteNeoCommand(selectedList,INST_DROP,INST_DROP_LOW2LOW);
					break;
				// NEO: MSD END <-- Xanatos --

				default:
					// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
					if (wParam>=MP_TEMPLIST && wParam<=MP_TEMPLIST+99) {
						CString tempdir = thePrefs.GetTempDir(wParam-MP_TEMPLIST);
						
						if (selectedCount > 0)
						{
							SetRedraw(false);
							CString fileList;
							bool validmove = false;
							for (pos = selectedList.GetHeadPosition(); pos != 0; )
							{
								CPartFile* cur_file = selectedList.GetNext(pos);
								if (cur_file->GetStatus() != PS_COMPLETING && /*cur_file->GetStatus() != PS_HASHING &&*/ cur_file->GetStatus() != PS_MOVING && cur_file->GetStatus() != PS_IMPORTING){ // NEO: PIX - [PartImportExport] // NEO: SSH - [SlugFillerSafeHash]
									validmove = true;
									if (selectedCount < 50)
										fileList.Append(_T("\n") + CString(cur_file->GetFileName()));
								}
							}

							CString quest;
							if (selectedCount == 1)
								quest = GetResString(IDS_X_TEMPMOVE2);
							else
								quest = GetResString(IDS_X_TEMPMOVE);
							if (validmove && AfxMessageBox(quest + fileList, MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDYES)
							{
								//bool bMovedItems = false;
								while (!selectedList.IsEmpty())
								{
									CPartFile* cur_file = selectedList.GetHead();
									selectedList.RemoveHead();

									if(!CompareDirectories(cur_file->GetPath(), tempdir))
										continue;

									if (cur_file->GetStatus() != PS_COMPLETING && /*cur_file->GetStatus() != PS_HASHING &&*/ cur_file->GetStatus() != PS_MOVING && cur_file->GetStatus() != PS_IMPORTING){ // NEO: PIX - [PartImportExport] // NEO: SSH - [SlugFillerSafeHash]
										cur_file->MovePartFile(tempdir);
										//bMovedItems = true;
									}
									
								}
								//if (bMovedItems)
								//	AutoSelectItem();
							}
							SetRedraw(true);
						}
					}else
					// NEO: MTD END <-- Xanatos --
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){
						theWebServices.RunURL(file, wParam);
					}
					else if (wParam>=MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99){
						SetRedraw(FALSE);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetCategory(wParam - MP_ASSIGNCAT);
							partfile->UpdateDisplayedInfo(true);
							selectedList.RemoveHead();
						}
						SetRedraw(TRUE);
						UpdateCurrentCategoryView();
						if (thePrefs.ShowCatTabInfos())
							theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					}
					else if (wParam>=MP_PREVIEW_APP_MIN && wParam<=MP_PREVIEW_APP_MAX){
						thePreviewApps.RunApp(file, wParam);
					}
					break;
			}
		}
		else{
			CUpDownClient* client = (CUpDownClient*)content->value;

			// NEO: MOD - [ClientMenu] -- Xanatos -->
			CPartFile* file = (CPartFile*)content->owner;
			UINT selectedCount = 0;
			CTypedPtrList<CPtrList, CUpDownClient*> selectedList; 
			POSITION pos = GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = GetNextSelectedItem(pos);
				if(index > -1) 
				{
					if (((const CtrlItem_Struct*)GetItemData(index))->type != FILE_TYPE)
					{
						selectedCount++;
						selectedList.AddTail((CUpDownClient*)((const CtrlItem_Struct*)GetItemData(index))->value);
					}
				} 
			} 
			// NEO: MOD END <-- Xanatos --

			switch (wParam){
				case MP_SHOWLIST:
					//client->RequestSharedFileList();
					// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
					{
						// NEO: MLD - [ModelesDialogs]
						CClientDetailDialog* dlg = new CClientDetailDialog(client, this, IDD_BROWSEFILES);
						dlg->OpenDialog(); 
						// NEO: MLD END
						//CClientDetailDialog dialog(client, this, IDD_BROWSEFILES);
						//dialog.DoModal();
					}
					// NEO: XSF END <-- Xanatos --
					break;
				case MP_MESSAGE:
					theApp.emuledlg->chatwnd->StartSession(client);
					break;
				case MP_ADDFRIEND:
					if (theApp.friendlist->AddFriend(client))
						UpdateItem(client);
					break;
				case MP_DETAIL:
				case MPG_ALTENTER:
					ShowClientDialog(client);
					break;
				// NEO: MCM - [ManualClientManagement] -- Xanatos -->
				case MP_SWAP_TO_CLIENT:
					while (!selectedList.IsEmpty()){
						if (selectedList.GetHead()->GetDownloadState() != DS_DOWNLOADING && selectedList.GetHead()->GetRequestFile() != file)
						{
							selectedList.GetHead()->DoSwap(file, false, _T("Manual: Swap to this File"),true); // ZZ:DownloadManager
							UpdateItem(file);
						}
						selectedList.RemoveHead();
					}
					break;
				case MP_SWAP_FROM_CLIENT:
					while (!selectedList.IsEmpty()){
						if (selectedList.GetHead()->GetDownloadState() != DS_DOWNLOADING && selectedList.GetHead()->GetRequestFile() == file)
						{
							selectedList.GetHead()->SwapToAnotherFile(_T("Manual: Swap to an Other File"), false, true, false, NULL, true, true, true); // ZZ:DownloadManager
							UpdateItem(file);
						}
						selectedList.RemoveHead();
					}
					break;
				case MP_LOCK_CLIENT:
					while (!selectedList.IsEmpty() && selectedList.GetHead()->GetRequestFile() == file){
						selectedList.GetHead()->DisableSwaping(!client->IsSwapingDisabled());
						UpdateItem(file);
						selectedList.RemoveHead();
					}
					break;
				case MP_DROP_CLIENT:
					while (!selectedList.IsEmpty()){
						theApp.downloadqueue->RemoveSource(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					break;
				case MP_STOP_CLIENT:
					while (!selectedList.IsEmpty()){
						if (selectedList.GetHead()->GetDownloadState() == DS_DOWNLOADING && selectedList.GetHead()->GetRequestFile() == file) {
							selectedList.GetHead()->SendCancelTransfer();
							selectedList.GetHead()->SetDownloadState(DS_ONQUEUE);
							UpdateItem(file);
						}
						selectedList.RemoveHead();
					}
					break;
				// NEO: MCM END <-- Xanatos --
				case MP_BOOT:
					if (client->GetKadPort())
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
					break;
				// NEO: MCM - [ManualClientManagement] -- Xanatos --
/*// ZZ:DownloadManager -->
#ifdef _DEBUG
				case MP_A4AF_CHECK_THIS_NOW: {
					CPartFile* file = (CPartFile*)content->owner;
					if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if (client->GetDownloadState() != DS_DOWNLOADING)
						{
							client->SwapToAnotherFile(_T("Manual init of source check. Test to be like ProcessA4AFClients(). CDownloadListCtrl::OnCommand() MP_SWAP_A4AF_DEBUG_THIS"), false, false, false, NULL, true, true, true); // ZZ:DownloadManager
							UpdateItem(file);
						}
					}
					break;
				}
#endif
// <-- ZZ:DownloadManager*/
			}
		}
	}
	else /*nothing selected*/
	{
		switch (wParam){
			case MP_CLEARCOMPLETED:
				ClearCompleted();
				break;
		}
	}

	// NEO: PP - [PasswordProtection] -- Xanatos -->
	switch (wParam){
		case MP_PWPROT_SHOW:
			SetRedraw(false);
			theApp.emuledlg->transferwnd->NeoCommand(INST_OTHER,INST_OTHER_PROTECT_SHOW);
			SetRedraw(true);
			break;
		case MP_PWPROT_HIDE:
			SetRedraw(false);
			theApp.emuledlg->transferwnd->NeoCommand(INST_OTHER,INST_OTHER_PROTECT_HIDE);
			SetRedraw(true);
			break;
	}
	// NEO: PP END <-- Xanatos --

	return TRUE;
}

void CDownloadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = GetSortItem();
	bool m_oldSortAscending = GetSortAscending();

	//if (sortItem==9) {
	if (sortItem==9 || sortItem==2 || (sortItem==3 && IsAlternate())) {  // MOD - [SessionDL] <-- Xanatos --
		m_bRemainSort=(sortItem != pNMListView->iSubItem) ? false : (m_oldSortAscending?m_bRemainSort:!m_bRemainSort);
	}

	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem;
	UpdateSortHistory(sortItem + (sortAscending ? 0:100), 100);
	
	// Save new preferences
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);
	
	// Sort table
	uint8 adder=0;
	//if (sortItem!=9 || !m_bRemainSort)
	if ((sortItem!=9 && GetSortItem()!=2 && (GetSortItem()!=3 || !IsAlternate())) || !m_bRemainSort ) // MOD - [SessionDL] <-- Xanatos --
		SetSortArrow(sortItem, sortAscending);
	else {
        SetSortArrow(sortItem, sortAscending?arrowDoubleUp : arrowDoubleDown);
		// MOD - [SessionDL] -- Xanatos -->
		if(GetSortItem()==2)
			adder=18;
		else if(GetSortItem()==3 && IsAlternate())
			adder=27;
		else
		// MOD END <-- Xanatos --
		adder=81;
	}
	

	//SortItems(SortProc, sortItem + (sortAscending ? 0:100) + adder );
	SortItems(SortProc, sortItem + (sortAscending ? 0:100) + (IsAlternate()?2000:0) + adder ); // NEO: SEa - [SortAltExtension] <-- Xanatos --

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CtrlItem_Struct* item1 = (CtrlItem_Struct*)lParam1;
	CtrlItem_Struct* item2 = (CtrlItem_Struct*)lParam2;

	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	bool showSrc=false;  
	if (lParamSort>=2000)
	{ 
		showSrc=true; 
		lParamSort-=2000; 
	} 
	// NEO: SEa END <-- Xanatos --

	//int dwOrgSort = lParamSort;

	int sortMod = 1;
	if(lParamSort >= 100) 
	{
		sortMod = -1;
		lParamSort -= 100;
	}

	int comp;

	/*if(item1->type == FILE_TYPE && item2->type != FILE_TYPE) 
	{
		if(item1->value == item2->parent->value)
			return -1;
		comp = Compare((CPartFile*)item1->value, (CPartFile*)(item2->parent->value), lParamSort);
	}
	else if(item2->type == FILE_TYPE && item1->type != FILE_TYPE) 
	{
		if(item1->parent->value == item2->value)
			return 1;
		comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)item2->value, lParamSort);
	}
	else if (item1->type == FILE_TYPE) */
	if (item1->type == FILE_TYPE && item2->type == FILE_TYPE) // NEO: SE - [SortExtension] <-- Xanatos --
	{
		if (showSrc) return 0; // NEO: SEa - [SortAltExtension] <-- Xanatos --
		CPartFile* file1 = (CPartFile*)item1->value;
		CPartFile* file2 = (CPartFile*)item2->value;
		comp = Compare(file1, file2, lParamSort);
	}
	//else
	else if(item1->type != FILE_TYPE && item2->type != FILE_TYPE) // NEO: SE - [SortExtension] <-- Xanatos --
	{
		if (item1->parent->value!=item2->parent->value) 
			return 0; // NEO: SEa - [SortAltExtension] <-- Xanatos --
		/*{
			comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)(item2->parent->value), lParamSort);
			return sortMod * comp;
		}*/
		if (item1->type != item2->type)
			return item1->type - item2->type;

		CUpDownClient* client1 = (CUpDownClient*)item1->value;
		CUpDownClient* client2 = (CUpDownClient*)item2->value;
		comp = Compare(client1, client2, lParamSort);
	}
	// NEO: SE - [SortExtension] -- Xanatos -->
	else // compare file and source
	{
		return 0;
	}
	// NEO: SE END <-- Xanatos --
	/*int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (comp == 0 && (dwNextSort = theApp.emuledlg->transferwnd->downloadlistctrl.GetNextSortOrder(dwOrgSort)) != (-1)){
		return SortProc(lParam1, lParam2, dwNextSort);
	}
	else*/
	return sortMod * comp;
}

void CDownloadListCtrl::ClearCompleted(int incat){
	if (incat==-2)
		incat=curTab;

	// Search for completed file(s)
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			//if(file->IsPartFile() == false && (file->CheckShowItemInGivenCat(incat) || incat==-1) ){
			if(file->IsPartFile() == false
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
			 || (file->IsVoodooFile() && !file->HaveMasters()) // remove all voodoo files without masters 
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --	
			 && (file->CheckShowItemInGivenCat(incat) || incat==-1) ){
				if (RemoveFile(file))
					it = m_ListItems.begin();
			}
		}
	}
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
}

void CDownloadListCtrl::ClearCompleted(const CPartFile* pFile)
{
	if (!pFile->IsPartFile())
	{
		for (ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); )
		{
			CtrlItem_Struct* cur_item = it->second;
			it++;
			if (cur_item->type == FILE_TYPE)
			{
				const CPartFile* pCurFile = reinterpret_cast<CPartFile*>(cur_item->value);
				if (pCurFile == pFile)
				{
					RemoveFile(pCurFile);
					return;
				}
			}
		}
	}
}

void CDownloadListCtrl::SetStyle()
{
	if (thePrefs.IsDoubleClickEnabled())
		SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	else
		SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);
}

void CDownloadListCtrl::OnListModified(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);
}

int CDownloadListCtrl::Compare(const CPartFile* file1, const CPartFile* file2, LPARAM lParamSort)
{
	int comp=0;
	switch(lParamSort)
	{
		case 0: //filename asc
			comp=CompareLocaleStringNoCase(file1->GetFileName(),file2->GetFileName());
			break;
		case 1: //size asc
			comp=CompareUnsigned64(file1->GetFileSize(), file2->GetFileSize());
			break;
		case 2: //transferred asc
			comp=CompareUnsigned64(file1->GetTransferred(), file2->GetTransferred());
			break;
		// NEO: MOD - [SessionDL] -- Xanatos -->
		case 20: //transferred session asc
			comp=CompareUnsigned64(file1->GetTransferredSession(), file2->GetTransferredSession());
			break;
		// NEO: MOD END <-- Xanatos --
		case 3: //completed asc
			comp=CompareUnsigned64(file1->GetCompletedSize(), file2->GetCompletedSize());
			break;
		case 4: //speed asc
			// NEO: MOD - [SpeedColumn] -- Xanatos -->
			{
				uint32 datarate1 = file1->GetDatarate();
				uint32 datarate2 = file2->GetDatarate();
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
				datarate1 += file1->GetLanDatarate();
				datarate2 += file2->GetLanDatarate();
#endif //LANCAST // NEO: NLC END 
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				datarate1 += file1->GetVoodooDatarate();
				datarate2 += file2->GetVoodooDatarate();
#endif // VOODOO // NEO: VOODOO END
				comp=CompareUnsigned(datarate1, datarate2);
			}
			// MOD END <-- Xanatos --
			//comp=CompareUnsigned(file1->GetDatarate(), file2->GetDatarate());
			break;
		case 5: //progress asc
			comp = CompareFloat(file1->GetPercentCompleted(), file2->GetPercentCompleted());
			break;
		case 6: //sources asc
			comp=CompareUnsigned(file1->GetSourceCount(), file2->GetSourceCount());
			break;
		case 7: //priority asc
			comp=CompareUnsigned(file1->GetDownPriority(), file2->GetDownPriority());
			break;
		case 8: //Status asc 
			comp=CompareUnsigned(file1->getPartfileStatusRang(),file2->getPartfileStatusRang());
			break;
		case 9: //Remaining Time asc
		{
			//Make ascending sort so we can have the smaller remaining time on the top 
			//instead of unknowns so we can see which files are about to finish better..
			time_t f1 = file1->getTimeRemaining();
			time_t f2 = file2->getTimeRemaining();
			//Same, do nothing.
			if( f1 == f2 ) 
			{
				comp=0;
				break;
			}
			//If descending, put first on top as it is unknown
			//If ascending, put first on bottom as it is unknown
			if( f1 == -1 ) 
			{
				comp=1;
				break;
			}
			//If descending, put second on top as it is unknown
			//If ascending, put second on bottom as it is unknown
			if( f2 == -1 ) 
			{
				comp=-1;
				break;
			}
			//If descending, put first on top as it is bigger.
			//If ascending, put first on bottom as it is bigger.
			comp = CompareUnsigned(f1, f2);
			break;
		}
		case 90: //Remaining SIZE asc
			comp=CompareUnsigned64(file1->GetFileSize()-file1->GetCompletedSize(), file2->GetFileSize()-file2->GetCompletedSize());
			break;
		case 10: //last seen complete asc
			if (file1->lastseencomplete > file2->lastseencomplete)
				comp=1;
			else if(file1->lastseencomplete < file2->lastseencomplete)
				comp=-1;
			else
				comp=0;
			break;
		case 11: //last received Time asc
			if (file1->GetFileDate() > file2->GetFileDate())
				comp=1;
			else if(file1->GetFileDate() < file2->GetFileDate())
				comp=-1;
			else
				comp=0;
			break;
		case 12:
			// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
			if (file1->GetCategory() > file2->GetCategory())
				comp=1;
			else if (file1->GetCategory() < file2->GetCategory())
				comp=-1;
			else
				comp=0;
			// NEO: NXC END <-- Xanatos --
			//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
			//comp=CompareLocaleStringNoCase(	(const_cast<CPartFile*>(file1)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->strTitle:GetResString(IDS_ALL),
			//								(const_cast<CPartFile*>(file2)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->strTitle:GetResString(IDS_ALL) );
			break;
		// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
		case 13:
			comp=(file1->GetMaxSources() > file2->GetMaxSources());
			if(comp == 0)
				comp=(file1->GetValidSourcesCount() > file2->GetValidSourcesCount());
			break;
		// NEO: SRT END <-- Xanatos --
		// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
		case 14:
			if (CPartFile::NextRightFileHasHigherPrio(file2, file1))
				comp=1;
			else if (CPartFile::NextRightFileHasHigherPrio(file1, file2))
				comp=-1;
			else
				comp=0;
			break;
		// NEO: NXC END <-- Xanatos --
		// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
		case 15:
			comp=CompareLocaleStringNoCase(file1->GetPath(),file2->GetPath());
			break;
		// NEO: MTD END <-- Xanatos --
		default:
			comp=0;
	}
	return comp;
 }

int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort)
{
	switch (lParamSort)
	{
	case 0: //name asc
		if (client1->GetUserName() == client2->GetUserName())
			return 0;
		else if (!client1->GetUserName())	// place clients with no usernames at bottom
			return 1;
		else if (!client2->GetUserName())	// place clients with no usernames at bottom
			return -1;
		return CompareLocaleStringNoCase(client1->GetUserName(), client2->GetUserName());

	case 1: //size but we use status asc
		return client1->GetSourceFrom() - client2->GetSourceFrom();

	// MOD - [SessionDL] -- Xanatos -->
	case 2://transferred asc
		return CompareUnsigned(client1->GetTransferredDown(), client2->GetTransferredDown());

	case 20: //transferred session asc
		return  CompareUnsigned64(client1->Credits() ? client1->Credits()->GetDownloadedTotal() : 0, client2->Credits() ? client2->Credits()->GetDownloadedTotal() : 0);

	case 3://completed asc
		return CompareUnsigned(client1->GetTransferredUp(), client2->GetTransferredUp());

	case 30: //completed session asc
		return  CompareUnsigned64(client1->Credits() ? client1->Credits()->GetUploadedTotal() : 0, client2->Credits() ? client2->Credits()->GetUploadedTotal() : 0);
	// MOD END <-- Xanatos --

	case 4: //speed asc
		return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());

	case 5: //progress asc
		return CompareUnsigned(client1->GetAvailablePartCount(), client2->GetAvailablePartCount());

	case 6:
		/*if (client1->GetClientSoft() == client2->GetClientSoft())
			return CompareUnsigned(client2->GetVersion(), client1->GetVersion());
		return CompareUnsigned(client1->GetClientSoft(), client2->GetClientSoft());*/
		return CompareLocaleStringNoCase(client1->DbgGetFullClientSoftVer(), client2->DbgGetFullClientSoftVer()); // NEO: MIDI - [ModIDInfo] <-- Xanatos --
	case 7: //qr asc
		if (client1->GetDownloadState() == DS_DOWNLOADING){
			if (client2->GetDownloadState() == DS_DOWNLOADING)
				return 0;
			else
				return -1;
		}
		else if (client2->GetDownloadState() == DS_DOWNLOADING)
			return 1;
		/*if (client1->GetRemoteQueueRank() == 0 
			&& client1->GetDownloadState() == DS_ONQUEUE && client1->IsRemoteQueueFull() == true)
			return 1;
		if (client2->GetRemoteQueueRank() == 0 
			&& client2->GetDownloadState() == DS_ONQUEUE && client2->IsRemoteQueueFull() == true)
			return -1;*/
		// NEO: FIX - [SourceCount] -- Xanatos -->
		if (client1->GetRemoteQueueRank() == 0 
			&& client1->GetDownloadState() == DS_REMOTEQUEUEFULL)
			return 1;
		if (client2->GetRemoteQueueRank() == 0 
			&& client2->GetDownloadState() == DS_REMOTEQUEUEFULL)
			return -1;
		// NEO: FIX END <-- Xanatos --
		if (client1->GetRemoteQueueRank() == 0)
			return 1;
		if (client2->GetRemoteQueueRank() == 0)
			return -1;
		return CompareUnsigned(client1->GetRemoteQueueRank(), client2->GetRemoteQueueRank());

	case 8:
		// NEO: FIX - [SourceCount] -- Xanatos --
		/*if (client1->GetDownloadState() == client2->GetDownloadState()){
			if (client1->IsRemoteQueueFull() && client2->IsRemoteQueueFull())
				return 0;
			else if (client1->IsRemoteQueueFull())
				return 1;
			else if (client2->IsRemoteQueueFull())
				return -1;
		}*/
		return client1->GetDownloadState() - client2->GetDownloadState();

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	case 12:
		if(client1->GetCountryName(true) && client2->GetCountryName(true))
			return CompareLocaleStringNoCase(client1->GetCountryName(true), client2->GetCountryName(true));
		else if(client1->GetCountryName(true))
			return 1;
		else
			return -1;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	default:
		return 0;
	}
}

// NEO: NTS - [NeoTreeStyle] -- Xanatos -->
void CDownloadListCtrl::OnClickDownloadlist(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p);

	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (p.x<14) 
		ExpandCollapseItem(pNMIA->iItem,2);
}
// NEO: NTS END <-- Xanatos --

void CDownloadListCtrl::OnNMDblclkDownloadlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetSelectionMark();
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content && content->value)
		{
			if (content->type == FILE_TYPE)
			{
				if (!thePrefs.IsDoubleClickEnabled())
				{
					CPoint pt;
					::GetCursorPos(&pt);
					ScreenToClient(&pt);
					LVHITTESTINFO hit;
					hit.pt = pt;
					if (HitTest(&hit) >= 0 && (hit.flags & LVHT_ONITEM))
					{
						LVHITTESTINFO subhit;
						subhit.pt = pt;
						if (SubItemHitTest(&subhit) >= 0 && subhit.iSubItem == 0)
						{
							CPartFile* file = (CPartFile*)content->value;
							if (thePrefs.ShowRatingIndicator() 
								&& (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()) 
								&& pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx 
								&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+RATING_ICON_WIDTH)
								ShowFileDialog(IDD_COMMENTLST);
							else if (thePrefs.GetPreviewOnIconDblClk()
									 && pt.x >= FILE_ITEM_MARGIN_X 
									 && pt.x < FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx) {
								if (file->IsReadyForPreview())
									file->PreviewFile();
								else
									MessageBeep(MB_OK);
							}
							else
								ShowFileDialog(0);
						}
					}
				}
			}
			else
			{
				ShowClientDialog((CUpDownClient*)content->value);
			}
		}
	}
	
	*pResult = 0;
}

void CDownloadListCtrl::CreateMenues()
{
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
	// NEO: MCM - [ManualClientManagement] -- Xanatos -->
	if (m_A4AFMenu)
		VERIFY( m_A4AFMenu.DestroyMenu() );
	// NEO: MCM END <-- Xanatos --
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	if (m_PWProtMenu) 
		VERIFY( m_PWProtMenu.DestroyMenu() ); 
	// NEO: PP END <-- Xanatos --
	// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
	if (m_CollectMenu) 
		VERIFY( m_CollectMenu.DestroyMenu() ); 
	// NEO: MSR END <-- Xanatos --
	if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	// NEO: MSD - [ManualSourcesDrop] -- Xanatos -->
	if (m_DropMenu) 
		VERIFY( m_DropMenu.DestroyMenu() );  
	// NEO: MSD END <-- Xanatos --
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	if (m_TempDirMenu) 
		VERIFY( m_TempDirMenu.DestroyMenu() ); 
	// NEO: MTD END <-- Xanatos --
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );

	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	m_TempDirMenu.CreateMenu();
	m_TempDirMenu.AddMenuTitle(NULL, true);
	m_TempDirMenu.AppendMenu(MF_STRING,MP_MTD_UNLOAD,GetResString(IDS_X_MTD_UNLOAD), _T("MTD_UNLOAD"));
	m_TempDirMenu.AppendMenu(MF_SEPARATOR);
	// NEO: MTD END <-- Xanatos --

	m_FileMenu.CreatePopupMenu();
	m_FileMenu.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE), true);

	// Add file commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	m_FileMenu.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));
	m_FileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	m_FileMenu.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), _T("DELETE"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING,MP_FORCE, GetResString(IDS_X_DL_FORCE), _T("FORCE")); // NEO: OCF - [OnlyCompleetFiles] <-- Xanatos --
	m_FileMenu.AppendMenu(MF_STRING,MP_STANDBY, GetResString(IDS_X_DL_STANDBY), _T("STANDBY")); // NEO: SD - [StandByDL] <-- Xanatos --
	m_FileMenu.AppendMenu(MF_STRING,MP_SUSPEND, GetResString(IDS_X_DL_SUSPEND), _T("SUSPEND")); // NEO: SC - [SuspendCollecting] <-- Xanatos --

	// Add 'Download Priority' sub menu
	//
	m_PrioMenu.CreateMenu();
	m_PrioMenu.AddMenuTitle(NULL, true);
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"), _T("FILEPRIORITY"));

	// NEO: MCM - [ManualClientManagement] -- Xanatos -->
	m_A4AFMenu.CreateMenu();
	m_A4AFMenu.AddMenuTitle(NULL, true); 
	m_A4AFMenu.AppendMenu(MF_STRING, MP_FORCEA4AF, GetResString(IDS_X_A4AF_FORCEALL), _T("ADVA4AFALL")); // NEO: NXC - [NeoExtendedCategories]
	m_A4AFMenu.AppendMenu(MF_SEPARATOR); 
	m_A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_TO_A4AF,GetResString(IDS_X_SWAP_TO_A4AF), _T("ADVA4AFTO"));
	m_A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_FROM_A4AF,GetResString(IDS_X_SWAP_FROM_A4AF), _T("ADVA4AFFROM"));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_A4AFMenu.m_hMenu, GetResString(IDS_X_A4AF_MENU), _T("ADVA4AF"));
	// NEO: MCM END <-- Xanatos --

	m_FileMenu.AppendMenu(MF_SEPARATOR); // NEO: MOD <-- Xanatos --
	m_FileMenu.AppendMenu(MF_STRING, MP_OPEN, GetResString(IDS_DL_OPEN), _T("OPENFILE"));
	if (thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio())
    	m_FileMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS));
	m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
	m_FileMenu.AppendMenu(MF_STRING, MP_METINFO, GetResString(IDS_DL_INFO), _T("FILEINFO"));
	m_FileMenu.AppendMenu(MF_STRING, MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), _T("FILECOMMENTS"));
	m_FileMenu.AppendMenu(MF_STRING,MP_TWEAKS, GetResString(IDS_X_FILE_TWEAKS), _T("FILECONFIG")); // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
	m_CollectMenu.CreateMenu();
	m_CollectMenu.AddMenuTitle(NULL, true);
	m_CollectMenu.AppendMenu(MF_STRING,MP_COLLECT_ALL_SOURCES,GetResString(IDS_X_COLLECT_ALL_SOURCES), _T("SRCALL"));
	m_CollectMenu.AppendMenu(MF_SEPARATOR);
	m_CollectMenu.AppendMenu(MF_STRING,MP_COLLECT_XS_SOURCES,GetResString(IDS_X_COLLECT_XS_SOURCES), _T("XSSRC"));
	m_CollectMenu.AppendMenu(MF_STRING,MP_COLLECT_SVR_SOURCES,GetResString(IDS_X_COLLECT_SVR_SOURCES), _T("SVRSRC"));
	m_CollectMenu.AppendMenu(MF_STRING,MP_COLLECT_KAD_SOURCES,GetResString(IDS_X_COLLECT_KAD_SOURCES), _T("KADSRC"));
	m_CollectMenu.AppendMenu(MF_STRING,MP_COLLECT_UDP_SOURCES,GetResString(IDS_X_COLLECT_UDP_SOURCES), _T("UDPSRC"));
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
	m_CollectMenu.AppendMenu(MF_SEPARATOR);
	m_CollectMenu.AppendMenu(MF_STRING,MP_COLLECT_VOODOO_SOURCES,GetResString(IDS_X_COLLECT_VOODOO_SOURCES), _T("SRCVOODOO"));
#endif // VOODOO // NEO: VOODOOx END
	m_CollectMenu.AppendMenu(MF_SEPARATOR);
	m_CollectMenu.AppendMenu(MF_STRING,MP_AHL_INCREASE, GetResString(IDS_X_AHL_INCREASE), _T("AHLINCREASE"));
	m_CollectMenu.AppendMenu(MF_STRING,MP_AHL_DECREASE, GetResString(IDS_X_AHL_DECREASE), _T("AHLDECREASE"));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectMenu.m_hMenu, GetResString(IDS_X_SOURCE_COLLECTING), _T("COLLECTING")); 
	// NEO: MSR END <-- Xanatos --

	// NEO: MOD - [SourcesMenu] -- Xanatos -->
	m_SourcesMenu.CreateMenu();
	m_SourcesMenu.AddMenuTitle(NULL, true);
	m_SourcesMenu.AppendMenu(MF_STRING,MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY), _T("MANADDSRC"));
	// NEO: MOD END <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	m_SourcesMenu.AppendMenu(MF_SEPARATOR);
	m_SourcesMenu.AppendMenu(MF_STRING,MP_LOADSOURCE, GetResString(IDS_X_LOADSRCMANUALLY), _T("LOADSRC"));
	m_SourcesMenu.AppendMenu(MF_STRING,MP_SAVESOURCE, GetResString(IDS_X_SAVESRCMANUALLY), _T("SAVESRC"));
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
 #ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
	m_SourcesMenu.AppendMenu(MF_SEPARATOR);
	m_SourcesMenu.AppendMenu(MF_STRING,MP_FINDSOURCES, GetResString(IDS_X_FINDSOURCES), _T("FINDSOURCES"));
 #endif // NEO_CD // NEO: SFL END <-- Xanatos --
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_X_SOURCE_STORAGE), _T("STORAGE")); // NEO: MOD - [SourcesMenu] <-- Xanatos --
	// Add (extended user mode) 'Source Handling' sub menu
	//
	//if (thePrefs.IsExtControlsEnabled()) {
	//	m_SourcesMenu.CreateMenu();
	//	m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY));
		//m_SourcesMenu.AppendMenu(MF_STRING, MP_SETSOURCELIMIT, GetResString(IDS_SETPFSLIMIT)); // NEO: SRT - [SourceRequestTweaks] -- Xanatos --
	//	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_A4AF));
	//}

	// NEO: MSD - [ManualSourcesDrop] -- Xanatos -->
	m_DropMenu.CreateMenu();
	m_DropMenu.AddMenuTitle(NULL, true);
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_NNP,GetResString(IDS_X_DROP_NNP), _T("DROPNNP"));
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_FULLQ,GetResString(IDS_X_DROP_FULLQ), _T("DROPFULLQ"));
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_HIGHQ,GetResString(IDS_X_DROP_HIGHQ), _T("DROPHIGHQ"));
	m_DropMenu.AppendMenu(MF_SEPARATOR);
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_WAITINGRETRY,GetResString(IDS_X_DROP_WAITINGRETRY), _T("DROPWAITINGRETRY")); // NEO: TCR - [TCPConnectionRetry]
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_CACHED,GetResString(IDS_X_DROP_CACHED), _T("DROPCACHED")); // NEO: XSC - [ExtremeSourceCache]
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_LOADED,GetResString(IDS_X_DROP_LOADED), _T("DROPLOADED"));
#endif // NEO_SS // NEO: NSS END
	m_DropMenu.AppendMenu(MF_SEPARATOR);
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_TOMANY,GetResString(IDS_X_DROP_TOMANY), _T("DROPTOMANY"));
	m_DropMenu.AppendMenu(MF_STRING,MP_DROP_LOW2LOW,GetResString(IDS_X_DROP_LOW2LOW), _T("DROPLOW2LOW"));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_DropMenu.m_hMenu, GetResString(IDS_X_DROP), _T("DROP")); 
	// NEO: MSD END <-- Xanatos --

	m_FileMenu.AppendMenu(MF_SEPARATOR);

	m_FileMenu.AppendMenu(MF_STRING,MP_IMPORT, GetResString(IDS_X_IMPORT_PARTS), _T("PARTIMPORT"));  // NEO: PIX - [PartImportExport] <-- Xanatos --
	// NEO: MOD - [SpaceAllocate] -- Xanatos -->
	if(thePrefs.GetAllocCompleteMode() == false) // only when full allocation is disabled
		m_FileMenu.AppendMenu(MF_STRING,MP_PREALOCATE, GetResString(IDS_X_PREALOCATE), _T("PREALOCATE"));
	// NEO: MOD END <-- Xanatos --
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_TempDirMenu.m_hMenu, GetResString(IDS_X_TEMPDIRMENUTITLE), _T("TEMPDIRS")); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	m_FileMenu.AppendMenu(MF_SEPARATOR);// NEO: MOD <-- Xanatos --
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	m_PWProtMenu.CreateMenu();
	m_PWProtMenu.AddMenuTitle(NULL, true);
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_SHOW,GetResString(IDS_X_PWPROT_SHOW), _T("PWPROT_SHOW"));
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_HIDE,GetResString(IDS_X_PWPROT_HIDE), _T("PWPROT_HIDE"));
	m_PWProtMenu.AppendMenu(MF_SEPARATOR);
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_SET,GetResString(IDS_X_PWPROT_SET), _T("PWPROT_SET"));
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_CHANGE,GetResString(IDS_X_PWPROT_CHANGE), _T("PWPROT_CHANGE"));
	m_PWProtMenu.AppendMenu(MF_STRING,MP_PWPROT_UNSET,GetResString(IDS_X_PWPROT_UNSET), _T("PWPROT_UNSET"));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PWProtMenu.m_hMenu, GetResString(IDS_X_PWPROT_MENU), _T("PWPROT"));
	// NEO: PP END <-- Xanatos --
	m_FileMenu.AppendMenu(MF_STRING, MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR), _T("CLEARCOMPLETE")); // NEO: MOD <-- Xanatos --

	// Add 'Copy & Paste' commands
	//
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_FileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK"));
	m_FileMenu.AppendMenu(MF_STRING, MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD), _T("PASTELINK"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Search commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_FIND, GetResString(IDS_FIND), _T("Search"));
	m_FileMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED), _T("KadFileSearch"));
	// Web-services and categories will be added on-the-fly..
}

CString CDownloadListCtrl::getTextList()
{
	CString out;

	for (ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			const CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);

			CString temp;
			temp.Format(_T("\n%s\t [%.1f%%] %i/%i - %s"),
						file->GetFileName(),
						file->GetPercentCompleted(),
						file->GetTransferringSrcCount(),
						file->GetSourceCount(), 
						file->getPartfileStatus());

			out += temp;
		}
	}

	return out;
}

//int CDownloadListCtrl::GetFilesCountInCurCat()
//{
//	int iCount = 0;
//	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
//	{
//		CtrlItem_Struct* cur_item = it->second;
//		if (cur_item->type == FILE_TYPE)
//		{
//			CPartFile* file = (CPartFile*)cur_item->value;
//			if (file->CheckShowItemInGivenCat(curTab))
//				iCount++;
//		}
//	}
//	return iCount;
//}

void CDownloadListCtrl::ShowFilesCount()
{
	// NEO: MOD - [Percentage] -- Xanatos -->
	const bool DwlPercentage = thePrefs.GetUseDwlPercentage() != FALSE;
	int iCount = 0;
	float Percentage = 0.0f;
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			CPartFile* file = (CPartFile*)cur_item->value;
			if (file->CheckShowItemInGivenCat(curTab)){
				iCount++;
				if(DwlPercentage)
					Percentage += file->GetPercentCompleted();
			}
		}
	}
	if(DwlPercentage && iCount)
		Percentage /= iCount;
	theApp.emuledlg->transferwnd->UpdateFilesCount(iCount,Percentage);
	// NEO: MOD END <-- Xanatos --
	//theApp.emuledlg->transferwnd->UpdateFilesCount(GetFilesCountInCurCat());
}

void CDownloadListCtrl::ShowSelectedFileDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt = point; 
    ScreenToClient(&pt); 
    int it = HitTest(pt);
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly! 

	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(GetSelectionMark());
	if (content->type == FILE_TYPE)
	{
		CPartFile* file = (CPartFile*)content->value;
		if (thePrefs.ShowRatingIndicator() 
			&& (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()) 
			&& pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx 
			&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+RATING_ICON_WIDTH)
			ShowFileDialog(IDD_COMMENTLST);
		else
			ShowFileDialog(0);
	}
	else
	{
		ShowClientDialog((CUpDownClient*)content->value);
	}
}

int CDownloadListCtrl::GetCompleteDownloads(int cat, int& total)
{
	total = 0;
	int count = 0;
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			/*const*/ CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (file->CheckShowItemInGivenCat(cat) || cat==-1)
			{
				total++;
				if (file->GetStatus() == PS_COMPLETE)
					count++;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
				else if (file->IsVoodooFile() && !file->HaveMasters()) // remove all voodoo files without masters 
					count++;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --	
			}
		}
	}
	return count;
}

void CDownloadListCtrl::UpdateCurrentCategoryView(){
	ChangeCategory(curTab);
}

void CDownloadListCtrl::UpdateCurrentCategoryView(CPartFile* thisfile) {

	ListItems::const_iterator it = m_ListItems.find(thisfile);
	if (it != m_ListItems.end()) {
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			//if (!file->CheckShowItemInGivenCat(curTab))
			if (!file->CheckShowItemInGivenCat(curTab) || file->IsPWProtHidden()) // NEO: PP - [PasswordProtection] <-- Xanatos --
				HideFile(file);
			else
				ShowFile(file);
		}
	}

}

void CDownloadListCtrl::ChangeCategory(int newsel){

	SetRedraw(FALSE);

	// remove all displayed files with a different cat and show the correct ones
	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			//if (!file->CheckShowItemInGivenCat(newsel))
			if (!file->CheckShowItemInGivenCat(newsel) || file->IsPWProtHidden()) // NEO: PP - [PasswordProtection] <-- Xanatos --
				HideFile(file);
			else
				ShowFile(file);
		}
	}

	SetRedraw(TRUE);
	curTab=newsel;
	ShowFilesCount();
	// NEO: SE - [SortExtension] -- Xanatos -->
	AlterSortArrow();

	uint8 adder=0;
	//if (GetSortItem()!=9 || !m_bRemainSort )
	if ((GetSortItem()!=9 && GetSortItem()!=2 && GetSortItem()!=3) || !m_bRemainSort ) // MOD - [SessionDL]
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending()?arrowDoubleUp : arrowDoubleDown);
		// MOD - [SessionDL]
		if(GetSortItem()==2)
			adder=18;
		else if(GetSortItem()==3 && IsAlternate())
			adder=27;
		else
		// MOD END
		adder=81;
	}

	if (NeoPrefs.DisableAutoSort() == 2)
		SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100) + adder);
	// NEO: SE END <-- Xanatos --
}

void CDownloadListCtrl::HideFile(CPartFile* tohide)
{
	HideSources(tohide);

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(tohide);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result != -1){
			DeleteItem(result);
			return;
		}
	}
}

void CDownloadListCtrl::ShowFile(CPartFile* toshow){
	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toshow);
	ListItems::const_iterator it = rangeIt.first;
	if(it != rangeIt.second){
		CtrlItem_Struct* updateItem  = it->second;

		// Check if entry is already in the List
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result == -1)
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)updateItem);
	}
}

void CDownloadListCtrl::GetDisplayedFiles(CArray<CPartFile*,CPartFile*> *list){
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			list->Add(file);
		}
	}	
}

void CDownloadListCtrl::MoveCompletedfilesCat(uint8 from, uint8 to)
{
	int mycat;

	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator.
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (!file->IsPartFile()){
				mycat=file->GetCategory();
				if ( mycat>=min(from,to) && mycat<=max(from,to)) {
					if (mycat==from) 
						file->SetCategory(to); 
					else
						if (from<to)
							file->SetCategory(mycat-1);
						else
							file->SetCategory(mycat+1);
				}
			}
		}
	}
}

void CDownloadListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	/*TRACE("CDownloadListCtrl::OnGetDispInfo iItem=%d iSubItem=%d", pDispInfo->item.iItem, pDispInfo->item.iSubItem);
	if (pDispInfo->item.mask & LVIF_TEXT)
		TRACE(" LVIF_TEXT");
	if (pDispInfo->item.mask & LVIF_IMAGE)
		TRACE(" LVIF_IMAGE");
	if (pDispInfo->item.mask & LVIF_STATE)
		TRACE(" LVIF_STATE");
	TRACE("\n");*/

	// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
	// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
	// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
	// it needs to know the contents of the primary item.
	//
	// But, the listview control sends this notification all the time, even if we do not search for an item. At least
	// this notification is only sent for the visible items and not for all items in the list. Though, because this
	// function is invoked *very* often, no *NOT* put any time consuming code here in.

    if (pDispInfo->item.mask & LVIF_TEXT){
        const CtrlItem_Struct* pItem = reinterpret_cast<CtrlItem_Struct*>(pDispInfo->item.lParam);
        if (pItem != NULL && pItem->value != NULL){
			if (pItem->type == FILE_TYPE){
				switch (pDispInfo->item.iSubItem){
					case 0:
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CPartFile*)pItem->value)->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else if (pItem->type == UNAVAILABLE_SOURCE || pItem->type == AVAILABLE_SOURCE){
				switch (pDispInfo->item.iSubItem){
					case 0:
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
						if (((CUpDownClient*)pItem->value)->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CUpDownClient*)pItem->value)->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else
				ASSERT(0);
        }
    }
    *pResult = 0;
}

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
void CDownloadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don't show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
				pGetInfoTip->pszText[0] = _T('\0');
			return;
		}

		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(pGetInfoTip->iItem);
		if (content && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString info;

			// build info text and display it
			if (content->type == 1) // for downloading files
			{
				const CPartFile* partfile = (CPartFile*)content->value;
				info = partfile->GetInfoSummary();
			}
			else if (content->type == 3 || content->type == 2) // for sources
			{
				const CUpDownClient* client = (CUpDownClient*)content->value;
				if (client->IsEd2kClient())
				{
					in_addr server;
					server.S_un.S_addr = client->GetServerIP();
					info.Format(GetResString(IDS_USERINFO)
								+ GetResString(IDS_SERVER) + _T(": %s:%u\n\n")
								+ GetResString(IDS_NEXT_REASK) + _T(": %s"),
								client->GetUserName() ? client->GetUserName() : _T("?"),
								ipstr(server), client->GetServerPort(),
								CastSecondsToHM(client->GetTimeUntilReask(client->GetRequestFile()) / 1000));
					if (thePrefs.IsExtControlsEnabled())
						info.AppendFormat(_T(" (%s)"), CastSecondsToHM(client->GetTimeUntilReask(content->owner) / 1000));
					info += _T('\n');
					info.AppendFormat(GetResString(IDS_SOURCEINFO), client->GetAskedCountDown(), client->GetAvailablePartCount());
					info += _T('\n');

					if (content->type == 2)
					{
						info += GetResString(IDS_CLIENTSOURCENAME) + (!client->GetClientFilename().IsEmpty() ? client->GetClientFilename() : _T("-"));
						/*if (!client->GetFileComment().IsEmpty())
							info += _T('\n') + GetResString(IDS_CMT_READ) + _T(' ') + client->GetFileComment();
						if (client->GetFileRating())
							info += _T('\n') + GetResString(IDS_QL_RATING) + _T(':') + GetRateString(client->GetFileRating());*/
						// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
						if(const CClientFileStatus* status = client->GetFileStatus(client->GetRequestFile()))
						{
							if (!status->GetFileComment().IsEmpty())
								info += _T('\n') + GetResString(IDS_CMT_READ) + _T(' ') + status->GetFileComment();
							if (status->GetFileRating())
								info += _T('\n') + GetResString(IDS_QL_RATING) + _T(':') + GetRateString(status->GetFileRating());
						}
						// NEO: SCFS END <-- Xanatos --
					}
					else
					{	// client asked twice
						info += GetResString(IDS_ASKEDFAF);
                        if (client->GetRequestFile() && client->GetRequestFile()->GetFileName())
                            info.AppendFormat(_T(": %s"), client->GetRequestFile()->GetFileName());
					}

                    if (thePrefs.IsExtControlsEnabled() && !client->m_OtherRequests_list.IsEmpty())
					{
						CSimpleArray<const CString*> apstrFileNames;
						POSITION pos = client->m_OtherRequests_list.GetHeadPosition();
						while (pos)
							apstrFileNames.Add(&client->m_OtherRequests_list.GetNext(pos)->GetFileName());
						Sort(apstrFileNames);
						if (content->type == 2)
							info += _T('\n');
						info += _T('\n');
						info += GetResString(IDS_A4AF_FILES);
						info += _T(':');
						for (int i = 0; i < apstrFileNames.GetSize(); i++)
						{
							const CString* pstrFileName = apstrFileNames[i];
							if (info.GetLength() + (i > 0 ? 2 : 0) + pstrFileName->GetLength() >= pGetInfoTip->cchTextMax) {
								static const TCHAR szEllipses[] = _T("\n:...");
								if (info.GetLength() + (int)ARRSIZE(szEllipses) - 1 < pGetInfoTip->cchTextMax)
									info += szEllipses;
								break;
							}
							if (i > 0)
								info += _T("\n:");
							info += *pstrFileName;
						}
                    }
				}
				else
				{
					info.Format(_T("URL: %s\nAvailable parts: %u"), client->GetUserName(), client->GetAvailablePartCount());
				}
			}

			info += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

//void CDownloadListCtrl::ShowFileDialog(UINT uInvokePage)
void CDownloadListCtrl::ShowFileDialog(UINT uInvokePage, BOOL Preferences) // NEO: FCFG - [FileConfiguration] <-- Xanatos --
{
	CSimpleArray<CPartFile*> aFiles;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iItem = GetNextSelectedItem(pos);
		if (iItem != -1)
		{
			const CtrlItem_Struct* pCtrlItem = (CtrlItem_Struct*)GetItemData(iItem);
			if (pCtrlItem->type == FILE_TYPE)
				aFiles.Add((CPartFile*)pCtrlItem->value);
		}
	}

	if (aFiles.GetSize() > 0)
	{
		CDownloadListListCtrlItemWalk::SetItemType(FILE_TYPE);
		// NEO: FCFG - [FileConfiguration] -- Xanatos -->
		if(Preferences)
		{
			CSimpleArray<CKnownFile*> paFiles;
			for (int i = 0; i < aFiles.GetSize(); i++)
				paFiles.Add(aFiles[i]);
			// NEO: MLD - [ModelesDialogs] 
			CFilePreferencesDialog* dlg = new CFilePreferencesDialog(&paFiles, uInvokePage, this);
			dlg->OpenDialog(); 
			// NEO: MLD END
			//CFilePreferencesDialog dialog(&paFiles, uInvokePage, this);
			//dialog.DoModal();
		}
		else
		// NEO: FCFG END <-- Xanatos --
		{
			// NEO: NFDD - [NewFileDetailDialog] -- Xanatos -->
			CSimpleArray<CAbstractFile*> paFiles;
			for (int i = 0; i < aFiles.GetSize(); i++)
				paFiles.Add(aFiles[i]);
			//CFileDetailDialog dialog(&paFiles, uInvokePage, this);
			// NEO: NFDD END <-- Xanatos --
			//CFileDetailDialog dialog(&aFiles, uInvokePage, this);
			//dialog.DoModal();
			// NEO: MLD - [ModelesDialogs] -- Xanatos -->
			CFileDetailDialog* dlg = new CFileDetailDialog(&paFiles, uInvokePage, this);
			dlg->OpenDialog(); 
			// NEO: MLD END <-- Xanatos --
		}
	}
}

CDownloadListListCtrlItemWalk::CDownloadListListCtrlItemWalk(CDownloadListCtrl* pListCtrl)
	: CListCtrlItemWalk(pListCtrl)
{
	m_pDownloadListCtrl = pListCtrl;
	m_eItemType = (ItemType)-1;
}

CObject* CDownloadListListCtrlItemWalk::GetPrevSelectableItem()
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	ASSERT( m_eItemType != (ItemType)-1 );

	int iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pDownloadListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pDownloadListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem-1 >= 0)
			{
				iItem--;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE))
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)ctrl_item->value);
				}
			}
		}
	}
	return NULL;
}

CObject* CDownloadListListCtrlItemWalk::GetNextSelectableItem()
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	ASSERT( m_eItemType != (ItemType)-1 );

	int iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pDownloadListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pDownloadListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem+1 < iItemCount)
			{
				iItem++;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE))
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)ctrl_item->value);
				}
			}
		}
	}
	return NULL;
}

void CDownloadListCtrl::ShowClientDialog(CUpDownClient* pClient)
{
	CDownloadListListCtrlItemWalk::SetItemType(AVAILABLE_SOURCE); // just set to something !=FILE_TYPE
	// NEO: MLD - [ModelesDialogs] -- Xanatos -->
	CClientDetailDialog* dlg = new CClientDetailDialog(pClient, this);
	dlg->OpenDialog(); 
	// NEO: MLD END <-- Xanatos --
	//CClientDetailDialog dialog(pClient, this);
	//dialog.DoModal();
}
