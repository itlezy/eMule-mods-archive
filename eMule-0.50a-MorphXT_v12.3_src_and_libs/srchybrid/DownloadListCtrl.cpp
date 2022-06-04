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
#include "TransferDlg.h"
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
#include "ToolbarWnd.h"
#include "MassRename.h" //SLAHAM: ADDED MassRename DownloadList
#include "log.h" //MassRename DownloadList
#include "SR13-ImportParts.h"//MORPH - Added by SiRoB, Import Parts [SR13]
#include "Ntservice.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CDownloadListCtrl

#define DLC_BARUPDATE 512

#define RATING_ICON_WIDTH	16


IMPLEMENT_DYNAMIC(CtrlItem_Struct, CObject)

IMPLEMENT_DYNAMIC(CDownloadListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnClick)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnLvnItemActivate)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnListModified)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNmDblClk)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CDownloadListCtrl::CDownloadListCtrl()
	: CDownloadListListCtrlItemWalk(this)
{
	m_pFontBold = NULL;
	//MORPH START leuk_he:run as ntservice v1..
	/*
	m_tooltip = new CToolTipCtrlX;
	*/
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
		m_tooltip = new CToolTipCtrlX;
	else
		m_tooltip = NULL;
	//MORPH END leuk_he:run as ntservice v1..
	SetGeneralPurposeFind(true);
	SetSkinKey(L"DownloadsLv");
	m_dwLastAvailableCommandsCheck = 0;
	m_availableCommandsDirty = true;
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	if (m_PreviewMenu)
		VERIFY( m_PreviewMenu.DestroyMenu() );
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
    if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	//MORPH START - Added by AndCycle, showSharePermissions
	if (m_PermMenu)
		VERIFY( m_PermMenu.DestroyMenu() );	// xMule_MOD: showSharePermissions
	//MORPH END   - Added by AndCycle, showSharePermissions
	//MORPH START - Added by SiRoB, Advanced A4AF derivated from Khaos
	if (m_A4AFMenuFlag)
		VERIFY( m_A4AFMenuFlag.DestroyMenu() );
	//MORPH END   - Added by SiRoB, Advanced A4AF derivated from Khaos
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );
	
	while (m_ListItems.empty() == false) {
		delete m_ListItems.begin()->second; // second = CtrlItem_Struct*
		m_ListItems.erase(m_ListItems.begin());
	}
	if (!RunningAsService()) // MORPH leuk_he:run as ntservice v1.. (worksaround for MFC as a service) 
		delete m_tooltip;
}

void CDownloadListCtrl::Init()
{
	SetPrefsKey(_T("DownloadListCtrl"));
	SetStyle();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );

	if (!theApp.IsRunningAsService()) { // MORPH leuk_he:run as ntservice v1.. (worksaround for MFC as a service) 
		CToolTipCtrl* tooltip = GetToolTips();
		if (tooltip){
		m_tooltip->SetFileIconToolTip(true);
			m_tooltip->SubclassWindow(*tooltip);
			tooltip->ModifyStyle(0, TTS_NOPREFIX);
			tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
			tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
		}
	}

	InsertColumn(0, GetResString(IDS_DL_FILENAME),		LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_DL_SIZE),			LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_DL_TRANSF),		LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH,		-1, true);
	InsertColumn(3, GetResString(IDS_DL_TRANSFCOMPL),	LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_DL_SPEED),			LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(5, GetResString(IDS_DL_PROGRESS),		LVCFMT_LEFT,  DFLT_PARTSTATUS_COL_WIDTH);
	InsertColumn(6, GetResString(IDS_DL_SOURCES),		LVCFMT_RIGHT,  60);
	InsertColumn(7, GetResString(IDS_PRIORITY),			LVCFMT_LEFT,  DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(8, GetResString(IDS_STATUS),			LVCFMT_LEFT,   70);
	// khaos::accuratetimerem+
	/*
	InsertColumn(9, GetResString(IDS_DL_REMAINS),		LVCFMT_LEFT,  110);
	*/
	InsertColumn(9,GetResString(IDS_DL_REMAINS),		LVCFMT_LEFT, 100);
	// khaos::accuratetimerem-

	CString lsctitle = GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(_T(':'));
	InsertColumn(10, lsctitle,							LVCFMT_LEFT,  150,						-1, true);
	lsctitle = GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(_T(':'));
	InsertColumn(11, lsctitle,							LVCFMT_LEFT,  120,						-1, true);
	// khaos::categorymod+ Two new ResStrings, too.
	/*
	InsertColumn(12, GetResString(IDS_CAT),				LVCFMT_LEFT,  100,						-1, true);
	InsertColumn(13, GetResString(IDS_ADDEDON),			LVCFMT_LEFT,  120);
	*/
	InsertColumn(12, GetResString(IDS_ADDEDON),			LVCFMT_LEFT,  120);
	InsertColumn(13, GetResString(IDS_CAT_COLCATEGORY),LVCFMT_LEFT,60);
	InsertColumn(14, GetResString(IDS_CAT_COLORDER),LVCFMT_LEFT,60);
	// khaos::categorymod-
	// khaos::accuratetimerem+
	InsertColumn(15, GetResString(IDS_REMAININGSIZE), LVCFMT_LEFT, 80);
	// khaos::accuratetimerem-
	//MORPH START - Added by SiRoB, IP2Country
	InsertColumn(16, GetResString(IDS_COUNTRY) ,LVCFMT_LEFT, 100);
	//MORPH END   - Added by SiRoB, IP2Country

	SetAllIcons();
	Localize();
	LoadSettings();
	curTab=0;

	//MORPH - Removed by SiRoB, Allways creat the font
	/*
	if (thePrefs.GetShowActiveDownloadsBold())
	{
	*/
		if (thePrefs.GetUseSystemFontForMainControls())
		{
			CFont *pFont = GetFont();
			LOGFONT lfFont = {0};
			pFont->GetLogFont(&lfFont);
			lfFont.lfWeight = FW_BOLD;
			m_fontBold.CreateFontIndirect(&lfFont);
			m_pFontBold = &m_fontBold;
		}
		else
			m_pFontBold = &theApp.m_fontDefaultBold;
	//MORPH - Removed by SiRoB, Allways creat the font
	/*
	}
	*/
	//MORPH START - Added by SiRoB, Draw Client Percentage
	//m_fontBoldSmaller.CreateFont(12,0,0,1,FW_BOLD,0,0,0,0,3,2,1,34,_T("MS Serif"));
	LOGFONT lfSmallerFont = {0};
	m_pFontBold->GetLogFont(&lfSmallerFont);
	//lfSmallerFont.lfWeight = FW_BOLD;
	lfSmallerFont.lfHeight = 11;
	m_fontBoldSmaller.CreateFontIndirect(&lfSmallerFont);
	//MORPH END   - Added by SiRoB, Draw Client Percentage
	//MORPH START - Draw Display Chunk Detail
	lfSmallerFont.lfWeight = FW_NORMAL;
	m_fontSmaller.CreateFontIndirect(&lfSmallerFont);
	//MORPH END   - Draw Display Chunk Detail

	// Barry - Use preferred sort order from preferences
	//MORPH START - Changed by SiRoB, Remain time and size Columns have been splited
	/*
	m_bRemainSort = thePrefs.TransferlistRemainSortStyle();
	int adder = 0;
	if (GetSortItem() != 9 || !m_bRemainSort)
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending() ? arrowDoubleUp : arrowDoubleDown);
		adder = 81;
	}
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100) + adder);
	*/
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100));
	//MORPH END - Changed by SiRoB, Remain time and size Columns have been splited
}

void CDownloadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CDownloadListCtrl::SetAllIcons()
{
	ApplyImageList(NULL);
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	m_ImageList.Add(CTempIconLoader(_T("SrcDownloading")));
	m_ImageList.Add(CTempIconLoader(_T("SrcOnQueue")));
	m_ImageList.Add(CTempIconLoader(_T("SrcConnecting")));
	m_ImageList.Add(CTempIconLoader(_T("SrcNNPQF")));
	m_ImageList.Add(CTempIconLoader(_T("SrcUnknown")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("Server")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	//MORPH START - Added by SiRoB, More client
	m_ImageList.Add(CTempIconLoader(_T("ClientRightEdonkey"))); //24
	m_ImageList.Add(CTempIconLoader(_T("Morph"))); //25
	m_ImageList.Add(CTempIconLoader(_T("SCARANGEL"))); //26
	m_ImageList.Add(CTempIconLoader(_T("STULLE"))); //27
	m_ImageList.Add(CTempIconLoader(_T("XTREME"))); //28
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE"))); //29
	m_ImageList.Add(CTempIconLoader(_T("EMF"))); //30
	m_ImageList.Add(CTempIconLoader(_T("NEO"))); //31
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO"))); //32
	m_ImageList.Add(CTempIconLoader(_T("XRAY"))); //33
	m_ImageList.Add(CTempIconLoader(_T("MAGIC"))); //34
	//MORPH END   - Added by SiRoB, More client
	// Mighty Knife: Community icon
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("Community")));
	// [end] Mighty Knife
	//MORPH START - Addded by SiRoB, Friend Addon
	m_overlayimages.Add(CTempIconLoader(_T("ClientFriendOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientFriendSlotOvl")));
	//MORPH END   - Addded by SiRoB, Friend Addon
	//MORPH START - Credit Overlay Icon
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	//MORPH END   - Credit Overlay Icon
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
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

	// khaos::categorymod+
	/*
	strRes = GetResString(IDS_CAT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);

	strRes = GetResString(IDS_ADDEDON);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	*/
	strRes = GetResString(IDS_ADDEDON);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	strRes = GetResString(IDS_CAT_COLCATEGORY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	strRes = GetResString(IDS_CAT_COLCATEGORY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	strRes = GetResString(IDS_CAT_COLORDER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
	// khaos::categorymod-

	// khaos::accuratetimerem+
	strRes = GetResString(IDS_REMAININGSIZE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);
	// khaos::accuratetimerem-
		
	//MORPH START - Added by SiRoB, IP2Country
	strRes = GetResString(IDS_COUNTRY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);
	//MORPH END   - Added by SiRoB, IP2Country

	CreateMenues();
	ShowFilesCount();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	//if (theApp.IsRunningAsService()) return;// MORPH leuk_he:run as ntservice v1..

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
		InsertItem(LVIF_PARAM | LVIF_TEXT, itemnr, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)newitem);

	ShowFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..
	
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    newitem->owner = owner;
    newitem->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE;
    newitem->value = source;
	newitem->dwUpdated = 0; 
	newitem->dwUpdatedchunk = 0; //MORPH - Downloading Chunk Detail Display

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
				cur_item->dwUpdatedchunk = 0; //MORPH - Downloading Chunk Detail Display
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
			InsertItem(LVIF_PARAM | LVIF_TEXT, result + 1, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)newitem);
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
	theApp.emuledlg->transferwnd->GetDownloadClientsList()->RemoveClient(source); //MORPH - Added by Stulle, Remove client from DownloadClientsList on RemoveSource [WiZaRd]
}

bool CDownloadListCtrl::RemoveFile(const CPartFile* toremove)
{
	bool bResult = false;
	if (!theApp.emuledlg->IsRunning())
		return bResult;
	// Retrieve all entries matching the File or linked to the file
	// Remark: The 'asked another files' clients must be removed from here
	ASSERT(toremove != NULL);
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
	if (theApp.IsRunningAsService(SVC_LIST_OPT)) return;// MORPH leuk_he:run as ntservice v1..
	
	if (!theApp.emuledlg->IsRunning())
		return;
	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || IsWindowVisible() == FALSE )
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toupdate);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		//MORPH START - UpdateItemThread
		/*
		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result != -1){
			updateItem->dwUpdated = 0;
			Update(result);
		}
		*/
		updateItem->dwUpdated = 0;
		updateItem->dwUpdatedchunk = 0; //MORPH - Downloading Chunk Detail Display
		m_updatethread->AddItemToUpdate((LPARAM)updateItem);
		//MORPH END - UpdateItemThread
	}
	m_availableCommandsDirty = true;
}

void CDownloadListCtrl::GetFileItemDisplayText(CPartFile *lpPartFile, int iSubItem, LPTSTR pszText, int cchTextMax)
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	switch (iSubItem)
	{
		case 0: 	// file name
			_tcsncpy(pszText, lpPartFile->GetFileName(), cchTextMax);
			break;

		case 1:		// size
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetFileSize(), false, false), cchTextMax);
			break;

		case 2:		// transferred
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetTransferred(), false, false), cchTextMax);
			break;

		case 3:		// transferred complete
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetCompletedSize(), false, false), cchTextMax);
			break;

		case 4:		// speed
			if (lpPartFile->GetTransferringSrcCount())
				_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetDatarate(), false, true), cchTextMax);
			break;

		case 5: 	// progress
			_sntprintf(pszText, cchTextMax, _T("%s: %.1f%%"), GetResString(IDS_DL_PROGRESS), lpPartFile->GetPercentCompleted());
			break;

		case 6:	{	// sources
			CString strBuffer;
			UINT sc = lpPartFile->GetSourceCount();
			//MORPH START - Modified by IceCream, [sivka: -counter for A4AF in sources column-]
			/*
// ZZ:DownloadManager -->
			if (!(lpPartFile->GetStatus() == PS_PAUSED && sc == 0) && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				UINT ncsc = lpPartFile->GetNotCurrentSourcesCount();
				strBuffer.Format(_T("%i"), sc - ncsc);
				if (ncsc > 0)
					strBuffer.AppendFormat(_T("/%i"), sc);
				if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetSrcA4AFCount() > 0)
					strBuffer.AppendFormat(_T("+%i"), lpPartFile->GetSrcA4AFCount());
				if (lpPartFile->GetTransferringSrcCount() > 0)
					strBuffer.AppendFormat(_T(" (%i)"), lpPartFile->GetTransferringSrcCount());
			}
// <-- ZZ:DownloadManager
			*/
			strBuffer.Format(_T("%i/%i/%i (%i)"),
				lpPartFile->GetSrcA4AFCount(), //MORPH - Added by SiRoB, A4AF counter
				(lpPartFile->GetSrcStatisticsValue(DS_ONQUEUE) + lpPartFile->GetSrcStatisticsValue(DS_DOWNLOADING)), //MORPH - Modified by SiRoB
				sc, lpPartFile->GetTransferringSrcCount());
			//MORPH END   - Modified by IceCream, [sivka: -counter for A4AF in sources column-]
			//MORPH START - Added by Stulle, Global Source Limit
			/*
			if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
				*/
			if (thePrefs.IsUseGlobalHL())
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetMaxSources());
			else if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
			//MORPH END   - Added by Stulle, Global Source Limit
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}

		case 7:		// prio
			switch (lpPartFile->GetDownPriority())
			{
				case PR_LOW:
					if (lpPartFile->IsAutoDownPriority())
						_tcsncpy(pszText, GetResString(IDS_PRIOAUTOLOW), cchTextMax);
					else
						_tcsncpy(pszText, GetResString(IDS_PRIOLOW), cchTextMax);
					break;

				case PR_NORMAL:
					if (lpPartFile->IsAutoDownPriority())
						_tcsncpy(pszText, GetResString(IDS_PRIOAUTONORMAL), cchTextMax);
					else
						_tcsncpy(pszText, GetResString(IDS_PRIONORMAL), cchTextMax);
					break;

				case PR_HIGH:
					if (lpPartFile->IsAutoDownPriority())
						_tcsncpy(pszText, GetResString(IDS_PRIOAUTOHIGH), cchTextMax);
					else
						_tcsncpy(pszText, GetResString(IDS_PRIOHIGH), cchTextMax);
					break;
			}
			break;

		case 8:
			_tcsncpy(pszText, lpPartFile->getPartfileStatus(), cchTextMax);
			break;

		// khaos::accuratetimerem+
		/*
		case 9:		// remaining time & size
			if (lpPartFile->GetStatus() != PS_COMPLETING && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				time_t restTime;
				if (!thePrefs.UseSimpleTimeRemainingComputation())
					restTime = lpPartFile->getTimeRemaining();
				else
					restTime = lpPartFile->getTimeRemainingSimple();
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
			}
			break;
		*/
		case 9:		// remaining time NOT size
		{
			CString strBuffer;
			if (lpPartFile->GetStatus() != PS_COMPLETING && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				switch (thePrefs.GetTimeRemainingMode()) {
				case 0:
					{
						sint32 curTime = lpPartFile->getTimeRemaining();
						sint32 avgTime = lpPartFile->GetTimeRemainingAvg();
						strBuffer.Format(_T("%s (%s)"), CastSecondsToHM(curTime), CastSecondsToHM(avgTime));
						break;
					}
				case 1:
					{
						sint32 curTime = lpPartFile->getTimeRemaining();
						strBuffer.Format(_T("%s"), CastSecondsToHM(curTime));
						break;
					}
				case 2:
					{
						sint32 avgTime = lpPartFile->GetTimeRemainingAvg();
						strBuffer.Format(_T("%s"), CastSecondsToHM(avgTime));
						break;
					}
				}
			}
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}
		// khaos::accuratetimerem-

		case 10: {	// last seen complete
			CString strBuffer;
			if (lpPartFile->m_nCompleteSourcesCountLo == 0)
				strBuffer.Format(_T("< %u"), lpPartFile->m_nCompleteSourcesCountHi);
			else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
				strBuffer.Format(_T("%u"), lpPartFile->m_nCompleteSourcesCountLo);
			else
				strBuffer.Format(_T("%u - %u"), lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);

			if (lpPartFile->lastseencomplete == NULL)
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), GetResString(IDS_NEVER), strBuffer);
			else
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), lpPartFile->lastseencomplete.Format(thePrefs.GetDateTimeFormat4Lists()), strBuffer);
			break;
		}

		case 11: // last receive
			if (lpPartFile->GetFileDate() != NULL && lpPartFile->GetCompletedSize() > (uint64)0)
				_tcsncpy(pszText, lpPartFile->GetCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()), cchTextMax);
			else
				_tcsncpy(pszText, GetResString(IDS_NEVER), cchTextMax);
			break;

		// khaos::categorymod+
		/*
		case 12: // cat
			_tcsncpy(pszText, (lpPartFile->GetCategory() != 0) ? thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle : _T(""), cchTextMax);
			break;
		case 13: // added on
			if (lpPartFile->GetCrCFileDate() != NULL)
				_tcsncpy(pszText, lpPartFile->GetCrCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()), cchTextMax);
			else
				_tcsncpy(pszText, _T("?"), cchTextMax);
			break;
		*/
		case 12: // added on
			if (lpPartFile->GetCrCFileDate() != NULL)
				_tcsncpy(pszText, lpPartFile->GetCrCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()), cchTextMax);
			else
				_tcsncpy(pszText, _T("?"), cchTextMax);
			break;
		case 13: // Category
		{
			if (!thePrefs.ShowCatNameInDownList())
				_sntprintf(pszText, cchTextMax, _T("%u"), lpPartFile->GetCategory());
			else
				_sntprintf(pszText, cchTextMax, _T("%s"), thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle);
			break;
		}
		case 14: // Resume Mod
		{
			CString strBuffer;
			strBuffer.Empty();
			//MORPH START - Added by SiRoB, ForcedA4AF
			if (thePrefs.UseSmartA4AFSwapping() && lpPartFile->GetStatus() != PS_PAUSED)
			{
				if (lpPartFile == theApp.downloadqueue->forcea4af_file) {
					//if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", ")); // this is the first item O_o
					strBuffer.AppendFormat(_T("%s"), GetResString(IDS_A4AF_FORCEALL));
				}
				if (lpPartFile->ForceA4AFOff()) {
					if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
					strBuffer.AppendFormat(_T("%s"), GetResString(IDS_A4AF_OFFFLAG));
				}
				if (lpPartFile->ForceAllA4AF()) {
					if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
					strBuffer.AppendFormat(_T("%s"), GetResString(IDS_A4AF_ONFLAG));
				}
			}
			//MORPH END   - Added by SiRoB, ForcedA4AF
			Category_Struct* ActiveCat=thePrefs.GetCategory(theApp.emuledlg->transferwnd->GetActiveCategory());
			Category_Struct* curCat=thePrefs.GetCategory(lpPartFile->GetCategory());
			if (curCat && ActiveCat && ActiveCat->viewfilters.nFromCats == 0) {
				if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
				switch (curCat->prio) {
					case PR_LOW:
						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_PRIOLOW));
						break;
					case PR_NORMAL:
						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_PRIONORMAL));
						break;
					case PR_HIGH:
						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_PRIOHIGH));
						break;
				}
				//MORPH START - Added by SiRoB, Avanced A4AF
				if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
				UINT iA4AFMode = thePrefs.AdvancedA4AFMode();
				if (iA4AFMode && curCat->iAdvA4AFMode)
					iA4AFMode = curCat->iAdvA4AFMode;
				else
					strBuffer.Append(((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
				switch (iA4AFMode) {
					case 0:
						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_A4AF_DISABLED));
						break;
					case 1:
						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_A4AF_BALANCE));
						break;
					case 2:
						strBuffer.AppendFormat(_T("%s=%u: %s"), GetResString(IDS_CAT_COLORDER), lpPartFile->GetCatResumeOrder(), GetResString(IDS_A4AF_STACK));
						break;
				}
					if (curCat->downloadInAlphabeticalOrder) {
					if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
					strBuffer.AppendFormat(_T("%s"), GetResString(IDS_DOWNLOAD_ALPHABETICAL));
				}
			} else {
				if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
				strBuffer.AppendFormat(_T("%s=%u"), GetResString(IDS_CAT_COLORDER), lpPartFile->GetCatResumeOrder());
			}
			//MORPH END   - Added by SiRoB, Avanced A4AF
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}
		// khaos::categorymod-
		// khaos::accuratetimerem+
		case 15:		// remaining size
		{
			if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE )
				_sntprintf(pszText, cchTextMax, _T("%s"), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
			break;
		}
		// khaos::accuratetimerem-
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPCRECT lpRect, UINT uDrawTextAlignment, CtrlItem_Struct *pCtrlItem)
{
	/*const*/ CPartFile *pPartFile = (CPartFile *)pCtrlItem->value;
	TCHAR szItem[1024];
	GetFileItemDisplayText(pPartFile, nColumn, szItem, _countof(szItem));
	switch (nColumn)
	{
		case 0: {	// file name
			CRect rcDraw(lpRect);
			int iIconPosY = (rcDraw.Height() > theApp.GetSmallSytemIconSize().cy) ? ((rcDraw.Height() - theApp.GetSmallSytemIconSize().cy) / 2) : 0;
			int iImage = theApp.GetFileTypeSystemImageIdx(pPartFile->GetFileName());
			if (theApp.GetSystemImageList() != NULL)
				::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), rcDraw.left, rcDraw.top + iIconPosY, ILD_TRANSPARENT);
			rcDraw.left += theApp.GetSmallSytemIconSize().cx;

			//MORPH START - Optimization
			/*
			if (thePrefs.ShowRatingIndicator() && (pPartFile->HasComment() || pPartFile->HasRating() || pPartFile->IsKadCommentSearchRunning())){
			*/
			if ( thePrefs.ShowRatingIndicator() )
				if (pPartFile->HasComment() || pPartFile->HasRating() || pPartFile->IsKadCommentSearchRunning()){
			//MORPH END   - Optimization
				m_ImageList.Draw(dc, pPartFile->UserRating(true) + 14, CPoint(rcDraw.left + 2, rcDraw.top + iIconPosY), ILD_NORMAL);
				rcDraw.left += 2 + RATING_ICON_WIDTH;
			}

			rcDraw.left += sm_iLabelOffset;
			dc->DrawText(szItem, -1, &rcDraw, MLC_DT_TEXT | uDrawTextAlignment);
			break;
		}

		case 5: {	// progress
			CRect rcDraw(*lpRect);
			rcDraw.bottom--;
			rcDraw.top++;

			int iWidth = rcDraw.Width();
			int iHeight = rcDraw.Height();
			if (pCtrlItem->status == (HBITMAP)NULL)
				VERIFY(pCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL));
			CDC cdcStatus;
			HGDIOBJ hOldBitmap;
			cdcStatus.CreateCompatibleDC(dc);
			int cx = pCtrlItem->status.GetBitmapDimension().cx; 
			DWORD dwTicks = GetTickCount();
			if (pCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !pCtrlItem->dwUpdated)
			{
				pCtrlItem->status.DeleteObject();
				pCtrlItem->status.CreateCompatibleBitmap(dc, iWidth, iHeight);
				pCtrlItem->status.SetBitmapDimension(iWidth, iHeight);
				hOldBitmap = cdcStatus.SelectObject(pCtrlItem->status);

				RECT rec_status;
				rec_status.left = 0;
				rec_status.top = 0;
				rec_status.bottom = iHeight;
				rec_status.right = iWidth;
				pPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar());
				pCtrlItem->dwUpdated = dwTicks + (rand() % 128);
			}
			else
				hOldBitmap = cdcStatus.SelectObject(pCtrlItem->status);
			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY);
			cdcStatus.SelectObject(hOldBitmap);

			if (thePrefs.GetUseDwlPercentage())
			{
				//MORPH START - Changed by SIRoB, Turn into black pen
				/*
				COLORREF oldclr = dc->SetTextColor(RGB(255, 255, 255));
				*/
				COLORREF oldclr = dc->SetTextColor(RGB(0,0,0));
				//MORPH END   - Changed by SIRoB, Turn into black pen
				int iOMode = dc->SetBkMode(TRANSPARENT);
				_sntprintf(szItem, _countof(szItem), _T("%.1f%%"), pPartFile->GetPercentCompleted());
				szItem[_countof(szItem) - 1] = _T('\0');
				//MORPH START - Changed by SiRoB, Bold Percentage :) and right justify
				/*
				dc->DrawText(szItem, -1, &rcDraw, (MLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
				*/
				CFont *pOldFont = dc->SelectObject(&m_fontBold);
					
#define	DrawPercentText	dc->DrawText(szItem, -1, &rcDraw, ((MLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
				DrawPercentText;
				rcDraw.left+=1;rcDraw.right+=1;
				DrawPercentText;
				rcDraw.left+=1;rcDraw.right+=1;
				DrawPercentText;
					
				rcDraw.top+=1;rcDraw.bottom+=1;
				DrawPercentText;
				rcDraw.top+=1;rcDraw.bottom+=1;
				DrawPercentText;
					
				rcDraw.left-=1;rcDraw.right-=1;
				DrawPercentText;
				rcDraw.left-=1;rcDraw.right-=1;
				DrawPercentText;
					
				rcDraw.top-=1;rcDraw.bottom-=1;
				DrawPercentText;
					
				rcDraw.left++;rcDraw.right++;
				dc->SetTextColor(RGB(255,255,255));
				DrawPercentText;
				dc->SelectObject(pOldFont); //MORPH - Added by SiRoB, Bold Percentage :)
				//MORPH END   - Changed by SiRoB, Bold Percentage :) and right justify
				dc->SetBkMode(iOMode);
				dc->SetTextColor(oldclr);
			}
			break;
		}

		default:
			dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			break;
	}
}

void CDownloadListCtrl::GetSourceItemDisplayText(const CtrlItem_Struct *pCtrlItem, int iSubItem, LPTSTR pszText, int cchTextMax)
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	const CUpDownClient *pClient = (CUpDownClient *)pCtrlItem->value;
	pszText[0] = _T('\0');
	switch (iSubItem)
	{
		case 0: 	// icon, name, status
			if (pClient->GetUserName() == NULL)
				_sntprintf(pszText, cchTextMax, _T("(%s)"), GetResString(IDS_UNKNOWN));
			else
				_tcsncpy(pszText, pClient->GetUserName(), cchTextMax);
			break;
	
		case 1:		// size
			switch (pClient->GetSourceFrom())
			{
				case SF_SERVER:
					_tcsncpy(pszText, _T("eD2K Server"), cchTextMax);
					break;
				case SF_KADEMLIA:
					_tcsncpy(pszText, GetResString(IDS_KADEMLIA), cchTextMax);
					break;
				case SF_SOURCE_EXCHANGE:
					_tcsncpy(pszText, GetResString(IDS_SE), cchTextMax);
					break;
				case SF_PASSIVE:
					_tcsncpy(pszText, GetResString(IDS_PASSIVE), cchTextMax);
					break;
				case SF_LINK:
					_tcsncpy(pszText, GetResString(IDS_SW_LINK), cchTextMax);
					break;
				//MORPH - Source cache
				case SF_CACHE_SERVER:
					_tcsncpy(pszText, _T("eD2K Server (SC)"), cchTextMax);
					break;
				case SF_CACHE_SOURCE_EXCHANGE:
					_sntprintf(pszText, cchTextMax, _T("%s (SC)"),GetResString(IDS_SE));
					break;
				//MORPH - Source cache
				//MORPH START - Added by SiRoB, Source Loader Saver [SLS]
				case SF_SLS:
					_tcsncpy(pszText, GetResString(IDS_SOURCE_LOADER_SAVER), cchTextMax);
					break;
				//MORPH END   - Added by SiRoB, Source Loader Saver [SLS]
			}
			break;

		case 2:		// transferred
			//MORPH START - Changed by SiRoB, Download/Upload
			if(pClient->Credits() && (pClient->Credits()->GetUploadedTotal() || pClient->Credits()->GetDownloadedTotal())){
				_sntprintf(pszText, cchTextMax, _T("%s/%s"),
				CastItoXBytes(pClient->Credits()->GetUploadedTotal(), false, false),
				CastItoXBytes(pClient->Credits()->GetDownloadedTotal(), false, false));
			}
			break;
			//MORPH END  - Changed by SiRoB, Download/Upload
		case 3:		// completed
			//MORPH START - Downloading Chunk Detail Display
			/*
			// - 'Transferred' column: Show transferred data
			// - 'Completed' column: If 'Transferred' column is hidden, show the amount of transferred data
			//	  in 'Completed' column. This is plain wrong (at least when receiving compressed data), but
			//	  users seem to got used to it.
			if (iSubItem == 2 || IsColumnHidden(2)) {
				if (pCtrlItem->type == AVAILABLE_SOURCE && pClient->GetTransferredDown())
					_tcsncpy(pszText, CastItoXBytes(pClient->GetTransferredDown(), false, false), cchTextMax);
			}
			*/
			if (pClient->GetSessionPayloadDown())
				_tcsncpy(pszText, CastItoXBytes(pClient->GetSessionPayloadDown(), false, false), cchTextMax);
			//MORPH END - Downloading Chunk Detail Display
			break;

		case 4:		// speed
			if (pCtrlItem->type == AVAILABLE_SOURCE && pClient->GetDownloadDatarate()) {
				if (pClient->GetDownloadDatarate())
					_tcsncpy(pszText, CastItoXBytes(pClient->GetDownloadDatarate(), false, true), cchTextMax);
			}
			break;

		case 5: 	// file info
			_tcsncpy(pszText, GetResString(IDS_DL_PROGRESS), cchTextMax);
			break;

		case 6:		// sources
			_tcsncpy(pszText, pClient->GetClientSoftVer(), cchTextMax);
			break;

		case 7:		// prio
			// EastShare START - Removed by TAHO, move to Status Column
			/*
			if (pClient->GetDownloadState() == DS_ONQUEUE)
			{
				if (pClient->IsRemoteQueueFull())
					_tcsncpy(pszText, GetResString(IDS_QUEUEFULL), cchTextMax);
				else if (pClient->GetRemoteQueueRank())
					_sntprintf(pszText, cchTextMax, _T("QR: %u"), pClient->GetRemoteQueueRank());
			}
			*/
			// EastShare END - Removed by TAHO, move to Status Column
			// EastShare START - Addeded by TAHO, last asked time
			{
				uint32 lastAskedTime = pClient->GetLastAskedTime();
				if ( lastAskedTime )
					_tcsncpy(pszText, CastSecondsToHM(( ::GetTickCount() - lastAskedTime) /1000), cchTextMax);
				else
					_tcsncpy(pszText, _T("?"), cchTextMax);
			}
			// EastShare END - Addeded by TAHO, last asked time
			break;

		case 8: {	// status
			CString strBuffer;
			if (pCtrlItem->type == AVAILABLE_SOURCE) {
				strBuffer = pClient->GetDownloadStateDisplayString();
			}
			else {
				strBuffer = GetResString(IDS_ASKED4ANOTHERFILE);
// ZZ:DownloadManager -->
				if (thePrefs.IsExtControlsEnabled()) {
					if (pClient->IsInNoNeededList(pCtrlItem->owner))
						strBuffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(')');
					else if (pClient->GetDownloadState() == DS_DOWNLOADING)
						strBuffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(')');
					else if (const_cast<CUpDownClient *>(pClient)->IsSwapSuspended(pClient->GetRequestFile()))
						strBuffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(')');

					if (pClient->GetRequestFile() && pClient->GetRequestFile()->GetFileName())
						strBuffer.AppendFormat(_T(": \"%s\""), pClient->GetRequestFile()->GetFileName());
				}
			}

			if (thePrefs.IsExtControlsEnabled() && !pClient->m_OtherRequests_list.IsEmpty())
				strBuffer.Append(_T("*"));
// ZZ:DownloadManager <--
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}

		case 9:		// remaining time & size
			//SLAHAM: ADDED Show Downloading Time =>
			_sntprintf(pszText, cchTextMax, _T("DT: %s (%s)[%u]"), CastSecondsToHM(pClient->dwSessionDLTime/1000), CastSecondsToHM(pClient->dwTotalDLTime/1000), pClient->uiStartDLCount);
			//SLAHAM: ADDED Show Downloading Time <=
			break;

		case 10:	// last seen complete
			//SLAHAM: ADDED Known Since =>
			if( pClient->dwThisClientIsKnownSince )
				_sntprintf(pszText, cchTextMax, _T("%s %s"), GetResString(IDS_KNOWN_SINCE), CastSecondsToHM((::GetTickCount() - pClient->dwThisClientIsKnownSince)/1000));
			else
				_tcsncpy(pszText, _T("WHO IS THAT???"), cchTextMax);
			//SLAHAM: ADDED Known Since <=
			break;

		case 11:	// last received
			break;

		case 12:	// category
			break;

		case 13:	// added on
			break;
		// Note, the labels of the above two columns are not corretct. To simplify merging I will keep the official layout.
		// MorphXT has 'category' in column 13 and 'added on' in column 12.
		case 14:	// linear priority
			break;
		//MORPH START - Remaining Client Available Data
		case 15:	// remaining size
			_tcsncpy(pszText, CastItoXBytes(pClient->GetRemainingAvailableData((CPartFile*)pCtrlItem->owner), false, false), cchTextMax);
			break;
		//MORPH END   - Remaining Client Available Data
		//MORPH START - Added by SiRoB, IP2Country
		case 16:
			_tcsncpy(pszText, pClient->GetCountryName(), cchTextMax);
			break;
		//MORPH END   - Added by SiRoB, IP2Country
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPCRECT lpRect, UINT uDrawTextAlignment, CtrlItem_Struct *pCtrlItem)
{
	const CUpDownClient *pClient = (CUpDownClient *)pCtrlItem->value;
	TCHAR szItem[1024];
	GetSourceItemDisplayText(pCtrlItem, nColumn, szItem, _countof(szItem));
	switch (nColumn)
	{
		case 0: {	// icon, name, status
			CRect cur_rec(*lpRect);
			int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
			POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
			if (pCtrlItem->type == AVAILABLE_SOURCE)
			{
				switch (pClient->GetDownloadState())
				{
					case DS_CONNECTING:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_CONNECTED:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_WAITCALLBACKKAD:
					case DS_WAITCALLBACK:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_ONQUEUE:
						if (pClient->IsRemoteQueueFull())
							m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						else
							m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
						break;
					case DS_DOWNLOADING:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						break;
					case DS_REQHASHSET:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						break;
					case DS_NONEEDEDPARTS:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						break;
					case DS_ERROR:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						break;
					case DS_TOOMANYCONNS:
					case DS_TOOMANYCONNSKAD:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					default:
						m_ImageList.Draw(dc, 4, point, ILD_NORMAL);
						break;
				}
			}
			else {
				m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
			}
			cur_rec.left += 20;

			UINT uOvlImg = 0;
			if ((pClient->Credits() && pClient->Credits()->GetCurrentIdentState(pClient->GetIP()) == IS_IDENTIFIED))
				uOvlImg |= 1;
			if (pClient->IsObfuscatedConnectionEstablished())
				uOvlImg |= 2;

			POINT point2 = {cur_rec.left, cur_rec.top + iIconPosY};
			//MORPH START - Modified by SiRoB, More client & ownCredit overlay icon
			//MORPH - Removed by SiRoB, Friend Addon
			/*
			if (pClient->IsFriend())
				m_ImageList.Draw(dc, 6, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else*/ if (pClient->GetClientSoft() == SO_EDONKEYHYBRID)
				m_ImageList.Draw(dc, 9, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->GetClientSoft() == SO_MLDONKEY)
				m_ImageList.Draw(dc, 8, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->GetClientSoft() == SO_SHAREAZA)
				m_ImageList.Draw(dc, 10, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->GetClientSoft() == SO_URL)
				m_ImageList.Draw(dc, 11, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->GetClientSoft() == SO_AMULE)
				m_ImageList.Draw(dc, 12, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->GetClientSoft() == SO_LPHANT)
				m_ImageList.Draw(dc, 13, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->ExtProtocolAvailable())
			//MORPH START - Modified by SiRoB, More client icon & Credit overlay icon
			{
				if(pClient->GetModClient() == MOD_NONE)
					m_ImageList.Draw(dc, 5, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else
					m_ImageList.Draw(dc, pClient->GetModClient()+24, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			}
			//MORPH END   - Modified by SiRoB, More client icon & Credit overlay icon
			else if ( pClient->GetClientSoft() == SO_EDONKEY)
					m_ImageList.Draw(dc, 24, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			//MORPH END - Modified by SiRoB, More client & ownCredits overlay icon
			else
				m_ImageList.Draw(dc, 7, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			//MORPH START - Credit Overlay Icon
			if (pClient->Credits() && pClient->Credits()->GetMyScoreRatio(pClient->GetIP()) > 1) {
				if (uOvlImg & 1)
					m_overlayimages.Draw(dc, 4, point2, ILD_TRANSPARENT);
				else
					m_overlayimages.Draw(dc, 3, point2, ILD_TRANSPARENT);
			}
			//MORPH END   - Credit Overlay Icon
			//MORPH START - Friend Addon
			if (pClient->IsFriend())
				m_overlayimages.Draw(dc, pClient->GetFriendSlot()?2:1,point2, ILD_TRANSPARENT);
			//MORPH END   - Friend Addon
			// Mighty Knife: Community visualization
			if (pClient->IsCommunity())
				m_overlayimages.Draw(dc,0, point2, ILD_TRANSPARENT);
			// [end] Mighty Knife
			cur_rec.left += 20;

			//Morph Start - added by AndCycle, IP to Country
			if(theApp.ip2country->ShowCountryFlag() && IsColumnHidden(16)){
				POINT point3= {cur_rec.left,cur_rec.top+1};
				theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, pClient->GetCountryFlagIndex(), point3, CSize(18,16), CPoint(0,0), ILD_NORMAL);
				cur_rec.left+=20;
			}
			//Morph End - added by AndCycle, IP to Country

			//MORPH START - Added by IceCream, [sivka: -A4AF counter, ahead of user nickname-]
			CString buffer;
			buffer.Format(_T("(%i) "),pClient->m_OtherRequests_list.GetCount()+1+pClient->m_OtherNoNeeded_list.GetCount());
			dc->DrawText(buffer, buffer.GetLength(), &cur_rec, MLC_DT_TEXT);
			cur_rec.left += dc->GetTextExtent(buffer).cx;
			//MORPH END   - Added by IceCream, [sivka: -A4AF counter, ahead of user nickname-]

			dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
			break;
		}

		//MORPH START - Downloading Chunk Detail Display
		case 3:// completed
		{
			if (pCtrlItem->type == AVAILABLE_SOURCE && pClient->GetDownloadState() == DS_DOWNLOADING) {
				CRect rcDraw(*lpRect);
				rcDraw.bottom--;
				rcDraw.top++; 

				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (pCtrlItem->statuschunk == (HBITMAP)NULL)
					VERIFY(pCtrlItem->statuschunk.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = pCtrlItem->statuschunk.GetBitmapDimension().cx;
				DWORD dwTicks = GetTickCount();
				if(pCtrlItem->dwUpdatedchunk + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !pCtrlItem->dwUpdatedchunk) { 
					pCtrlItem->statuschunk.DeleteObject(); 
					pCtrlItem->statuschunk.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					pCtrlItem->statuschunk.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(pCtrlItem->statuschunk); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					//MORPH START - Changed by SiRoB, Advanced A4AF derivated from Khaos
					/*
					pClient->DrawStatusBarChunk(&cdcStatus,  &rec_status,(pCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 
					*/
					pClient->DrawStatusBarChunk(&cdcStatus,  &rec_status,(CPartFile*)pCtrlItem->owner, thePrefs.UseFlatBar());
					//MORPH END   - Changed by SiRoB, Advanced A4AF derivated from Khaos

					CString buffer;
					COLORREF oldclr = cdcStatus.SetTextColor(RGB(0,0,0));
					int iOMode = cdcStatus.SetBkMode(TRANSPARENT);
					if (pClient->GetCurrentDownloadingChunk()==(UINT)-1) {
						if (pClient->m_lastPartAsked==(uint16)-1)
							buffer = _T("?");
						else
							buffer.Format(_T("%u"), pClient->m_lastPartAsked);
					} else
						buffer.Format(_T("%u"), pClient->GetCurrentDownloadingChunk());
					buffer.AppendFormat(_T(" @ %.1f%%"), pClient->GetDownChunkProgressPercent());
					CFont *pOldFont = cdcStatus.SelectObject(&m_fontSmaller);
#define	DrawClientPercentTextLeft		cdcStatus.DrawText(buffer, buffer.GetLength(),&rec_status, MLC_DT_TEXT)
					rec_status.top-=1;rec_status.bottom-=1;
					rec_status.left+=1;rec_status.right-=3;
					DrawClientPercentTextLeft;rec_status.left+=1;rec_status.right+=1;
					DrawClientPercentTextLeft;rec_status.left+=1;rec_status.right+=1;
					DrawClientPercentTextLeft;rec_status.top+=1;rec_status.bottom+=1;
					DrawClientPercentTextLeft;rec_status.top+=1;rec_status.bottom+=1;
					DrawClientPercentTextLeft;rec_status.left-=1;rec_status.right-=1;
					DrawClientPercentTextLeft;rec_status.left-=1;rec_status.right-=1;
					DrawClientPercentTextLeft;rec_status.top-=1;rec_status.bottom-=1;
					DrawClientPercentTextLeft;rec_status.left++;rec_status.right++;
					cdcStatus.SetTextColor(RGB(255,255,255));
					DrawClientPercentTextLeft;
					
					cdcStatus.SetTextColor(RGB(0,0,0));
					buffer.Format(_T("%s"), CastItoXBytes(pClient->GetSessionPayloadDown(), false, false));
#define	DrawClientPercentTextRight		cdcStatus.DrawText(buffer, buffer.GetLength(),&rec_status, MLC_DT_TEXT | DT_RIGHT)
					rec_status.top-=1;rec_status.bottom-=1;
					DrawClientPercentTextRight;rec_status.left+=1;rec_status.right+=1;
					DrawClientPercentTextRight;rec_status.left+=1;rec_status.right+=1;
					DrawClientPercentTextRight;rec_status.top+=1;rec_status.bottom+=1;
					DrawClientPercentTextRight;rec_status.top+=1;rec_status.bottom+=1;
					DrawClientPercentTextRight;rec_status.left-=1;rec_status.right-=1;
					DrawClientPercentTextRight;rec_status.left-=1;rec_status.right-=1;
					DrawClientPercentTextRight;rec_status.top-=1;rec_status.bottom-=1;
					DrawClientPercentTextRight;rec_status.left++;rec_status.right++;
					cdcStatus.SetTextColor(RGB(255,255,255));
					DrawClientPercentTextRight;

					cdcStatus.SelectObject(pOldFont);
					cdcStatus.SetBkMode(iOMode);
					cdcStatus.SetTextColor(oldclr);
					
					pCtrlItem->dwUpdatedchunk = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(pCtrlItem->statuschunk); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
			} else if (pClient->GetSessionPayloadDown())
				dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			break;
		}
		//MORPH END - Downloading Chunk Detail Display

		case 5: {	// file info
			CRect rcDraw(*lpRect);
			rcDraw.bottom--;
			rcDraw.top++;

			int iWidth = rcDraw.Width();
			int iHeight = rcDraw.Height();
			if (pCtrlItem->status == (HBITMAP)NULL)
				VERIFY(pCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL));
			CDC cdcStatus;
			HGDIOBJ hOldBitmap;
			cdcStatus.CreateCompatibleDC(dc);
			int cx = pCtrlItem->status.GetBitmapDimension().cx;
			DWORD dwTicks = GetTickCount();
			if (pCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !pCtrlItem->dwUpdated)
			{
				pCtrlItem->status.DeleteObject();
				pCtrlItem->status.CreateCompatibleBitmap(dc, iWidth, iHeight);
				pCtrlItem->status.SetBitmapDimension(iWidth, iHeight);
				hOldBitmap = cdcStatus.SelectObject(pCtrlItem->status);

				RECT rec_status;
				rec_status.left = 0;
				rec_status.top = 0;
				rec_status.bottom = iHeight;
				rec_status.right = iWidth;
				//MORPH START - Changed by SiRoB, Advanced A4AF derivated from Khaos
				/*
				pClient->DrawStatusBar(&cdcStatus,  &rec_status,(pCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar());
				*/
				pClient->DrawStatusBar(&cdcStatus,  &rec_status,(CPartFile*)pCtrlItem->owner, thePrefs.UseFlatBar());
				//MORPH END   - Changed by SiRoB, Advanced A4AF derivated from Khaos

				//Commander - Added: Client percentage - Start
				//MORPH - Changed by SiRoB, Keep A4AF info
				if (thePrefs.GetUseClientPercentage() && pClient->GetPartStatus((CPartFile*)pCtrlItem->owner) /*&& pCtrlItem->type == AVAILABLE_SOURCE*/)
				{
					float percent = (float)pClient->GetAvailablePartCount(((CPartFile*)pCtrlItem->owner)) / (float)((CPartFile*)pCtrlItem->owner)->GetPartCount()* 100.0f;
					if (percent > 0.05f)
					{
						//Commander - Added: Draw Client Percentage xored, caching before draw - Start
						CString buffer;
						buffer.Format(_T("%.1f%%"), percent);
						COLORREF oldclr = cdcStatus.SetTextColor(RGB(0,0,0));
						int iOMode = cdcStatus.SetBkMode(TRANSPARENT);
						CFont *pOldFont = cdcStatus.SelectObject(&m_fontSmaller);
#define	DrawClientPercentText		cdcStatus.DrawText(buffer, buffer.GetLength(),&rec_status, ((MLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
						rec_status.top-=1;rec_status.bottom-=1;
						DrawClientPercentText;rec_status.left+=1;rec_status.right+=1;
						DrawClientPercentText;rec_status.left+=1;rec_status.right+=1;
						DrawClientPercentText;rec_status.top+=1;rec_status.bottom+=1;
						DrawClientPercentText;rec_status.top+=1;rec_status.bottom+=1;
						DrawClientPercentText;rec_status.left-=1;rec_status.right-=1;
						DrawClientPercentText;rec_status.left-=1;rec_status.right-=1;
						DrawClientPercentText;rec_status.top-=1;rec_status.bottom-=1;
						DrawClientPercentText;rec_status.left++;rec_status.right++;
						cdcStatus.SetTextColor(RGB(255,255,255));
						DrawClientPercentText;
						cdcStatus.SelectObject(pOldFont);
						cdcStatus.SetBkMode(iOMode);
						cdcStatus.SetTextColor(oldclr);
						//Commander - Added: Draw Client Percentage xored, caching before draw - End	
					}
				}
				//Commander - Added: Client percentage - End
				pCtrlItem->dwUpdated = dwTicks + (rand() % 128);
			}
			else
				hOldBitmap = cdcStatus.SelectObject(pCtrlItem->status);
			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY);
			cdcStatus.SelectObject(hOldBitmap);
			break;
		}

		// EastShare START - Added by TAHO, color
		case 8:	{	// status
			COLORREF crOldTxtColor = dc->GetTextColor();
			if (pCtrlItem->type == AVAILABLE_SOURCE){
				switch (pClient->GetDownloadState()) {
					case DS_CONNECTING:
						dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_CONNECTED:
						dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_WAITCALLBACK:
						dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_ONQUEUE:
						if( pClient->IsRemoteQueueFull() ){
							dc->SetTextColor((COLORREF)RGB(10,130,160));
						}
						else {
							if ( pClient->GetRemoteQueueRank()){
								DWORD	estimatedTime = pClient->GetRemoteQueueRankEstimatedTime();
								if (estimatedTime == (DWORD)-1)
									dc->SetTextColor((COLORREF)RGB(240,125,10));
								else if(estimatedTime == 0 || estimatedTime > GetTickCount()+FILEREASKTIME)
									dc->SetTextColor((COLORREF)RGB(60,10,240));
								else
									dc->SetTextColor((COLORREF)RGB(10,180,50));
							}
							else{
								dc->SetTextColor((COLORREF)RGB(50,80,140));
							}
						}
						break;
					case DS_DOWNLOADING:
						dc->SetTextColor((COLORREF)RGB(192,0,0));
						break;
					case DS_REQHASHSET:
						dc->SetTextColor((COLORREF)RGB(245,240,100));
						break;
					case DS_NONEEDEDPARTS:
						dc->SetTextColor((COLORREF)RGB(30,200,240)); 
						break;
					case DS_LOWTOLOWIP:
						dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					case DS_TOOMANYCONNS:
						dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					default:
						dc->SetTextColor((COLORREF)RGB(135,135,135)); 
				}
			}
			dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			dc->SetTextColor(crOldTxtColor);
			break;
		}
		// EastShare END   - Added by TAHO, color
		//MORPH START - We got text here
		/*
		case 9:		// remaining time & size
		case 10:	// last seen complete
		*/
		//MORPH END   - We got text here
		case 11:	// last received
		case 12:	// category
		case 13:	// added on
			break;
		// Note, the labels of the above two columns are not correct. To simplify merging I will keep the official layout.
		// MorphXT has 'category' in column 13 and 'added on' in column 12.
		case 14:	//MORPH - linear priority
			break;
		//MORPH START - Added by SiRoB, IP2Country
		case 16: {
			RECT cur_rec = *lpRect;
			if(theApp.ip2country->ShowCountryFlag()){
				POINT point3= {cur_rec.left,cur_rec.top+1};
				theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, pClient->GetCountryFlagIndex(), point3, CSize(18,16), CPoint(0,0), ILD_NORMAL);
				cur_rec.left+=20;
			}
			dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
			if(theApp.ip2country->ShowCountryFlag()){
				cur_rec.left-=20;
			}
			break;
		}
		//MORPH END   - Added by SiRoB, IP2Country

		default:
			dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			break;
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	BOOL bCtrlFocused;
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	CtrlItem_Struct *content = (CtrlItem_Struct *)lpDrawItemStruct->itemData;
	//MORPH START - Changed by SiRoB, Show Downloading file in bold
	/*
	if (m_pFontBold)
	*/
	if (m_pFontBold && thePrefs.GetShowActiveDownloadsBold())
	//MORPH END   - Changed by SiRoB, Show Downloading file in bold
	{
		if (content->type == FILE_TYPE && ((const CPartFile *)content->value)->GetTransferringSrcCount())
			dc.SelectObject(m_pFontBold);
		else if ((content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE) && (((const CUpDownClient *)content->value)->GetDownloadState() == DS_DOWNLOADING))
			dc.SelectObject(m_pFontBold);
	}

	BOOL notLast = lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start=0;
	int tree_end=0;

	int iTreeOffset = 6;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - sm_iLabelOffset;
	cur_rec.left += sm_iIconOffset;

	if (content->type == FILE_TYPE)
	{
		if (!g_bLowColorDesktop && (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) {
			DWORD dwCatColor = thePrefs.GetCatColor(((/*const*/ CPartFile*)content->value)->GetCategory(), COLOR_WINDOWTEXT);
			if (dwCatColor > 0)
				dc.SetTextColor(dwCatColor);
			//MORPH START - Added by IceCream, show download in red
			if(thePrefs.GetEnableDownloadInRed() && ((/*const*/ CPartFile*)content->value)->GetTransferringSrcCount())
				dc.SetTextColor(RGB(192,0,0));
			//MORPH END   - Added by IceCream, show download in red
			//MORPH START - Added by FrankyFive, show paused files in gray
			if(((/*const*/ CPartFile*)content->value)->GetStatus() == PS_PAUSED)
				dc.SetTextColor(RGB(128,128,128));
		    //MORPH END   - Added by FrankyFive, show paused files in gray
		}

		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			if (!IsColumnHidden(iColumn))
			{
				UINT uDrawTextAlignment;
				int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
				if (iColumn == 5)
				{
					int iNextLeft = cur_rec.left + iColumnWidth;
					int iNextRight = cur_rec.right + iColumnWidth;
					//set up tree vars
					cur_rec.left = cur_rec.right + iTreeOffset;
					cur_rec.right = cur_rec.left + min(8, iColumnWidth);
					tree_start = cur_rec.left + 1;
					tree_end = cur_rec.right;
					//normal column stuff
					cur_rec.left = cur_rec.right + 1;
					cur_rec.right = tree_start + iColumnWidth - iTreeOffset;
					if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
						DrawFileItem(dc, 5, &cur_rec, uDrawTextAlignment, content);
					cur_rec.left = iNextLeft;
					cur_rec.right = iNextRight;
				}
				else
				{
					cur_rec.right += iColumnWidth;
					if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
						DrawFileItem(dc, iColumn, &cur_rec, uDrawTextAlignment, content);
					if (iColumn == 0) {
						cur_rec.left += sm_iLabelOffset;
						cur_rec.right -= sm_iSubItemInset;
					}
					cur_rec.left += iColumnWidth;
				}
			}
		}
	}
	else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE)
	{
		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			if (!IsColumnHidden(iColumn))
			{
				UINT uDrawTextAlignment;
				int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
				if (iColumn == 5)
				{
					int iNextLeft = cur_rec.left + iColumnWidth;
					int iNextRight = cur_rec.right + iColumnWidth;
					//set up tree vars
					cur_rec.left = cur_rec.right + iTreeOffset;
					cur_rec.right = cur_rec.left + min(8, iColumnWidth);
					tree_start = cur_rec.left + 1;
					tree_end = cur_rec.right;
					//normal column stuff
					cur_rec.left = cur_rec.right + 1;
					cur_rec.right = tree_start + iColumnWidth - iTreeOffset;
					if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
						DrawSourceItem(dc, 5, &cur_rec, uDrawTextAlignment, content);
					cur_rec.left = iNextLeft;
					cur_rec.right = iNextRight;
				}
				else
				{
					cur_rec.right += iColumnWidth;
					if (cur_rec.left < cur_rec.right && HaveIntersection(rcClient, cur_rec))
						DrawSourceItem(dc, iColumn, &cur_rec, uDrawTextAlignment, content);
					if (iColumn == 0) {
						cur_rec.left += sm_iLabelOffset;
						cur_rec.right -= sm_iSubItemInset;
					}
					cur_rec.left += iColumnWidth;
				}
			}
		}
	}

	DrawFocusRect(dc, lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_FOCUS, bCtrlFocused, lpDrawItemStruct->itemState & ODS_SELECTED);

	//draw tree last so it draws over selected and focus (looks better)
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
	if (!theApp.IsRunningAsService(SVC_LIST_OPT) && (content->type == FILE_TYPE || content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE)) // MORPH leuk_he:run as ntservice v1..
		m_updatethread->AddItemUpdated((LPARAM)content->value); //MORPH - UpdateItemThread
}

void CDownloadListCtrl::HideSources(CPartFile* toCollapse)
{
	SetRedraw(false);
	int pre = 0;
	int post = 0;
	for (int i = 0; i < GetItemCount(); i++)
	{
		CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData(i);
		if (item != NULL && item->owner == toCollapse)
		{
			pre++;
			item->dwUpdated = 0;
			item->dwUpdatedchunk = 0; //MORPH - Downloading Chunk Detail Display
			item->status.DeleteObject();
			item->statuschunk.DeleteObject(); //MORPH - Downloading Chunk Detail Display
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
	if (content != NULL && bCollapseSource && content->parent != NULL)
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
					InsertItem(LVIF_PARAM | LVIF_TEXT, iItem + 1, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)cur_item);
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
}

void CDownloadListCtrl::OnLvnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
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
		if (content != NULL && content->type == FILE_TYPE)
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
			int iFilesCanPauseOnPreview = 0;
			int iFilesDoPauseOnPreview = 0;
			int iFilesInCats = 0;
			int iFileForceA4AF = 0; //MORPH - Added by SiRoB, A4AF
			int iFileForceAllA4AF = 0; //MORPH - Added by SiRoB, A4AF
			int iFileForceA4AFOff = 0; //MORPH - Added by SiRoB, A4AF
			int iFileNotSeenCompleteSource = 0; //MORPH - Added by SiRoB, Only download complete files v2.1 (shadow)
			int iFileFollowTheMajority = 0; // EastShare       - FollowTheMajority by AndCycle
			int	iFilesToImport = 0; //MORPH START - Added by SiRoB, Import Part

			UINT uPrioMenuItem = 0;
			UINT uPermMenuItem = 0; //MORPH - Added by SiRoB, showSharePermissions
			const CPartFile* file1 = NULL;
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData == NULL || pItemData->type != FILE_TYPE)
					continue;
				const CPartFile* pFile = (CPartFile*)pItemData->value;
				if (bFirstItem)
					file1 = pFile;
				iSelectedItems++;

				bool bFileDone = (pFile->GetStatus()==PS_COMPLETE || pFile->GetStatus()==PS_COMPLETING);
				iFilesToCancel += pFile->GetStatus() != PS_COMPLETING ? 1 : 0;
				 //MORPH START - Added by SiRoB, Import Part
				iFilesToImport += pFile->GetFileOp() == PFOP_SR13_IMPORTPARTS ? 1 : 0;
				 //MORPH END   - Added by SiRoB, Import Part
				iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
                iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
                iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
				iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;
				iFilesCanPauseOnPreview += (pFile->IsPreviewableFileType() && !pFile->IsReadyForPreview() && pFile->CanPauseFile()) ? 1 : 0;
				iFilesDoPauseOnPreview += (pFile->IsPausingOnPreview()) ? 1 : 0;
				iFilesInCats += (!pFile->HasDefaultCategory()) ? 1 : 0; 
				//MORPH START - Added by SiRoB, Only download complete files v2.1 (shadow)
				iFileNotSeenCompleteSource += pFile->notSeenCompleteSource() && pFile->GetStatus() != PS_ERROR && !bFileDone;
				//MORPH END   - Added by SiRoB, Only download complete files v2.1 (shadow)
				iFileFollowTheMajority += pFile->DoFollowTheMajority() ? 1 : 0; // EastShare       - FollowTheMajority by AndCycle
				//MORPH START - Added by SiRoB, A4AF
				iFileForceAllA4AF += (theApp.downloadqueue->forcea4af_file == pFile)?1:0;
				iFileForceA4AF += pFile->ForceAllA4AF()?1:0;
				iFileForceA4AFOff += pFile->ForceA4AFOff()?1:0;
				//MORPH END   - Added by SiRoB, A4AF

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

				//MORPH START - Added by SiRoB, showSharePermissions
				UINT uCurPermMenuItem = 0;
				if (pFile->GetPermissions()==-1)
					uCurPermMenuItem = MP_PERMDEFAULT;
				else if (pFile->GetPermissions()==PERM_ALL)
					uCurPermMenuItem = MP_PERMALL;
				else if (pFile->GetPermissions() == PERM_FRIENDS)
					uCurPermMenuItem = MP_PERMFRIENDS;
				else if (pFile->GetPermissions() == PERM_NOONE)
					uCurPermMenuItem = MP_PERMNONE;
				// Mighty Knife: Community visible filelist
				else if (pFile->GetPermissions() == PERM_COMMUNITY)
					uCurPermMenuItem = MP_PERMCOMMUNITY;
				// [end] Mighty Knife
				else
					ASSERT(0);

				if (bFirstItem)
					uPermMenuItem = uCurPermMenuItem;
				else if (uPermMenuItem != uCurPermMenuItem)
					uPermMenuItem = 0;
				//MORPH END   - Added by SiRoB, showSharePermissions
				bFirstItem = false;
			}

			m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
			m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
			//MORPH - Added by SiRoB, khaos::kmod+ Popup menu should be disabled when advanced A4AF mode is turned off and we need to check appropriate A4AF items.
			if (thePrefs.IsExtControlsEnabled()) {
			    m_SourcesMenu.EnableMenuItem(MP_FORCEA4AF, thePrefs.UseSmartA4AFSwapping() && iSelectedItems == 1 && iFilesNotDone == 1? MF_ENABLED : MF_GRAYED);
			    m_SourcesMenu.CheckMenuItem(MP_FORCEA4AF,  iFileForceAllA4AF > 0 && iSelectedItems == 1 ? MF_CHECKED : MF_UNCHECKED); // todo
			
			    m_SourcesMenu.EnableMenuItem((UINT_PTR)m_A4AFMenuFlag.m_hMenu, iFilesNotDone > 0 && (thePrefs.AdvancedA4AFMode() || thePrefs.UseSmartA4AFSwapping())? MF_ENABLED : MF_GRAYED);
			     m_A4AFMenuFlag.ModifyMenu(MP_FORCEA4AFONFLAG, (iFileForceA4AF > 0 && iSelectedItems == 1 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, MP_FORCEA4AFONFLAG, ((GetSelectedCount() > 1) ? GetResString(IDS_INVERT) + _T(" ") : _T("")) + GetResString(IDS_A4AF_ONFLAG));
			    m_A4AFMenuFlag.ModifyMenu(MP_FORCEA4AFOFFFLAG, (iFileForceA4AFOff > 0 && iSelectedItems == 1 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, MP_FORCEA4AFOFFFLAG, ((GetSelectedCount() > 1) ? GetResString(IDS_INVERT) + _T(" ") :  _T("")) + GetResString(IDS_A4AF_OFFFLAG));
			}
			// khaos::kmod-

			// enable commands if there is at least one item which can be used for the action
			m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesToCancel > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);
			
			//EastShare Start - Added by AndCycle, Only download complete files v2.1 (shadow)
			m_FileMenu.EnableMenuItem(MP_FORCE,iFileNotSeenCompleteSource > 0 ? MF_ENABLED : MF_GRAYED);
			//EastShare End - Added by AndCycle, Only download complete files v2.1 (shadow)

			//MORPH START show less controls
			if(!thePrefs.IsLessControls())
			{
				//EastShare Start - FollowTheMajority by AndCycle
				m_FileMenu.EnableMenuItem(MP_FOLLOW_THE_MAJORITY,MF_ENABLED); // just in case
				if(iSelectedItems == 1)
					m_FileMenu.CheckMenuItem(MP_FOLLOW_THE_MAJORITY, iFileFollowTheMajority == 1 ? MF_CHECKED : MF_UNCHECKED);
				else if(iFileFollowTheMajority == 0)
					m_FileMenu.CheckMenuItem(MP_FOLLOW_THE_MAJORITY, MF_UNCHECKED);
				else if(iFileFollowTheMajority == iSelectedItems)
					m_FileMenu.CheckMenuItem(MP_FOLLOW_THE_MAJORITY, MF_CHECKED);
				else //if(iSelectedItems > 1 && iFileFollowTheMajority != iSelectedItems)
					m_FileMenu.ModifyMenu(MP_FOLLOW_THE_MAJORITY, MF_UNCHECKED | MF_STRING, MP_FOLLOW_THE_MAJORITY, GetResString(IDS_INVERT) + _T(" ") + GetResString(IDS_FOLLOW_THE_MAJORITY));
				//EastShare End   - FollowTheMajority by AndCycle
			}
			//MORPH END show less controls

			bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
			m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);
            
			CMenu PreviewWithMenu;
			PreviewWithMenu.CreateMenu();
			int iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(PreviewWithMenu, (iSelectedItems == 1) ? file1 : NULL);
			if(thePrefs.IsExtControlsEnabled())
			{
				if (!thePrefs.GetPreviewPrio())
				{
					m_PreviewMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesPreviewType == 1 && iFilesToPreview == 0 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
					m_PreviewMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesGetPreviewParts == 1) ? MF_CHECKED : MF_UNCHECKED);
				}
				m_PreviewMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				m_PreviewMenu.EnableMenuItem(MP_PAUSEONPREVIEW, iFilesCanPauseOnPreview > 0 ? MF_ENABLED : MF_GRAYED);
				m_PreviewMenu.CheckMenuItem(MP_PAUSEONPREVIEW, (iSelectedItems > 0 && iFilesDoPauseOnPreview == iSelectedItems) ? MF_CHECKED : MF_UNCHECKED);
				if (iPreviewMenuEntries > 0 && !thePrefs.GetExtraPreviewWithMenu())
					m_PreviewMenu.InsertMenu(1, MF_POPUP | MF_BYPOSITION | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
				else if (iPreviewMenuEntries > 0)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | MF_BYCOMMAND | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
            }
			else {
				m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				if (iPreviewMenuEntries)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | MF_BYCOMMAND | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
			}

			bool bDetailsEnabled = (iSelectedItems > 0);
			m_FileMenu.EnableMenuItem(MP_METINFO, bDetailsEnabled ? MF_ENABLED : MF_GRAYED);
			if (thePrefs.IsDoubleClickEnabled() && bOpenEnabled)
				m_FileMenu.SetDefaultItem(MP_OPEN);
			else if (!thePrefs.IsDoubleClickEnabled() && bDetailsEnabled)
				m_FileMenu.SetDefaultItem(MP_METINFO);
			else
				m_FileMenu.SetDefaultItem((UINT)-1);
			m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, (iSelectedItems >= 1 /*&& iFilesNotDone == 1*/) ? MF_ENABLED : MF_GRAYED);
			
			//MORPH START show less controls
			if(!thePrefs.IsLessControls())
			{
				//MORPH START - Added by SiRoB, Import Parts [SR13]
				m_FileMenu.ModifyMenuAndIcon(MP_SR13_ImportParts, MF_STRING, MP_SR13_ImportParts,(iFilesToImport > 0) ? GetResString(IDS_IMPORTPARTS_STOP) :GetResString(IDS_IMPORTPARTS), _T("FILEIMPORTPARTS"));
				m_FileMenu.EnableMenuItem(MP_SR13_ImportParts, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
				
				//m_FileMenu.EnableMenuItem(MP_SR13_InitiateRehash, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
				//MORPH END   - Added by SiRoB, Import Parts [SR13]
			}
			//MORPH END show less controls

			int total;
			m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab, total) > 0 ? MF_ENABLED : MF_GRAYED);

			if (m_SourcesMenu && thePrefs.IsExtControlsEnabled()) {
				m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_ENABLED);
				m_SourcesMenu.EnableMenuItem(MP_ADDSOURCE, (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED);
				m_SourcesMenu.EnableMenuItem(MP_SETSOURCELIMIT, (iFilesNotDone == iSelectedItems) ? MF_ENABLED : MF_GRAYED);
			}

			m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

			//MORPH START show less controls
			if(!thePrefs.IsLessControls())
			{
				//MORPH START - Added by SiRoB, Show Share Permissions
				m_FileMenu.EnableMenuItem((UINT_PTR)m_PermMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
				CString buffer;
				switch (thePrefs.GetPermissions()){
					case PERM_ALL:
					buffer.Format(_T(" (%s)"),GetResString(IDS_PW_EVER));
						break;
					case PERM_FRIENDS:
					buffer.Format(_T(" (%s)"),GetResString(IDS_FSTATUS_FRIENDSONLY));
						break;
					case PERM_NOONE:
					buffer.Format(_T(" (%s)"),GetResString(IDS_HIDDEN));
						break;
					// Mighty Knife: Community visible filelist
					case PERM_COMMUNITY:
					buffer.Format(_T(" (%s)"),GetResString(IDS_COMMUNITY));
						break;
					// [end] Mighty Knife
					default:
					buffer = _T(" (?)");
						break;
				}
				m_PermMenu.ModifyMenu(MP_PERMDEFAULT, MF_STRING, MP_PERMDEFAULT, GetResString(IDS_DEFAULT) + buffer);
				// Mighty Knife: Community visible filelist
				m_PermMenu.CheckMenuRadioItem(MP_PERMDEFAULT, MP_PERMCOMMUNITY, uPermMenuItem, 0);
				// [end] Mighty Knife
				//MORPH END   - Added by SiRoB, Show Share Permissions
			}
			//MORPH END show less controls

			m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK, iSelectedItems > 0? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK_US, iSelectedItems > 0? MF_ENABLED : MF_GRAYED);
			//MORPH START show less controls
			if(!thePrefs.IsLessControls())
				m_FileMenu.EnableMenuItem(MP_MASSRENAME, iSelectedItems > 0? MF_ENABLED : MF_GRAYED); //Commander - Added: MassRename [Dragon]
			//MORPH END show less controls
	
			CTitleMenu WebMenu;
			WebMenu.CreateMenu();
			WebMenu.AddMenuTitle(NULL, true);
			int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
			UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

			// create cat-submenue
			CMenu CatsMenu;
			CatsMenu.CreateMenu();
			FillCatsMenu(CatsMenu, iFilesInCats);
			m_FileMenu.AppendMenu(MF_POPUP, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT), _T("CATEGORY"));

			//MORPH START - Added by SiRoB, Khaos Category
			CTitleMenu mnuOrder;
			if (this->GetSelectedCount() > 1) {
				mnuOrder.CreatePopupMenu();
				mnuOrder.AddMenuTitle(GetResString(IDS_CAT_SETORDER));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERAUTOINC, GetResString(IDS_CAT_MNUAUTOINC));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERSTEPTHRU, GetResString(IDS_CAT_MNUSTEPTHRU));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERALLSAME, GetResString(IDS_CAT_MNUALLSAME));
				m_FileMenu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)mnuOrder.m_hMenu, GetResString(IDS_CAT_SETORDER), _T("FILELINEARPRIO"));
			}
			else {
				m_FileMenu.AppendMenu(MF_STRING, MP_CAT_SETRESUMEORDER, GetResString(IDS_CAT_SETORDER), _T("FILELINEARPRIO"));
			}
			//MORPH END   - Added by SiRoB, Khaos Category

			bool bToolbarItem = !thePrefs.IsDownloadToolbarEnabled();
			if (bToolbarItem)
			{
				m_FileMenu.AppendMenu(MF_SEPARATOR);
				m_FileMenu.AppendMenu(MF_STRING, MP_TOGGLEDTOOLBAR, GetResString(IDS_SHOWTOOLBAR));
			}

			GetPopupMenuPos(*this, point);
			m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			if (bToolbarItem)
			{
				VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
				VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			}
			//MORPH START - Added by SiRoB, Khaos Category
			m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount()-1,MF_BYPOSITION);
			if (mnuOrder) VERIFY( mnuOrder.DestroyMenu() );
			//MORPH END   - Added by SiRoB, Khaos Category

			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			if (iPreviewMenuEntries && thePrefs.IsExtControlsEnabled() && !thePrefs.GetExtraPreviewWithMenu())
				VERIFY( m_PreviewMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
			else if (iPreviewMenuEntries)
				VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
			VERIFY( WebMenu.DestroyMenu() );
			VERIFY( CatsMenu.DestroyMenu() );
			VERIFY( PreviewWithMenu.DestroyMenu() );
		}
		else{
			const CUpDownClient* client = (CUpDownClient*)content->value;
			CTitleMenu ClientMenu;
			ClientMenu.CreatePopupMenu();
			ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
			ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
			ClientMenu.SetDefaultItem(MP_DETAIL);
			if(!thePrefs.IsLessControls()){ //MORPH show less controls
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
			//MORPH START - Added by SiRoB, Friend Addon
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsFriend()) ? MF_ENABLED  | ((!client->HasLowID() && client->IsFriend() && client->GetFriendSlot())?MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
			//MORPH END - Added by SiRoB, Friend Addon
			} //MORPH show less controls
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
			if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
				ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
			ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

			CMenu A4AFMenu;
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
			}
			
			//MORPH START show less controls
			if(!thePrefs.IsLessControls())
			{
				//MORPH START - Added by Yun.SF3, List Requested Files
				ClientMenu.AppendMenu(MF_SEPARATOR);
				ClientMenu.AppendMenu(MF_STRING,MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); // Added by sivka
				//MORPH END - Added by Yun.SF3, List Requested Files
			}
			//MORPH END show less controls

			GetPopupMenuPos(*this, point);
			ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			
			VERIFY( A4AFMenu.DestroyMenu() );
			VERIFY( ClientMenu.DestroyMenu() );
		}
	}
	else{	// nothing selected
		int total;
		m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CANCEL, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);

		if (thePrefs.IsExtControlsEnabled()) {
			if (!thePrefs.GetPreviewPrio())
			{
				m_PreviewMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_GRAYED);
				m_PreviewMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_UNCHECKED);
			}
			m_PreviewMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
			m_PreviewMenu.EnableMenuItem(MP_PAUSEONPREVIEW, MF_GRAYED);
        }
		else {
			m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
		}
		//EastShare Start - Added by AndCycle, Only download complete files v2.1 (shadow)
		m_FileMenu.EnableMenuItem(MP_FORCE,MF_GRAYED);//shadow#(onlydownloadcompletefiles)
		//EastShare End - Added by AndCycle, Only download complete files v2.1 (shadow)
		
		if(!thePrefs.IsLessControls()) //MORPH show less controls
			//MORPH START - Added by SiRoB, ShowPermissions
			m_FileMenu.EnableMenuItem((UINT_PTR)m_PermMenu.m_hMenu, MF_GRAYED);
			//MORPH END   - Added by SiRoB, ShowPermissions
		//MORPH START - Added by SiRoB, copy feedback feature
		m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK_US, MF_GRAYED);
		//MORPH END   - Added by SiRoB, copy feedback feature
		if(!thePrefs.IsLessControls()) //MORPH show less controls
			m_FileMenu.EnableMenuItem(MP_MASSRENAME,MF_GRAYED);//Commander - Added: MassRename
		m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
		//MORPH START show less controls
		if(!thePrefs.IsLessControls())
		{
			//MORPH START - Added by SiRoB, Import Parts [SR13]
			m_FileMenu.EnableMenuItem(MP_SR13_ImportParts,MF_GRAYED);
			//m_FileMenu.EnableMenuItem(MP_SR13_InitiateRehash,MF_GRAYED);
			//MORPH END   - Added by SiRoB, Import Parts [SR13]
			m_FileMenu.EnableMenuItem(MP_FOLLOW_THE_MAJORITY,MF_GRAYED); //EastShare - FollowTheMajority by AndCycle
		}
		//MORPH END show less controls
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab,total) > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);
		if (m_SourcesMenu)
			m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

		// also show the "Web Services" entry, even if its disabled and therefore not useable, it though looks a little 
		// less confusing this way.
		CTitleMenu WebMenu;
		WebMenu.CreateMenu();
		WebMenu.AddMenuTitle(NULL, true);
		theWebServices.GetFileMenuEntries(&WebMenu);
		m_FileMenu.AppendMenu(MF_POPUP | MF_GRAYED, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

		bool bToolbarItem = !thePrefs.IsDownloadToolbarEnabled();
		if (bToolbarItem)
		{
			m_FileMenu.AppendMenu(MF_SEPARATOR);
			m_FileMenu.AppendMenu(MF_STRING, MP_TOGGLEDTOOLBAR, GetResString(IDS_SHOWTOOLBAR));
		}

		GetPopupMenuPos(*this, point);
		m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		if (bToolbarItem)
		{
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
		}
		m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
		VERIFY( WebMenu.DestroyMenu() );
	}
}

void CDownloadListCtrl::FillCatsMenu(CMenu& rCatsMenu, int iFilesInCats)
{
	ASSERT(rCatsMenu.m_hMenu);
	if (iFilesInCats == (-1))
	{
		iFilesInCats = 0;
		int iSel = GetNextItem(-1, LVIS_SELECTED);
		if (iSel != -1)
		{
			const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
			if (content != NULL && content->type == FILE_TYPE)
			{
				POSITION pos = GetFirstSelectedItemPosition();
				while (pos)
				{
					const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
					if (pItemData == NULL || pItemData->type != FILE_TYPE)
						continue;
					const CPartFile* pFile = (CPartFile*)pItemData->value;
					iFilesInCats += (!pFile->HasDefaultCategory()) ? 1 : 0; 
				}
			}
		}
	}
	rCatsMenu.AppendMenu(MF_STRING, MP_NEWCAT, GetResString(IDS_NEW) + _T("..."));	
	//MORPH START - Changed By SiRoB, Khaos Category
	// We don't use the Unassign menu item, we use the default cat
	/*
	CString label = GetResString(IDS_CAT_UNASSIGN);
	label.Remove('(');
	label.Remove(')'); // Remove brackets without having to put a new/changed ressource string in
	rCatsMenu.AppendMenu(MF_STRING | ((iFilesInCats == 0) ? MF_GRAYED : MF_ENABLED), MP_ASSIGNCAT, label);
	*/
	// MORPH END   - Changed By SiRoB, Khaos Category
	if (thePrefs.GetCatCount() > 1)
	{
		rCatsMenu.AppendMenu(MF_SEPARATOR);
		//MORPH START - Changed By SiRoB, Khaos Category
		/*
		for (int i = 1; i < thePrefs.GetCatCount(); i++){
		*/
		CString label;
		for (int i = 0; i < thePrefs.GetCatCount(); i++){
		//MORPH END   - Changed By SiRoB, Khaos Category
			label = thePrefs.GetCategory(i)->strTitle;
			label.Replace(_T("&"), _T("&&") );
			rCatsMenu.AppendMenu(MF_STRING, MP_ASSIGNCAT + i, label);
		}
	}
}

CTitleMenu* CDownloadListCtrl::GetPrioMenu()
{
	UINT uPrioMenuItem = 0;
	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content != NULL && content->type == FILE_TYPE)
		{
			bool bFirstItem = true;	
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData == NULL || pItemData->type != FILE_TYPE)
					continue;
				const CPartFile* pFile = (CPartFile*)pItemData->value;
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
				{
					uPrioMenuItem = 0;
					break;
				}
				bFirstItem = false;
			}
		}
	}
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	return &m_PrioMenu;
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
		case MP_TOGGLEDTOOLBAR:
			thePrefs.SetDownloadToolbar(true);
			theApp.emuledlg->transferwnd->ShowToolbar(true);
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel == -1)
		iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content != NULL && content->type == FILE_TYPE)
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
									case PS_WAITINGFORHASH:
									case PS_HASHING:
									case PS_COMPLETING:
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									case PS_COMPLETE:
										if (wParam == MP_CANCEL){
											bool delsucc = ShellDeleteFile(selectedList.GetHead()->GetFilePath());
											if (delsucc){
												theApp.sharedfiles->RemoveFile(selectedList.GetHead(), true);
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
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									default:
										if (selectedList.GetHead()->GetCategory())
											theApp.downloadqueue->StartNextFileIfPrefs(selectedList.GetHead()->GetCategory());
										selectedList.GetHead()->DeleteFile();
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
				case MP_PRIOHIGH:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIOLOW:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_LOW);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIONORMAL:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_NORMAL);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_PRIOAUTO:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(true);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
			//Commander - Added: MassRename [Dragon] - Start
			// Mighty Knife: Mass Rename
			case MP_MASSRENAME: {
				CMassRenameDialog MRDialog;
				// Add the files to the dialog
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL) {
					CPartFile*  file = selectedList.GetAt (pos);
					MRDialog.m_FileList.AddTail (file);
					selectedList.GetNext (pos);
				}
				int result = MRDialog.DoModal ();
				if (result == IDOK) {
					// The user has successfully entered new filenames. Now we have
					// to rename all the files...
					POSITION pos = selectedList.GetHeadPosition();
					int i=0;
					while (pos != NULL) {
						CString newname = MRDialog.m_NewFilenames.at (i);
						CString newpath = MRDialog.m_NewFilePaths.at (i);
						CPartFile* file = selectedList.GetAt (pos);
						// .part files could be renamed by simply changing the filename
						// in the CKnownFile object.
						if ((!file->IsPartFile()) && (_trename(file->GetFilePath(), newpath) != 0)){
							// Use the "Format"-Syntax of AddLogLine here instead of
							// CString.Format+AddLogLine, because if "%"-characters are
							// in the string they would be misinterpreted as control sequences!
							AddLogLine(false,_T("Failed to rename '%s' to '%s', Error: %hs"), file->GetFilePath(), newpath, _tcserror(errno));
						} else {
							CString strres;
							if (!file->IsPartFile()) {
								// Use the "Format"-Syntax of AddLogLine here instead of
								// CString.Format+AddLogLine, because if "%"-characters are
								// in the string they would be misinterpreted as control sequences!
								AddLogLine(false,_T("Successfully renamed '%s' to '%s'"), file->GetFilePath(), newpath);
								file->SetFollowTheMajority(false); // EastShare       - FollowTheMajority by AndCycle
								file->SetFileName(newname);
								file->SetFilePath(newpath);
								file->SetFullName(newpath);
							} else {
								// Use the "Format"-Syntax of AddLogLine here instead of
								// CString.Format+AddLogLine, because if "%"-characters are
								// in the string they would be misinterpreted as control sequences!
								AddLogLine(false,_T("Successfully renamed .part file '%s' to '%s'"), file->GetFileName(), newname);
								file->SetFollowTheMajority(false); // EastShare       - FollowTheMajority by AndCycle
								file->SetFileName(newname, true);
								file->SetFilePath(newpath);
								file->SavePartFile(); 
							}
						}

						// Next item
						selectedList.GetNext (pos);
						i++;
					}
				}
				break;
			}
			// [end] Mighty Knife
			//Commander - Added: MassRename [Dragon] - End
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
				//EastShare Start - Added by AndCycle, Only download complete files v2.1 (shadow)
				case MP_FORCE:
					SetRedraw(false);
					while(!selectedList.IsEmpty()) {
						CPartFile* partfile = selectedList.GetHead();
						partfile->lastseencomplete = CTime::GetCurrentTime();
						partfile->SavePartFile();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				//EastShare End - Added by AndCycle, Only download complete files v2.1 (shadow)
				// EastShare Start - FollowTheMajority by AndCycle
				case MP_FOLLOW_THE_MAJORITY:
					SetRedraw(false);
					while(!selectedList.IsEmpty()) {
						CPartFile* partfile = selectedList.GetHead();
						partfile->InvertFollowTheMajority();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				// EastShare End   - FollowTheMajority by AndCycle
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
									partfile->SetFollowTheMajority(false); // EastShare       - FollowTheMajority by AndCycle
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
								file->SetFollowTheMajority(false); // EastShare       - FollowTheMajority by AndCycle
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
						str += ((CAbstractFile*)selectedList.GetHead())->GetED2kLink();
						selectedList.RemoveHead();
					}
					theApp.CopyTextToClipboard(str);
					break;
				}
				case MP_SEARCHRELATED:
					theApp.emuledlg->searchwnd->SearchRelatedFiles(selectedList);
					theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
					break;
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
				case MP_PAUSEONPREVIEW:
				{
					bool bAllPausedOnPreview = true;
					for (pos = selectedList.GetHeadPosition(); pos != 0; )
						bAllPausedOnPreview = ((CPartFile*)selectedList.GetNext(pos))->IsPausingOnPreview() && bAllPausedOnPreview;
					while (!selectedList.IsEmpty()){
						CPartFile* pPartFile = selectedList.RemoveHead();
						if (pPartFile->IsPreviewableFileType() && !pPartFile->IsReadyForPreview())
							pPartFile->SetPauseOnPreview(!bAllPausedOnPreview);
						
					}					
					break;
				}		
				case MP_VIEWFILECOMMENTS:
					ShowFileDialog(IDD_COMMENTLST);
					break;
				//MORPH START - Added by SiRoB, Import Parts [SR13]
				case MP_SR13_ImportParts:
					file->SR13_ImportParts();
					break;
				/*
				case MP_SR13_InitiateRehash:
					SR13_InitiateRehash(file);
					break;
				*/
				//MORPH END   - Added by SiRoB, Import Parts [SR13]
				case MP_SHOWED2KLINK:
					ShowFileDialog(IDD_ED2KLINK);
					break;
				case MP_SETSOURCELIMIT: {
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
				}
				case MP_ADDSOURCE: {
					if (selectedCount > 1)
						break;
					CAddSourceDlg as;
					as.SetFile(file);
					as.DoModal();
					break;
				}
				// xMule_MOD: showSharePermissions
				case MP_PERMDEFAULT:
				case MP_PERMNONE:
				case MP_PERMFRIENDS:
				// Mighty Knife: Community visible filelist
				case MP_PERMCOMMUNITY:
				// [end] Mighty Knife
				case MP_PERMALL: {
					while(!selectedList.IsEmpty()) { 
						CPartFile *file = selectedList.GetHead();
						switch (wParam)
						{
							case MP_PERMDEFAULT:
								file->SetPermissions(-1);
								break;
							case MP_PERMNONE:
								file->SetPermissions(PERM_NOONE);
								break;
							case MP_PERMFRIENDS:
								file->SetPermissions(PERM_FRIENDS);
								break;
							// Mighty Knife: Community visible filelist
							case MP_PERMCOMMUNITY:
								file->SetPermissions(PERM_COMMUNITY);
								break;
							// [end] Mighty Knife
							default : // case MP_PERMALL:
								file->SetPermissions(PERM_ALL);
								break;
						}
						selectedList.RemoveHead();
					}
					Invalidate();
					break;
				}
				// xMule_MOD: showSharePermissions
 				//MORPH START - Added by IceCream, copy feedback feature
 				case MP_COPYFEEDBACK:
				{
					CString feed;
					feed.AppendFormat(GetResString(IDS_FEEDBACK_FROM), thePrefs.GetUserNick(), theApp.m_strModLongVersion);
					feed.Append(_T(" \r\n"));
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{			
						CKnownFile* file = selectedList.GetNext(pos);
						feed.Append(file->GetFeedback());
						feed.Append(_T("\r\n"));
					}
					//Todo: copy all the comments too
					theApp.CopyTextToClipboard(feed);
					break;
				}
				case MP_COPYFEEDBACK_US:
				{
					CString feed;
					feed.AppendFormat(_T("Feedback from %s on [%s]\r\n"),thePrefs.GetUserNick(),theApp.m_strModLongVersion);
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						feed.Append(file->GetFeedback(true));
						feed.Append(_T("\r\n"));
					}
					//Todo: copy all the comments too
					theApp.CopyTextToClipboard(feed);
					break;
				}
				//MORPH END - Added by IceCream, copy feedback feature
				// khaos::categorymod+
				// This is only called when there is a single selection, so we'll handle it thusly.
				case MP_CAT_SETRESUMEORDER: {
					InputBox	inputOrder;
					CString		currOrder;

					currOrder.Format(_T("%u"), file->GetCatResumeOrder());
					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_ORDER), currOrder);
					inputOrder.SetNumber(true);
					if (inputOrder.DoModal() == IDOK)
					{
					int newOrder = inputOrder.GetInputInt();
						if  (newOrder < 0 || newOrder == (int)file->GetCatResumeOrder()) break;

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
						
					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_EXPAUTOINC), _T("1"));
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
						currInstructions.Format(_T("%s %s"), GetResString(IDS_CAT_EXPSTEPTHRU), currFile);
						inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), currInstructions, currOrder);

						if (inputOrder.DoModal() == IDCANCEL) {
							if (MessageBox(GetResString(IDS_CAT_ABORTSTEPTHRU), GetResString(IDS_ABORT), MB_YESNO) == IDYES) {
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

					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_EXPALLSAME), _T("0"));
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
				case MP_FORCEA4AFONFLAG: {
					while (!selectedList.IsEmpty()) {
						CPartFile* cur_file = (CPartFile*)selectedList.RemoveHead();
						cur_file->SetForceAllA4AF(cur_file->ForceAllA4AF() ? false : true);
						if (cur_file->ForceAllA4AF())
							cur_file->SetForceA4AFOff(false);
					}
					break;
				}
				case MP_FORCEA4AFOFFFLAG: {
					while (!selectedList.IsEmpty()) {
						CPartFile* cur_file = (CPartFile*)selectedList.RemoveHead();
						cur_file->SetForceA4AFOff(cur_file->ForceA4AFOff() ? false : true);
						if (cur_file->ForceA4AFOff())
							cur_file->SetForceAllA4AF(false);
					}
					break;
				}
				// khaos::kmod-
				default:
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){
						theWebServices.RunURL(file, wParam);
					}
					else if ((wParam >= MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99) || wParam == MP_NEWCAT){
						int nCatNumber;
						if (wParam == MP_NEWCAT)
						{
							nCatNumber = theApp.emuledlg->transferwnd->AddCategoryInteractive();
							if (nCatNumber == 0) // Creation canceled
								break;
						}
						else
							nCatNumber = wParam - MP_ASSIGNCAT;
						SetRedraw(FALSE);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetCategory(nCatNumber);
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

			switch (wParam){
				case MP_SHOWLIST:
					client->RequestSharedFileList();
					break;
				//MORPH START - Added by SIRoB, Friend Addon
				//Xman friendhandling
				case MP_FRIENDSLOT:{
					//MORPH START - Modified by SIRoB, Added by Yun.SF3, ZZ Upload System
					if (client){
						bool IsAlready;
						IsAlready = client->GetFriendSlot();
						//theApp.friendlist->RemoveAllFriendSlots();
						if( IsAlready ) {
							client->SetFriendSlot(false);
						} else {
							client->SetFriendSlot(true);
						}
						theApp.friendlist->ShowFriends();
						UpdateItem(client);
					}
					//MORPH END - Modified by SIRoB, Added by Yun.SF3, ZZ Upload System
					break;
				}
				case MP_REMOVEFRIEND:{//LSD
					if (client && client->IsFriend())
					{					
						theApp.friendlist->RemoveFriend(client->m_Friend);
						UpdateItem(client);
					}
					break;
				}
				//Xman end
				//MORPH END   - Added by SIRoB, Friend Addon
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
				case MP_BOOT:
					if (client->GetKadPort() && client->GetKadVersion() > 1)
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort());
					break;
// ZZ:DownloadManager -->
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
// <-- ZZ:DownloadManager
				//MORPH START - Added by Yun.SF3, List Requested Files
				case MP_LIST_REQUESTED_FILES: { // added by sivka
					if (client != NULL)
						client->ShowRequestedFiles(); //Changed by SiRoB
					break;
				}
				//MORPH END - Added by Yun.SF3, List Requested Files
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
	m_availableCommandsDirty = true;
	return TRUE;
}

void CDownloadListCtrl::OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;
	bool sortAscending;
	// SLUGFILLER: DLsortFix Ctrl sorts sources only
	/*
	if (GetSortItem() != pNMListView->iSubItem)
	*/
	// I reuse int adder because it is a lot more convenient at this time.
	int adder = (GetAsyncKeyState(VK_CONTROL) < 0) ? 64:0;
	if (GetSortItem() != pNMListView->iSubItem + adder)
	// SLUGFILLER: DLsortFix Ctrl sorts sources only
	{
		//MORPH START - Keep old sorting default behavior (Only first column ascending)
		/*
		switch (pNMListView->iSubItem)
		{
			case 2: // Transferred
			case 3: // Completed
			case 4: // Download rate
			case 5: // Progress
			case 6: // Sources / Client Software
				sortAscending = false;
				break;
			case 9:
				// Keep the current 'm_bRemainSort' for that column, but reset to 'ascending'
				sortAscending = true;
				break;
			default:
				sortAscending = true;
				break;
		}
		*/
		switch (pNMListView->iSubItem)
		{
			case 1: // Filename/ Username
			case 8: // Status
				sortAscending = true;
				break;

			default:
				sortAscending = false;
				break;
		}
		//MORPH END   - Keep old sorting default behavior (Only first column ascending)
	}
	else
		sortAscending = !GetSortAscending();

	// Ornis 4-way-sorting
	//MORPH START - Removed by SiRoB, Remain time and size Columns have been splited
	/*
	int adder = 0;
	if (pNMListView->iSubItem == 9)
	{
		if (GetSortItem() == 9 && sortAscending) // check for 'ascending' because the initial sort order is also 'ascending'
			m_bRemainSort = !m_bRemainSort;
		adder = !m_bRemainSort ? 0 : 81;
	}

	// Sort table
	if (adder == 0)
		SetSortArrow(pNMListView->iSubItem, sortAscending);
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);
	*/
	SetSortArrow(pNMListView->iSubItem + adder, sortAscending); // we need to know about user only or not here
	//MORPH END   - Removed by SiRoB, Remain time and size Columns have been splited
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 100) + adder);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 100) + adder);

	// Save new preferences
	//MORPH - Removed by SiRoB, Remain time and size Columns have been splited
	/*
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);
	*/
	//MORPH - Removed by SiRoB, Remain time and size Columns have been splited

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CtrlItem_Struct *item1 = (CtrlItem_Struct *)lParam1;
	const CtrlItem_Struct *item2 = (CtrlItem_Struct *)lParam2;

	int dwOrgSort = lParamSort;
	int sortMod = 1;
	if (lParamSort >= 100)
	{
		sortMod = -1;
		lParamSort -= 100;
	}

	int comp;
	if (item1->type == FILE_TYPE && item2->type != FILE_TYPE)
	{
		if (item1->value == item2->parent->value)
			return -1;
		comp = Compare((const CPartFile *)item1->value, (const CPartFile *)(item2->parent->value), lParamSort);
	}
	else if (item2->type == FILE_TYPE && item1->type != FILE_TYPE)
	{
		if (item1->parent->value == item2->value)
			return 1;
		comp = Compare((const CPartFile *)(item1->parent->value), (const CPartFile *)item2->value, lParamSort);
	}
	else if (item1->type == FILE_TYPE)
	{
		const CPartFile *file1 = (const CPartFile *)item1->value;
		const CPartFile *file2 = (const CPartFile *)item2->value;
		comp = Compare(file1, file2, lParamSort);
	}
	else
	{
		if (item1->parent->value!=item2->parent->value)
		// SLUGFILLER: DLsortFix - preserve recursion
		/*
		{
			comp = Compare((const CPartFile *)(item1->parent->value), (const CPartFile *)(item2->parent->value), lParamSort);
			return sortMod * comp;
		}
		*/
			return SortProc((LPARAM)item1->parent, (LPARAM)item2->parent, (LPARAM)dwOrgSort);
		// SLUGFILLER: DLsortFix - preserve recursion
		if (item1->type != item2->type)
			return item1->type - item2->type;

		const CUpDownClient *client1 = (const CUpDownClient *)item1->value;
		const CUpDownClient *client2 = (const CUpDownClient *)item2->value;
		//MORPH - Keep A4AF Infos
		/*
		comp = Compare(client1, client2, lParamSort);
		*/
		comp = Compare(client1, client2, lParamSort, item1->owner);
		//MORPH - Keep A4AF Infos
	}

	// SLUGFILLER: DLsortFix - last-chance sort, detect and use
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (comp == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetDownloadList()->GetNextSortOrder(dwOrgSort)) != -1)
		return SortProc(lParam1, lParam2, dwNextSort);
	*/
	if (comp == 0 && dwOrgSort != 99 && theApp.emuledlg->transferwnd->GetDownloadList()->GetNextSortOrder(dwOrgSort) == -1)
		return SortProc(lParam1, lParam2, 99);	// Perhaps a bit hackity, but we only have one last-chance, and this beats saving it in the preferences
	// SLUGFILLER: DLsortFix
	
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
			if(file->IsPartFile() == false && (file->CheckShowItemInGivenCat(incat) || incat==-1) ){
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

void CDownloadListCtrl::OnListModified(NMHDR *pNMHDR, LRESULT * /*pResult*/)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - (int)notFirst, pNMListView->iItem + (int)notLast);
	m_availableCommandsDirty = true;
}

int CDownloadListCtrl::Compare(const CPartFile *file1, const CPartFile *file2, LPARAM lParamSort)
{
	// SLUGFILLER: DLsortFix - client-only sorting
	if (lParamSort > 63 && lParamSort != 99)
		return 0;
	// SLUGFILLER: DLsortFix
	int comp = 0;
	switch (lParamSort)
	{
		case 0: //filename asc
			comp = CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			break;
		
		case 1: //size asc
			comp = CompareUnsigned64(file1->GetFileSize(), file2->GetFileSize());
			break;
		
		case 2: //transferred asc
			comp = CompareUnsigned64(file1->GetTransferred(), file2->GetTransferred());
			break;
		
		case 3: //completed asc
			comp = CompareUnsigned64(file1->GetCompletedSize(), file2->GetCompletedSize());
			break;
		
		case 4: //speed asc
			comp = CompareUnsigned(file1->GetDatarate(), file2->GetDatarate());
			break;
		
		case 5: //progress asc
			comp = CompareFloat(file1->GetPercentCompleted(), file2->GetPercentCompleted());
			break;
		
		case 6: //sources asc
			comp = CompareUnsigned(file1->GetSourceCount(), file2->GetSourceCount());
			break;
		
		case 7: //priority asc
			comp = CompareUnsigned(file1->GetDownPriority(), file2->GetDownPriority());
			break;
		
		case 8: //Status asc 
			comp = CompareUnsigned(file1->getPartfileStatusRang(), file2->getPartfileStatusRang());
			break;
		
		case 9: //Remaining Time asc
		{
			//Make ascending sort so we can have the smaller remaining time on the top 
			//instead of unknowns so we can see which files are about to finish better..
			//MORPH - Changed by SiRoB, Sort by AverageRemainingTime
			/*
			time_t f1 = file1->getTimeRemaining();
			time_t f2 = file2->getTimeRemaining();
			*/
			time_t f1 = (thePrefs.GetTimeRemainingMode()!=2)?file1->getTimeRemaining():file1->GetTimeRemainingAvg();
			time_t f2 = (thePrefs.GetTimeRemainingMode()!=2)?file2->getTimeRemaining():file2->GetTimeRemainingAvg();
			//MORPH - Changed by SiRoB, Sort by AverageRemainingTime
			//Same, do nothing.
			if (f1 == f2) {
				comp = 0;
				break;
			}

			//If descending, put first on top as it is unknown
			//If ascending, put first on bottom as it is unknown
			if (f1 == -1) {
				comp = 1;
				break;
			}

			//If descending, put second on top as it is unknown
			//If ascending, put second on bottom as it is unknown
			if (f2 == -1) {
				comp = -1;
				break;
			}

			//If descending, put first on top as it is bigger.
			//If ascending, put first on bottom as it is bigger.
			comp = CompareUnsigned(f1, f2);
			break;
		}

		//MORPH - Removed by SiRoB, Remain time and size Columns have been splited
		/*
		case 90: //Remaining SIZE asc
			comp = CompareUnsigned64(file1->GetFileSize() - file1->GetCompletedSize(), file2->GetFileSize() - file2->GetCompletedSize());
			break;
		*/
		//MORPH - Removed by SiRoB, Remain time and size Columns have been splited
		
		case 10: //last seen complete asc
			if (file1->lastseencomplete > file2->lastseencomplete)
				comp = 1;
			else if (file1->lastseencomplete < file2->lastseencomplete)
				comp = -1;
			break;
		
		case 11: //last received Time asc
			if (file1->GetFileDate() > file2->GetFileDate())
				comp = 1;
			else if(file1->GetFileDate() < file2->GetFileDate())
				comp = -1;
			break;

		// khaos::categorymod+
		/*
		case 12:
			//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
			comp = CompareLocaleStringNoCase((const_cast<CPartFile*>(file1)->GetCategory() != 0) ? thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->strTitle:GetResString(IDS_ALL),
											 (const_cast<CPartFile*>(file2)->GetCategory() != 0) ? thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->strTitle:GetResString(IDS_ALL) );
			break;

		case 13: // addeed on asc
			if (file1->GetCrCFileDate() > file2->GetCrCFileDate())
				comp = 1;
			else if(file1->GetCrCFileDate() < file2->GetCrCFileDate())
				comp = -1;
			break;
		*/
		case 12: // addeed on asc
			if (file1->GetCrCFileDate() > file2->GetCrCFileDate())
				comp = 1;
			else if(file1->GetCrCFileDate() < file2->GetCrCFileDate())
				comp = -1;
			break;

		case 13: // Cat
			if (file1->GetCategory() > file2->GetCategory())
				comp=1;
			else if (file1->GetCategory() < file2->GetCategory())
				comp=-1;
			else
				comp=0;
			break;
		case 14: // Mod
			if (CPartFile::RightFileHasHigherPrio(file2, file1))
				comp=1;
			else if (CPartFile::RightFileHasHigherPrio(file1, file2))
				comp=-1;
			else
				comp=0;
			break;
		case 15: // Remaining Bytes
			if ((file1->GetFileSize() - file1->GetCompletedSize()) > (file2->GetFileSize() - file2->GetCompletedSize()))
				comp=1;
			else if ((file1->GetFileSize() - file1->GetCompletedSize()) < (file2->GetFileSize() - file2->GetCompletedSize()))
				comp=-1;
			else
				comp=0;
			break;
		// khaos::categorymod-
		//MORPH START - IP2Country
		case 16:
			comp=0;
		//MORPH END   - IP2Country
		// SLUGFILLER: DLsortFix
		case 99:	// met file name asc, only available as last-resort sort to make sure no two files are equal
			comp=CompareLocaleStringNoCase(file1->GetFullName(), file2->GetFullName());
			break;
		// SLUGFILLER: DLsortFix
	}
	return comp;
}

//MORPH - Keep A4AF Infos
/*
int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort)
*/
int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort, const CPartFile* pfile)
{
	lParamSort &= 63;	// SLUGFILLER: DLsortFix
	switch (lParamSort)
	{
		case 0: //name asc
			if (client1->GetUserName() && client2->GetUserName())
				return CompareLocaleStringNoCase(client1->GetUserName(), client2->GetUserName());
			else if (client1->GetUserName() == NULL)
				return 1; // place clients with no usernames at bottom
			else if (client2->GetUserName() == NULL)
				return -1; // place clients with no usernames at bottom
			return 0;

		case 1: //size but we use status asc
			return client1->GetSourceFrom() - client2->GetSourceFrom();

		case 2://transferred asc
			//MORPH START - Added By SiRoB, Download/Upload
			if (!client1->Credits())
				return 1;
			else if (!client2->Credits())
				return -1;
			return CompareUnsigned64(client2->Credits()->GetDownloadedTotal(), client1->Credits()->GetDownloadedTotal());
			//MORPH END - Added By SiRoB, Download/Upload
		case 3://completed asc
			//MORPH START - Display Chunk Detail
			/*
			return CompareUnsigned(client1->GetTransferredDown(), client2->GetTransferredDown());
			*/
			if (client1->GetDownloadState() == DS_DOWNLOADING && client2->GetDownloadState() == DS_DOWNLOADING)
			{
				if (client1->GetDownChunkProgressPercent() == client2->GetDownChunkProgressPercent())
					return 0;
				else
					return (client1->GetDownChunkProgressPercent() > client2->GetDownChunkProgressPercent()?1:-1);
			}
			else
				return CompareUnsigned(client1->GetSessionPayloadDown(), client2->GetSessionPayloadDown());
			//MORPH END - Display Chunk Detail

		case 4: //speed asc
			return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());

		case 5: //progress asc
			//MORPH - Keep A4AF Infos
			/*
			return CompareUnsigned(client1->GetAvailablePartCount(), client2->GetAvailablePartCount());
			*/
			return CompareUnsigned(client1->GetAvailablePartCount(pfile), client2->GetAvailablePartCount(pfile));
			//MORPH - Keep A4AF Infos

		case 6:
 			//MORPH - Added by Yun.SF3, Maella -Support for tag ET_MOD_VERSION 0x55 II-
			// Maella -Support for tag ET_MOD_VERSION 0x55-
			/*
			if (client1->GetClientSoft() == client2->GetClientSoft())
				return client1->GetVersion() - client2->GetVersion();
			return -(client1->GetClientSoft() - client2->GetClientSoft()); // invert result to place eMule's at top
			*/
			if (client1->GetClientSoft() == client2->GetClientSoft())
			{
				if(client2->GetVersion() == client1->GetVersion() && client1->GetClientSoft() == SO_EMULE)
					return CompareOptLocaleStringNoCase(client2->GetClientSoftVer(), client1->GetClientSoftVer());
				else
					return client1->GetVersion() - client2->GetVersion();
			}
			else
				return -(client1->GetClientSoft() - client2->GetClientSoft());
			// Maella end
			//MORPH - Added by Yun.SF3, Maella -Support for tag ET_MOD_VERSION 0x55 II-
		
		//EastShare START - Modified by TAHO, @@
		/*
		case 7: //qr asc
			if (client1->GetDownloadState() == DS_DOWNLOADING) {
				if (client2->GetDownloadState() == DS_DOWNLOADING)
					return 0;
				else
					return -1;
			}
			else if (client2->GetDownloadState() == DS_DOWNLOADING)
				return 1;

			if (client1->GetRemoteQueueRank() == 0 && client1->GetDownloadState() == DS_ONQUEUE && client1->IsRemoteQueueFull())
				return 1;
			if (client2->GetRemoteQueueRank() == 0 && client2->GetDownloadState() == DS_ONQUEUE && client2->IsRemoteQueueFull())
				return -1;
			if (client1->GetRemoteQueueRank() == 0)
				return 1;
			if (client2->GetRemoteQueueRank() == 0)
				return -1;
			return CompareUnsigned(client1->GetRemoteQueueRank(), client2->GetRemoteQueueRank());

		case 8:
			if (client1->GetDownloadState() == client2->GetDownloadState())
			{
				if (client1->IsRemoteQueueFull() && client2->IsRemoteQueueFull())
					return 0;
				else if (client1->IsRemoteQueueFull())
					return 1;
				else if (client2->IsRemoteQueueFull())
					return -1;
			}
			return client1->GetDownloadState() - client2->GetDownloadState();
		*/
		case 7: {//priority asc
			uint32 lastAskedTime1 = client1->GetLastAskedTime();
			uint32 lastAskedTime2 = client2->GetLastAskedTime();
			if ( lastAskedTime1 != 0){
				if ( lastAskedTime2 != 0){
					if (lastAskedTime1 > lastAskedTime2){
						return 1;
					} else if (lastAskedTime1 < lastAskedTime2) {
						return -1;
					} else {
						return 0;
					}
				}
				return 1;
			}
			else
				return (lastAskedTime2 == 0) ? -1 : 0;
		}
		case 8: {//Status asc
			EDownloadState clientState1 = client1->GetDownloadState();
			EDownloadState clientState2 = client2->GetDownloadState();

			if ( clientState1 == DS_DOWNLOADING ){
				if ( clientState2 == DS_DOWNLOADING) {
					return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());
				}
				return 1;
			} else if ( clientState2 == DS_DOWNLOADING) {
				return -1;
			}

			if ( clientState1 == DS_ONQUEUE ){
				if ( clientState2 == DS_ONQUEUE ) {
					if ( client1->IsRemoteQueueFull() ){
						return (client2->IsRemoteQueueFull()) ? 0 : 1;
					}
					else if ( client2->IsRemoteQueueFull() ){
						return -1;
					}

					if ( client1->GetRemoteQueueRank() ){
						return (client2->GetRemoteQueueRank()) ? CompareUnsigned(client1->GetRemoteQueueRank(), client2->GetRemoteQueueRank()) : -1;
					}
					return (client2->GetRemoteQueueRank()) ? 1 : 0;
				}
				return 1;
			} else if ( clientState2 == DS_ONQUEUE ){
				return -1;
			}

			if ( clientState1 == DS_NONEEDEDPARTS && clientState2 != DS_NONEEDEDPARTS)
				return 1;

			if ( clientState1 == DS_TOOMANYCONNS && clientState2 != DS_TOOMANYCONNS)
				return -1;

			return 0;
		}
		//EastShare END - Modified by TAHO, @@

		//SLAHAM: ADDED Show Downloading Time =>
		case 9: 
			return client1->dwSessionDLTime - client2->dwSessionDLTime;
		//SLAHAM: ADDED Show Downloading Time <=
		//SLAHAM: ADDED Known Since =>
		case 10:
		{
			uint32 known1 = client2->dwThisClientIsKnownSince;
			uint32 known2 = client1->dwThisClientIsKnownSince;
			return known1==known2? 0 : known1<known2? -1 : 1;
		}
		//SLAHAM: ADDED Known Since <=
		//MORPH START - Remaining Available Data
		case 14: //Remain Size
			return CompareUnsigned64(client1->GetRemainingAvailableData(pfile), client2->GetRemainingAvailableData(pfile));
		//MORPH END   - Remaining Available Data
		// Commander - Added: IP2Country column - Start
		case 15:
			if(client1->GetCountryName(true) && client2->GetCountryName(true))
				return CompareLocaleStringNoCase(client1->GetCountryName(true), client2->GetCountryName(true));
			else if(client1->GetCountryName(true))
				return 1;
			else
				return -1;
		// Commander - Added: IP2Country column - End
	}
	return 0;
}

void CDownloadListCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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
								&& pt.x >= sm_iIconOffset + theApp.GetSmallSytemIconSize().cx 
								&& pt.x <= sm_iIconOffset + theApp.GetSmallSytemIconSize().cx + RATING_ICON_WIDTH)
								ShowFileDialog(IDD_COMMENTLST);
							else if (thePrefs.GetPreviewOnIconDblClk()
									 && pt.x >= sm_iIconOffset 
									 && pt.x < sm_iIconOffset + theApp.GetSmallSytemIconSize().cx) {
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
	if (m_PreviewMenu)
		VERIFY( m_PreviewMenu.DestroyMenu() );
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_PermMenu)	VERIFY( m_PermMenu.DestroyMenu() );	// xMule_MOD: showSharePermissions
	//MORPH START - Added by SiRoB, Advanced A4AF Flag derivated from Khaos
	if (m_A4AFMenuFlag)
		VERIFY( m_A4AFMenuFlag.DestroyMenu() );
	//MORPH END   - Added by SiRoB, Advanced A4AF Flag derivated from Khaos
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );

	m_FileMenu.CreatePopupMenu();
	m_FileMenu.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE), true);

	// Add 'Download Priority' sub menu
	//
	m_PrioMenu.CreateMenu();
	m_PrioMenu.AddMenuTitle(NULL, true);
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));

	//MORPH START show less controls
	if(!thePrefs.IsLessControls())
	{
		// xMule_MOD: showSharePermissions
		m_PermMenu.CreateMenu();
		m_PermMenu.AddMenuTitle(NULL, true);
		m_PermMenu.AppendMenu(MF_STRING,MP_PERMDEFAULT,	GetResString(IDS_DEFAULT));
		m_PermMenu.AppendMenu(MF_STRING,MP_PERMNONE,	GetResString(IDS_HIDDEN));
		m_PermMenu.AppendMenu(MF_STRING,MP_PERMFRIENDS,	GetResString(IDS_FSTATUS_FRIENDSONLY));
		// Mighty Knife: Community visible filelist
		m_PermMenu.AppendMenu(MF_STRING,MP_PERMCOMMUNITY,GetResString(IDS_COMMUNITY));
		// [end] Mighty Knife
		m_PermMenu.AppendMenu(MF_STRING,MP_PERMALL,		GetResString(IDS_PW_EVER));
		// xMule_MOD: showSharePermissions
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PermMenu.m_hMenu, GetResString(IDS_PERMISSION), _T("FILEPERMISSION"));	// xMule_MOD: showSharePermissions
	}
	//MORPH END show less controls

	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"), _T("FILEPRIORITY"));

	// Add file commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	m_FileMenu.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));
	m_FileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	m_FileMenu.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), _T("DELETE"));
	//EastShare Start - Added by AndCycle, Only download complete files v2.1 (shadow)
	if(thePrefs.OnlyDownloadCompleteFiles())
		m_FileMenu.AppendMenu(MF_STRING,MP_FORCE, GetResString(IDS_DL_FORCE));//shadow#(onlydownloadcompletefiles)
	//EastShare End - Added by AndCycle, Only download complete files v2.1 (shadow)
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_OPEN, GetResString(IDS_DL_OPEN), _T("OPENFILE"));
	
	// Extended: Submenu with Preview options, Normal: Preview and possibly 'Preview with' item 
	if (thePrefs.IsExtControlsEnabled())
	{
		m_PreviewMenu.CreateMenu();
		m_PreviewMenu.AddMenuTitle(NULL, true);
		m_PreviewMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
    	//MORPH START - Added icon
    	/*
		m_PreviewMenu.AppendMenu(MF_STRING, MP_PAUSEONPREVIEW, GetResString(IDS_PAUSEONPREVIEW));
		if (!thePrefs.GetPreviewPrio())
    		m_PreviewMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS));
    	*/
		m_PreviewMenu.AppendMenu(MF_STRING, MP_PAUSEONPREVIEW, GetResString(IDS_PAUSEONPREVIEW), _T("FILEDOWNLOADPREVIEWPAUSE"));
		if (!thePrefs.GetPreviewPrio())
	    	m_PreviewMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS), _T("FILEDOWNLOADPREVIEWFIRST"));
    	//MORPH END   - Added icon
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PreviewMenu.m_hMenu, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
	}
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
	
	m_FileMenu.AppendMenu(MF_STRING, MP_METINFO, GetResString(IDS_DL_INFO), _T("FILEINFO"));
	m_FileMenu.AppendMenu(MF_STRING, MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), _T("FILECOMMENTS"));
	//MORPH START show less controls
	if(!thePrefs.IsLessControls())
	{
		//MORPH START - Added by SiRoB, Import Parts [SR13]
		m_FileMenu.AppendMenu(MF_STRING,MP_SR13_ImportParts, GetResString(IDS_IMPORTPARTS), _T("FILEIMPORTPARTS"));
 		//m_FileMenu.AppendMenu(MF_STRING,MP_SR13_InitiateRehash, GetResString(IDS_INITIATEREHASH), _T("FILEINITIATEREHASH"));
		//MORPH END   - Added by SiRoB, Import Parts [SR13]
		if (thePrefs.IsExtControlsEnabled()) m_FileMenu.AppendMenu(MF_STRING,MP_MASSRENAME, GetResString(IDS_MR), _T("FILEMASSRENAME"));//Commander - Added: MassRename [Dragon]
		m_FileMenu.AppendMenu(MF_STRING, MP_FOLLOW_THE_MAJORITY, GetResString(IDS_FOLLOW_THE_MAJORITY)); // EastShare       - FollowTheMajority by AndCycle
	}
	//MORPH END show less controls
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR), _T("CLEARCOMPLETE"));

	// Add (extended user mode) 'Source Handling' sub menu
	//
	if (thePrefs.IsExtControlsEnabled()) {
		m_SourcesMenu.CreateMenu();
		//MORPH START - Added icon
		/*
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY));
		*/
		m_SourcesMenu.AddMenuTitle(NULL, true);
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY), _T("FILEADDSRC"));
		//MORPH END   - Added icon
		m_SourcesMenu.AppendMenu(MF_STRING, MP_SETSOURCELIMIT, GetResString(IDS_SETPFSLIMIT));
		//MORPH START - Added by SiRoB, Advanced A4AF Flag derivated from Khaos
		m_A4AFMenuFlag.CreateMenu();
		m_A4AFMenuFlag.AppendMenu(MF_STRING, MP_FORCEA4AFONFLAG, GetResString(IDS_A4AF_ONFLAG));
		m_A4AFMenuFlag.AppendMenu(MF_STRING, MP_FORCEA4AFOFFFLAG, GetResString(IDS_A4AF_OFFFLAG));

		m_SourcesMenu.AppendMenu(MF_SEPARATOR);
		m_SourcesMenu.AppendMenu(MF_STRING, MP_FORCEA4AF, GetResString(IDS_A4AF_FORCEALL));
		m_SourcesMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_A4AFMenuFlag.m_hMenu, GetResString(IDS_A4AF_FLAGS), _T("ADVA4AF"));
		//MORPH END   - Added by SiRoB, Advanced A4AF Flag derivated from Khaos
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_A4AF));
	}
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Add 'Copy & Paste' commands
	//
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_FileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK"));
	m_FileMenu.AppendMenu(MF_STRING, MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD), _T("PASTELINK"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	//MORPH START - Added by IceCream, copy feedback feature
	m_FileMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK, GetResString(IDS_COPYFEEDBACK), _T("COPY"));
	m_FileMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK_US, GetResString(IDS_COPYFEEDBACK_US), _T("COPY"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	//MORPH END   - Added by IceCream, copy feedback feature

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

float CDownloadListCtrl::GetFinishedSize()
{
	float fsize = 0;

	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			CPartFile* file = (CPartFile*)cur_item->value;
			if (file->GetStatus() == PS_COMPLETE) {
				fsize += (uint64)file->GetFileSize();
			}
		}
	}
	return fsize;
}

int CDownloadListCtrl::GetFilesCountInCurCat()
{
	int iCount = 0;
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			CPartFile* file = (CPartFile*)cur_item->value;
			if (file->CheckShowItemInGivenCat(curTab))
				iCount++;
		}
	}
	return iCount;
}

void CDownloadListCtrl::ShowFilesCount()
{
	theApp.emuledlg->transferwnd->UpdateFilesCount(GetFilesCountInCurCat());
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
	if (content != NULL)
	{
		if (content->type == FILE_TYPE)
		{
			CPartFile* file = (CPartFile*)content->value;
			if (thePrefs.ShowRatingIndicator() 
				&& (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()) 
				&& pt.x >= sm_iIconOffset + theApp.GetSmallSytemIconSize().cx 
				&& pt.x <= sm_iIconOffset + theApp.GetSmallSytemIconSize().cx + RATING_ICON_WIDTH)
				ShowFileDialog(IDD_COMMENTLST);
			else
				ShowFileDialog(0);
		}
		else
		{
			ShowClientDialog((CUpDownClient*)content->value);
		}
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
			
			if (!file->CheckShowItemInGivenCat(curTab))
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
			
			if (!file->CheckShowItemInGivenCat(newsel))
				HideFile(file);
			else
				ShowFile(file);
		}
	}

	SetRedraw(TRUE);
	curTab=newsel;
	ShowFilesCount();
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
			InsertItem(LVIF_PARAM | LVIF_TEXT, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)updateItem);
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

void CDownloadListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
		NMLVDISPINFO *pDispInfo = (NMLVDISPINFO *)pNMHDR;
		/*TRACE("CDownloadListCtrl::OnLvnGetDispInfo iItem=%d iSubItem=%d", pDispInfo->item.iItem, pDispInfo->item.iSubItem);
		if (pDispInfo->item.mask & LVIF_TEXT)
			TRACE(" LVIF_TEXT");
		if (pDispInfo->item.mask & LVIF_IMAGE)
			TRACE(" LVIF_IMAGE");
		if (pDispInfo->item.mask & LVIF_STATE)
			TRACE(" LVIF_STATE");
		TRACE("\n");*/
		if (pDispInfo->item.mask & LVIF_TEXT) {
			const CtrlItem_Struct *pCtrlItem = reinterpret_cast<CtrlItem_Struct *>(pDispInfo->item.lParam);
			if (pCtrlItem != NULL && pCtrlItem->value != NULL) {
				if (pCtrlItem->type == FILE_TYPE)
					GetFileItemDisplayText((CPartFile *)pCtrlItem->value, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
				else if (pCtrlItem->type == UNAVAILABLE_SOURCE || pCtrlItem->type == AVAILABLE_SOURCE)
					GetSourceItemDisplayText(pCtrlItem, pDispInfo->item.iSubItem, pDispInfo->item.pszText, pDispInfo->item.cchTextMax);
				else
					ASSERT(0);
			}
		}
	}
    *pResult = 0;
}

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
								//MORPH START - Extra User Infos
								+ GetResString(IDS_CD_CSOFT) + _T(": %s\n")
								+ GetResString(IDS_COUNTRY) + _T(": %s\n")
								//MORPH END   - Extra User Infos
								+ GetResString(IDS_SERVER) + _T(": %s:%u\n\n")
								+ GetResString(IDS_NEXT_REASK) + _T(": %s"),
								client->GetUserName() ? client->GetUserName() : (_T('(') + GetResString(IDS_UNKNOWN) + _T(')')),
								//MORPH START - Extra User Infos
								client->GetClientSoftVer(),
								client->GetCountryName(true),
								//MORPH END   - Extra User Infos
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
						if (!client->GetFileComment().IsEmpty())
							info += _T('\n') + GetResString(IDS_CMT_READ) + _T(' ') + client->GetFileComment();
						if (client->GetFileRating())
							info += _T('\n') + GetResString(IDS_QL_RATING) + _T(':') + GetRateString(client->GetFileRating());
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

void CDownloadListCtrl::ShowFileDialog(UINT uInvokePage)
{
	CSimpleArray<CPartFile*> aFiles;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iItem = GetNextSelectedItem(pos);
		if (iItem != -1)
		{
			const CtrlItem_Struct* pCtrlItem = (CtrlItem_Struct*)GetItemData(iItem);
			if (pCtrlItem != NULL && pCtrlItem->type == FILE_TYPE)
				aFiles.Add((CPartFile*)pCtrlItem->value);
		}
	}

	if (aFiles.GetSize() > 0)
	{
		CDownloadListListCtrlItemWalk::SetItemType(FILE_TYPE);
		CFileDetailDialog dialog(&aFiles, uInvokePage, this);
		dialog.DoModal();
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
				if (ctrl_item != NULL && (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE)))
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
				if (ctrl_item != NULL && (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE)))
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
	CClientDetailDialog dialog(pClient, this);
	dialog.DoModal();
}

CImageList *CDownloadListCtrl::CreateDragImage(int /*iItem*/, LPPOINT lpPoint)
{
	const int iMaxSelectedItems = 30;
	int iSelectedItems = 0;
	CRect rcSelectedItems;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos && iSelectedItems < iMaxSelectedItems)
	{
		int iItem = GetNextSelectedItem(pos);
		const CtrlItem_Struct *pCtrlItem = (CtrlItem_Struct *)GetItemData(iItem);
		if (pCtrlItem != NULL && pCtrlItem && pCtrlItem->type == FILE_TYPE)
		{
			CRect rcLabel;
			GetItemRect(iItem, rcLabel, LVIR_LABEL);
			if (iSelectedItems == 0)
			{
				rcSelectedItems.left = sm_iIconOffset;
				rcSelectedItems.top = rcLabel.top;
				rcSelectedItems.right = rcLabel.right;
				rcSelectedItems.bottom = rcLabel.bottom;
			}
			rcSelectedItems.UnionRect(rcSelectedItems, rcLabel);
			iSelectedItems++;
		}
	}
	if (iSelectedItems == 0)
		return NULL;

	CClientDC dc(this);
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&dc))
		return NULL;

	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&dc, rcSelectedItems.Width(), rcSelectedItems.Height()))
		return NULL;

	CBitmap *pOldBmp = dcMem.SelectObject(&bmpMem);
	CFont *pOldFont = dcMem.SelectObject(GetFont());

	COLORREF crBackground = GetSysColor(COLOR_WINDOW);
	dcMem.FillSolidRect(0, 0, rcSelectedItems.Width(), rcSelectedItems.Height(), crBackground);
	dcMem.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	iSelectedItems = 0;
	pos = GetFirstSelectedItemPosition();
	while (pos && iSelectedItems < iMaxSelectedItems)
	{
		int iItem = GetNextSelectedItem(pos);
		const CtrlItem_Struct *pCtrlItem = (CtrlItem_Struct *)GetItemData(iItem);
		if (pCtrlItem && pCtrlItem->type == FILE_TYPE)
		{
			const CPartFile *pPartFile = (CPartFile *)pCtrlItem->value;
			CRect rcLabel;
			GetItemRect(iItem, rcLabel, LVIR_LABEL);

			CRect rcItem;
			rcItem.left = 0;
			rcItem.top = rcLabel.top - rcSelectedItems.top;
			rcItem.right = rcLabel.right;
			rcItem.bottom = rcItem.top + rcLabel.Height();

			if (theApp.GetSystemImageList())
			{
				int iImage = theApp.GetFileTypeSystemImageIdx(pPartFile->GetFileName());
				ImageList_Draw(theApp.GetSystemImageList(), iImage, dcMem, rcItem.left, rcItem.top, ILD_TRANSPARENT);
			}

			rcItem.left += 16 + sm_iLabelOffset;
			dcMem.DrawText(pPartFile->GetFileName(), -1, rcItem, MLC_DT_TEXT);
			rcItem.left -= 16 + sm_iLabelOffset;

			iSelectedItems++;
		}
	}
	dcMem.SelectObject(pOldBmp);
	dcMem.SelectObject(pOldFont);

	// At this point the bitmap in 'bmpMem' may or may not contain alpha data and we have to take special
	// care about passing such a bitmap further into Windows (GDI). Strange things can happen due to that
	// not all GDI functions can deal with RGBA bitmaps. Thus, create an image list with ILC_COLORDDB.
	CImageList *pimlDrag = new CImageList();
	pimlDrag->Create(rcSelectedItems.Width(), rcSelectedItems.Height(), ILC_COLORDDB | ILC_MASK, 1, 0);
	pimlDrag->Add(&bmpMem, crBackground);
	bmpMem.DeleteObject();

	if (lpPoint)
	{
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);
		lpPoint->x = ptCursor.x - rcSelectedItems.left;
		lpPoint->y = ptCursor.y - rcSelectedItems.top;
	}

	return pimlDrag;
}

bool CDownloadListCtrl::ReportAvailableCommands(CList<int>& liAvailableCommands)
{
	if ((m_dwLastAvailableCommandsCheck > ::GetTickCount() - SEC2MS(3) && !m_availableCommandsDirty))
		return false;
	m_dwLastAvailableCommandsCheck = ::GetTickCount();
	m_availableCommandsDirty = false;

	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content != NULL && content->type == FILE_TYPE)
		{
			// get merged settings
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
			POSITION pos = GetFirstSelectedItemPosition();
			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
				if (pItemData == NULL || pItemData->type != FILE_TYPE)
					continue;
				const CPartFile* pFile = (CPartFile*)pItemData->value;
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
			}


			// enable commands if there is at least one item which can be used for the action
			if (iFilesToCancel > 0)
				liAvailableCommands.AddTail(MP_CANCEL);
			if (iFilesToStop > 0)
				liAvailableCommands.AddTail(MP_STOP);
			if (iFilesToPause > 0)
				liAvailableCommands.AddTail(MP_PAUSE);
			if (iFilesToResume > 0)
				liAvailableCommands.AddTail(MP_RESUME);
			if (iSelectedItems == 1 && iFilesToOpen == 1)
				liAvailableCommands.AddTail(MP_OPEN);
			if (iSelectedItems == 1 && iFilesToPreview == 1)
				liAvailableCommands.AddTail(MP_PREVIEW);
			if (iSelectedItems > 0)
			{
				liAvailableCommands.AddTail(MP_METINFO);
				liAvailableCommands.AddTail(MP_VIEWFILECOMMENTS);
				liAvailableCommands.AddTail(MP_SHOWED2KLINK);
				liAvailableCommands.AddTail(MP_NEWCAT);
				liAvailableCommands.AddTail(MP_PRIOLOW);
				if (theApp.emuledlg->searchwnd->CanSearchRelatedFiles())
					liAvailableCommands.AddTail(MP_SEARCHRELATED);
			}
		}
	}
	int total;
	if (GetCompleteDownloads(curTab, total) > 0)
		liAvailableCommands.AddTail(MP_CLEARCOMPLETED);
	if (GetItemCount() > 0)
		liAvailableCommands.AddTail(MP_FIND);
	return true;
}
