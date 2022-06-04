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
#include "ListenSocket.h" //Xman changed: display the obfuscation icon for all clients which enabled it
#include "MassRename.h" //Xman Mass Rename (Morph)
#include "Log.h" //Xman Mass Rename (Morph)
#include "SR13-ImportParts.h"//MORPH - Added by SiRoB, Import Parts [SR13] - added by zz_fly
#include "SivkaFileSettings.h" // File Settings [sivka/Stulle] - Stulle


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
	SetGeneralPurposeFind(true);
	SetSkinKey(L"DownloadsLv");
	m_dwLastAvailableCommandsCheck = 0;
	m_availableCommandsDirty = true;
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	if (m_PreviewMenu)
		VERIFY( m_PreviewMenu.DestroyMenu() );
	//Xman Xtreme Downloadmanager
	if (m_DropMenu) 
		VERIFY( m_DropMenu.DestroyMenu() );
	//Xman End
	// ==> Follow The Majority [AndCycle/Stulle] - Stulle
	if (m_FollowTheMajorityMenu)
		VERIFY( m_FollowTheMajorityMenu.DestroyMenu() );
	// <== Follow The Majority [AndCycle/Stulle] - Stulle
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
    if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );
	
	while (m_ListItems.empty() == false) {
		delete m_ListItems.begin()->second; // second = CtrlItem_Struct*
		m_ListItems.erase(m_ListItems.begin());
	}
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		delete m_tooltip;
}

void CDownloadListCtrl::Init()
{
	GlobalHardLimit = 0; // show global HL - Stulle

	SetPrefsKey(_T("DownloadListCtrl"));
	SetStyle();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	// workaround running MFC as service
	if (!theApp.IsRunningAsService())
	{
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
		CToolTipCtrl* tooltip = GetToolTips();
		if (tooltip){
			m_tooltip->SetFileIconToolTip(true);
			m_tooltip->SubclassWindow(*tooltip);
			tooltip->ModifyStyle(0, TTS_NOPREFIX);
			tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
			tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
		}
	} // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	InsertColumn(0, GetResString(IDS_DL_FILENAME),		LVCFMT_LEFT,  DFLT_FILENAME_COL_WIDTH);
	InsertColumn(1, GetResString(IDS_DL_SIZE),			LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(2, GetResString(IDS_DL_TRANSF),		LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH,		-1, true);
	InsertColumn(3, GetResString(IDS_DL_TRANSFCOMPL),	LVCFMT_RIGHT, DFLT_SIZE_COL_WIDTH);
	InsertColumn(4, GetResString(IDS_DL_SPEED),			LVCFMT_RIGHT, DFLT_DATARATE_COL_WIDTH);
	InsertColumn(5, GetResString(IDS_DL_PROGRESS),		LVCFMT_LEFT,  DFLT_PARTSTATUS_COL_WIDTH);
	InsertColumn(6, GetResString(IDS_DL_SOURCES),		LVCFMT_RIGHT,  60);
	InsertColumn(7, GetResString(IDS_PRIORITY),			LVCFMT_LEFT,  DFLT_PRIORITY_COL_WIDTH);
	InsertColumn(8, GetResString(IDS_STATUS),			LVCFMT_LEFT,   70);
	InsertColumn(9, GetResString(IDS_DL_REMAINS),		LVCFMT_LEFT,  110);
	CString lsctitle = GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(_T(':'));
	InsertColumn(10, lsctitle,							LVCFMT_LEFT,  150,						-1, true);
	lsctitle = GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(_T(':'));
	InsertColumn(11, lsctitle,							LVCFMT_LEFT,  120,						-1, true);
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	InsertColumn(12, GetResString(IDS_CAT),				LVCFMT_LEFT,  100,						-1, true);
	*/
	InsertColumn(12, GetResString(IDS_CAT_COLCATEGORY),		LVCFMT_LEFT,  100);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	InsertColumn(13, GetResString(IDS_ADDEDON),			LVCFMT_LEFT,  120);
	InsertColumn(14, GetResString(IDS_AVGQR),			LVCFMT_LEFT,  70); //Xman Xtreme-Downloadmanager AVG-QR
	
	InsertColumn(15, GetResString(IDS_CAT_COLORDER),LVCFMT_LEFT,60); // Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	InsertColumn(16, GetResString(IDS_SHOW_DROPPED_SRC) ,LVCFMT_LEFT, 80); // show # of dropped sources - Stulle

	SetAllIcons();
	Localize();
	LoadSettings();
	curTab=0;

	//Xman Show active downloads bold
	/*
	if (thePrefs.GetShowActiveDownloadsBold())
	{
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
	}
	*/
	{
		CFont *pFont = GetFont();
		LOGFONT lfFont = {0};
		pFont->GetLogFont(&lfFont);
		lfFont.lfWeight = FW_BOLD;
		m_fontBold.CreateFontIndirect(&lfFont);
		m_pFontBold = thePrefs.GetUseSystemFontForMainControls() ? &m_fontBold : &theApp.m_fontDefaultBold;
		//Xman client percentage
		lfFont.lfHeight = 11;
		m_fontBoldSmaller.CreateFontIndirect(&lfFont);
		//Xman end		
	}
	//Xman End
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	//Xman narrow font at transferwindow
	CFont* pFont = GetFont();
	LOGFONT lfFont = {0};
	pFont->GetLogFont(&lfFont);
	_tcscpy(lfFont.lfFaceName, _T("Arial Narrow"));
	lfFont.lfWeight = FW_BOLD;
	m_fontNarrowBold.CreateFontIndirect(&lfFont);
	//Xman end
	*/
	// <== Design Settings [eWombat/Stulle] - Stulle

	// Barry - Use preferred sort order from preferences
	m_bRemainSort = thePrefs.TransferlistRemainSortStyle();
	int adder = 0;
	if (GetSortItem() != 9 || !m_bRemainSort)
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending() ? arrowDoubleUp : arrowDoubleDown);
		adder = 81;
	}
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 100) + adder);
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
	//Xman Show correct Icons
	/*
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
	*/
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));	//5
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));		//6
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));		//7
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));		//8
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));		//9
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));	//10
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search")));	//11 // rating for comments are searched on kad
	m_ImageList.Add(CTempIconLoader(_T("LEECHER")));			//12 //Xman Anti-Leecher
	m_ImageList.Add(CTempIconLoader(_T("Server")));
	m_ImageList.Add(CTempIconLoader(_T("ClientDefault")));		//14
	m_ImageList.Add(CTempIconLoader(_T("ClientDefaultPlus")));	//15
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));		//16
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));	//17
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));		//18
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));	//19
	m_ImageList.Add(CTempIconLoader(_T("ClientFriend")));			//20
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendPlus")));		//21
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));		//22
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));	//23
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));	//24
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//25
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));		//26
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));	//27
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));			//28
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));		//29
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));			//30
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));		//31
	//Xman friend visualization
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendSlotOvl"))); //32
	//Xman end
	//Xman end

	// ==> Mod Icons - Stulle
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //33
	m_ImageList.Add(CTempIconLoader(_T("STULLE"))); //34
	m_ImageList.Add(CTempIconLoader(_T("XTREME"))); //35
	m_ImageList.Add(CTempIconLoader(_T("MORPH"))); //36
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE"))); //37
	m_ImageList.Add(CTempIconLoader(_T("EMF"))); //38
	m_ImageList.Add(CTempIconLoader(_T("NEO"))); //39
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO"))); //40
	m_ImageList.Add(CTempIconLoader(_T("XRAY"))); //41
	m_ImageList.Add(CTempIconLoader(_T("MAGIC"))); //42
	// <== Mod Icons - Stulle

	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	// ==> Mod Icons - Stulle
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	// <== Mod Icons - Stulle
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
	// ==> show global HL - Stulle
	if (thePrefs.GetShowGlobalHL())
		strRes.AppendFormat(_T(" [%u]"),GlobalHardLimit);
	// <== show global HL - Stulle
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

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	strRes = GetResString(IDS_CAT);
	*/
	strRes = GetResString(IDS_CAT_COLCATEGORY);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);

	strRes = GetResString(IDS_ADDEDON);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	//Xman Xtreme Downloadmanager AVG-QR
	strRes = GetResString(IDS_AVGQR); 
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
	//Xman End

	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	strRes = GetResString(IDS_CAT_COLORDER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

	// ==> show # of dropped sources - Stulle
	strRes = GetResString(IDS_SHOW_DROPPED_SRC);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);
	// <== show # of dropped sources - Stulle

	CreateMenues();
	ShowFilesCount();

	// ==> Design Settings [eWombat/Stulle] - Stulle
	theApp.emuledlg->transferwnd->SetBackgroundColor(style_b_downloadlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
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
		InsertItem(LVIF_PARAM | LVIF_TEXT, itemnr, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)newitem);

	ShowFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    newitem->owner = owner;
    newitem->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE;
    newitem->value = source;
	newitem->dwUpdated = 0; 
	newitem->dwUpdatedchunk = 0; // Downloading Chunk Detail Display [SiRoB] - Stulle

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
				cur_item->dwUpdatedchunk = 0; // Downloading Chunk Detail Display [SiRoB] - Stulle
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
	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	if (theApp.IsRunningAsService(SVC_LIST_OPT))
		return;
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle

	if (!theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || theApp.emuledlg->transferwnd->GetDownloadList()->IsWindowVisible() == false )
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

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
			updateItem->dwUpdatedchunk = 0; // Downloading Chunk Detail Display [SiRoB] - Stulle
			Update(result);
		}
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
				//Xman // Maella -Accurate measure of bandwidth: IP, TCP or UDP, eDonkey protocol, etc-
				/*
				_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetDatarate(), false, true), cchTextMax);
				*/
				_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetDownloadDatarate(), false, true), cchTextMax);
				//Xman end
			break;

		case 5: 	// progress
			_sntprintf(pszText, cchTextMax, _T("%s: %.1f%%"), GetResString(IDS_DL_PROGRESS), lpPartFile->GetPercentCompleted());
			break;

		case 6:	{	// sources
			CString strBuffer;
			UINT sc = lpPartFile->GetSourceCount();
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
			// ==> Global Source Limit [Max/Stulle] - Stulle
			/*
			if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
			*/
			if (thePrefs.IsUseGlobalHL())
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetMaxSources());
			// ==> show HL per file constantly - Stulle
			else if (thePrefs.GetShowFileHLconst())
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetMaxSources());
			// <== show HL per file constantly - Stulle
			else if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
			// <== Global Source Limit [Max/Stulle] - Stulle
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

		case 12: // cat
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			/*
			_tcsncpy(pszText, (lpPartFile->GetCategory() != 0) ? thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle : _T(""), cchTextMax);
			*/
			if (!thePrefs.ShowCatNameInDownList())
				_sntprintf(pszText, cchTextMax, _T("%u"), lpPartFile->GetCategory());
			else
				_tcsncpy(pszText, thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle, cchTextMax);
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			break;
		case 13: // added on
			if (lpPartFile->GetCrCFileDate() != NULL)
				_tcsncpy(pszText, lpPartFile->GetCrCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()), cchTextMax);
			else
				_tcsncpy(pszText, _T("?"), cchTextMax);
			break;

		// Xman Xtreme Downloadmanager: AVG-QR
		case 14:
			if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE )
				_sntprintf(pszText, cchTextMax, _T("%u"), lpPartFile->GetAvgQr());
			break;
		//Xman end

		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		case 15: // Resume Mod
		{
			CString strBuffer;
			strBuffer.Empty();
			Category_Struct* ActiveCat=thePrefs.GetCategory(theApp.emuledlg->transferwnd->GetActiveCategory());
			Category_Struct* curCat=thePrefs.GetCategory(lpPartFile->GetCategory());
			if (curCat && ActiveCat && ActiveCat->viewfilters.nFromCats == 0) {
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
				if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
				UINT dlMode = thePrefs.GetDlMode();
				if (dlMode && curCat->m_iDlMode)
					dlMode = curCat->m_iDlMode;
				else
					strBuffer.Append(((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
				switch (dlMode) {
					case 0:
//						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_A4AF_DISABLED));
						break;
					case 1:
						strBuffer.AppendFormat(_T("%s"), GetResString(IDS_DOWNLOAD_ALPHABETICAL));
						break;
					case 2:
						strBuffer.AppendFormat(_T("%s=%u"), GetResString(IDS_CAT_COLORDER), lpPartFile->GetCatResumeOrder());
						break;
				}
			} else {
				if (strBuffer.IsEmpty() == false) strBuffer.Append(_T(", "));
				strBuffer.AppendFormat(_T("%s=%u"), GetResString(IDS_CAT_COLORDER), lpPartFile->GetCatResumeOrder());
			}
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

		// ==> show # of dropped sources - Stulle
		case 16:
			_sntprintf(pszText, cchTextMax, _T("%i"), lpPartFile->GetShowDroppedSrc());
			break;
		// <== show # of dropped sources - Stulle
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

			if (thePrefs.ShowRatingIndicator() && (pPartFile->HasComment() || pPartFile->HasRating() || pPartFile->IsKadCommentSearchRunning())){
				//Xman Show correct Icons
				/*
				m_ImageList.Draw(dc, pPartFile->UserRating(true) + 14, CPoint(rcDraw.left + 2, rcDraw.top + iIconPosY), ILD_NORMAL);
				*/
				m_ImageList.Draw(dc, pPartFile->UserRating(true) + 5, CPoint(rcDraw.left + 2, rcDraw.top + iIconPosY), ILD_NORMAL);
				//Xman end
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

			//Xman Code Improvement
			/*
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
			*/
			int iWidth = rcDraw.Width();
			int iHeight = rcDraw.Height();
			CDC cdcStatus;
			cdcStatus.CreateCompatibleDC(dc);
			int cx = 0;
			if (pCtrlItem->status != (HBITMAP)NULL)
				cx = pCtrlItem->status.GetBitmapDimension().cx; 
			DWORD dwTicks = GetTickCount();
			if(pCtrlItem->status == (HBITMAP)NULL || pCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !pCtrlItem->dwUpdated) {
				pCtrlItem->status.DeleteObject(); 
				pCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
				pCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
				cdcStatus.SelectObject(pCtrlItem->status); 

				RECT rec_status; 
				rec_status.left = 0; 
				rec_status.top = 0; 
				rec_status.bottom = iHeight; 
				rec_status.right = iWidth; 
				pPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar());

				pCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
			} else 
				cdcStatus.SelectObject(pCtrlItem->status);

			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
			//Xman End

			if (thePrefs.GetUseDwlPercentage())
			{
				COLORREF oldclr = dc->SetTextColor(RGB(255, 255, 255));
				int iOMode = dc->SetBkMode(TRANSPARENT);
				_sntprintf(szItem, _countof(szItem), _T("%.1f%%"), pPartFile->GetPercentCompleted());
				szItem[_countof(szItem) - 1] = _T('\0');
				dc->DrawText(szItem, -1, &rcDraw, (MLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
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
				//Xman SLS
				case SF_SLS:
					_tcsncpy(pszText, _T("SLS"), cchTextMax);
					break;
				//Xman end
			}
			break;

		case 2:		// transferred
			// ==> Show Client UL and DL in Transferred column of DownloadListCtrl [SiRoB] - Stulle
			if(pClient->Credits() && (pClient->Credits()->GetUploadedTotal() || pClient->Credits()->GetDownloadedTotal())){
				_sntprintf(pszText, cchTextMax, _T("%s/%s"),
				CastItoXBytes(pClient->Credits()->GetUploadedTotal(), false, false),
				CastItoXBytes(pClient->Credits()->GetDownloadedTotal(), false, false));
			}
			break;
			// <== Show Client UL and DL in Transferred column of DownloadListCtrl [SiRoB] - Stulle
		case 3:		// completed
			// ==> Downloading Chunk Detail Display [SiRoB] - Stulle
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
			// <== Downloading Chunk Detail Display [SiRoB] - Stulle
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
			//Xman // Maella -Support for tag ET_MOD_VERSION 0x55
			/*
			_tcsncpy(pszText, pClient->GetClientSoftVer(), cchTextMax);
			*/
			_tcsncpy(pszText, pClient->DbgGetFullClientSoftVer(), cchTextMax);
			//Xman End
			break;

		case 7:		// prio
			// ==> last asked time,next asked time - Max 
			/*
			if (pClient->GetDownloadState() == DS_ONQUEUE)
			{
				if (pClient->IsRemoteQueueFull())
					_tcsncpy(pszText, GetResString(IDS_QUEUEFULL), cchTextMax);
				else if (pClient->GetRemoteQueueRank())
					_sntprintf(pszText, cchTextMax, _T("QR: %u"), pClient->GetRemoteQueueRank());
			}
                        */ 
			{
				CString strBuffer;
				//lastAskedTime
				uint32 lastAskedTime = pClient->GetLastAskedTime();
				
				if ( lastAskedTime )
					strBuffer=_T("LR: ")+ CastSecondsToHM(( ::GetTickCount() - lastAskedTime) /1000);
				else
					strBuffer=_T("LR: 0 ");
				

				//nextAskedTime
				uint32 nextAskedTime = pClient->GetJitteredFileReaskTime();

				if(pClient->GetDownloadState()==DS_NONEEDEDPARTS)
					// ==> Timer for ReAsk File Sources [Stulle] - Stulle
					if(thePrefs.GetReAskTimeDif() > 0 && pClient->GetModClient() != MOD_SCAR && pClient->GetModClient() != MOD_MEPHISTO && pClient->GetModClient() != MOD_XTREME)
						nextAskedTime = 2 * (FILEREASKTIME + SEC2MS(30));
					else
					// <== Timer for ReAsk File Sources [Stulle] - Stulle
					nextAskedTime *=2;

				if(pClient->HasTooManyFailedUDP() || pClient->GetDownloadState()==DS_NONEEDEDPARTS || (pClient->HasLowID() && !(pClient->GetBuddyIP() && pClient->GetBuddyPort() && pClient->HasValidBuddyID())))
				{
					if ( (nextAskedTime+pClient->GetLastAskedTime()) < ::GetTickCount() )
						strBuffer=strBuffer+_T("NR: 0");
					else
						strBuffer=strBuffer+_T(" NR: ")+ CastSecondsToHM((nextAskedTime+pClient->GetLastAskedTime()-::GetTickCount())/1000);
				}
				else
				{
					if (pClient->GetNextTCPAskedTime()<=::GetTickCount() || (nextAskedTime+pClient->GetLastAskedTime()) < ::GetTickCount() )
						strBuffer=strBuffer+_T("NR: 0");
					else
						strBuffer=strBuffer+_T(" NR: ")+ CastSecondsToHM((nextAskedTime+pClient->GetLastAskedTime()-::GetTickCount())/1000)+_T(" NR-TCP: ")+ CastSecondsToHM((pClient->GetNextTCPAskedTime()-::GetTickCount())/1000);
				}
				_tcsncpy(pszText, strBuffer, cchTextMax);
			}
			// <== last asked time,next asked time - Max 	
			break;

		case 8: {	// status
			CString strBuffer;
			if (pCtrlItem->type == AVAILABLE_SOURCE) {
				strBuffer = pClient->GetDownloadStateDisplayString();
				//Xman
				//Xman only intern
				//if(lpUpDownClient->GetDownloadState()==DS_TOOMANYCONNS)
				//	buffer.Format(_T("P:%u,M:%u,Q:%u, %s"), pClient->m_downloadpriority, theApp.downloadqueue->GetMaxDownPrio(),pClient->GetRemoteQueueRank() ,GetResString(IDS_TOOMANYCONNS));
				//Xman Xtreme Downloadmanager
				if(thePrefs.IsExtControlsEnabled() && !(pClient->m_OtherRequests_list.IsEmpty() && pClient->m_OtherNoNeeded_list.IsEmpty())) { //Xman Xtreme Downloadmanager
					strBuffer.Append(_T("*"));
				}
				//Xman end
			}
			else {
				//Xman Xtreme Downloadmanager
				/*
				strBuffer = GetResString(IDS_ASKED4ANOTHERFILE);
				*/
				strBuffer.Format(_T("A4AF")); //= GetResString(IDS_ASKED4ANOTHERFILE);
				//Xman end

// ZZ:DownloadManager -->
				if (thePrefs.IsExtControlsEnabled()) {
					if (pClient->IsInNoNeededList(pCtrlItem->owner))
						strBuffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(')');
					else if (pClient->GetDownloadState() == DS_DOWNLOADING)
						strBuffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(')');
					//Xman 0.46b Bugfix
					/*
					else if (const_cast<CUpDownClient *>(pClient)->IsSwapSuspended(pClient->GetRequestFile()))
					*/
					else if (const_cast<CUpDownClient *>(pClient)->IsSwapSuspended(pCtrlItem->owner))
					//Xman end
						strBuffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(')');

					if (pClient->GetRequestFile() && pClient->GetRequestFile()->GetFileName())
						strBuffer.AppendFormat(_T(": \"%s\""), pClient->GetRequestFile()->GetFileName());
				}
			}

			//Xman Xtreme Downloadmanager
            		/*
			if (thePrefs.IsExtControlsEnabled() && !pClient->m_OtherRequests_list.IsEmpty())
				strBuffer.Append(_T("*"));
			*/
			//Xman End
// ZZ:DownloadManager <--
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}

		case 9:		// remaining time & size
			break;

		case 10:	// last seen complete
			break;

		case 11:	// last received
			break;

		case 12:	// category
			break;

		case 13:	// added on
			break;

		case 14:    //Xman Xtreme-Downloadmanager: DiffQR (under AVG-QR)
		{
			sint32    iDifference = pClient->GetDiffQR();
			if (pClient->GetDownloadState()==DS_ONQUEUE && pClient->GetRemoteQueueRank()>0) 
				_sntprintf(pszText, cchTextMax, _T("%+i"), iDifference);
			break;
		}
			//Xman end
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
			//Xman changed: display the obfuscation icon for all clients which enabled it
			/*
			if (pClient->IsObfuscatedConnectionEstablished())
			*/
			if(pClient->IsObfuscatedConnectionEstablished() 
				|| (!(pClient->socket != NULL && pClient->socket->IsConnected())
				&& (pClient->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (pClient->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
			//Xman end
				uOvlImg |= 2;

			POINT point2 = {cur_rec.left, cur_rec.top + iIconPosY};
			//Xman Show correct Icons
			/*
			if (pClient->IsFriend())
				m_ImageList.Draw(dc, 6, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else if (pClient->GetClientSoft() == SO_EDONKEYHYBRID)
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
				m_ImageList.Draw(dc, 5, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			else
				m_ImageList.Draw(dc, 7, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			*/
			uint8 image;
			if (pClient->IsFriend())
				image = 20;
			else if (pClient->GetClientSoft() == SO_EDONKEYHYBRID){
				image = 24;
			}
			else if (pClient->GetClientSoft() == SO_EDONKEY){
				image = 16;
			}
			else if (pClient->GetClientSoft() == SO_MLDONKEY){
				image = 22;
			}
			else if (pClient->GetClientSoft() == SO_SHAREAZA){
				image = 26;
			}
			else if (pClient->GetClientSoft() == SO_AMULE){
				image = 28;
			}
			else if (pClient->GetClientSoft() == SO_LPHANT){
				image = 30;
			}
			else if (pClient->ExtProtocolAvailable()){
				// ==> Mod Icons - Stulle
				/*
				image = 18;
				*/
				if(pClient->GetModClient() == MOD_NONE)
					image = 18;
				else
					image = (uint8)(pClient->GetModClient() + 32);
				// <== Mod Icons - Stulle
			}
			else{
				image = 14;
			}
			//Xman Anti-Leecher
			if(pClient->IsLeecher()>0)
				image=12;
			else
			//Xman end
			// ==> Mod Icons - Stulle
			/*
			if (((pClient->credits)?pClient->credits->GetMyScoreRatio(pClient->GetIP()):0) > 1)
				image++;
			*/
			if (pClient->GetModClient() == MOD_NONE && ((pClient->credits)?pClient->credits->GetMyScoreRatio(pClient->GetIP()):0) > 1)
					image++;
			// <== Mod Icons - Stulle

			m_ImageList.Draw(dc, image, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
			//Xman end
			//Xman friend visualization
			if (pClient->IsFriend() && pClient->GetFriendSlot())
				m_ImageList.Draw(dc,32, point2, ILD_NORMAL);
			//Xman end

			// ==> Mod Icons - Stulle
			if (pClient->Credits() && pClient->Credits()->GetMyScoreRatio(pClient->GetIP()) > 1  && pClient->GetModClient() != MOD_NONE)
			{
				if (uOvlImg & 1)
					m_overlayimages.Draw(dc, 1, point2, ILD_TRANSPARENT);
				else
					m_overlayimages.Draw(dc, 0, point2, ILD_TRANSPARENT);
			}
			// <== Mod Icons - Stulle

			cur_rec.left += 20;

			//EastShare Start - added by AndCycle, IP to Country 
			if(theApp.ip2country->ShowCountryFlag()){
				POINT point3= {cur_rec.left,cur_rec.top+1};
				//theApp.ip2country->GetFlagImageList()->DrawIndirect(dc, pClient->GetCountryFlagIndex(), point3, CSize(18,16), CPoint(0,0), ILD_NORMAL);
				theApp.ip2country->GetFlagImageList()->DrawIndirect(&theApp.ip2country->GetFlagImageDrawParams(dc,pClient->GetCountryFlagIndex(),point3));
				cur_rec.left+=20;
			}
			//EastShare End - added by AndCycle, IP to Country

			dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT | uDrawTextAlignment);
			break;
		}

		// ==> Downloading Chunk Detail Display [SiRoB] - Stulle
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
					pClient->DrawStatusBarChunk(&cdcStatus,  &rec_status,(CPartFile*)pCtrlItem->owner, thePrefs.UseFlatBar());

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
					CFont *pOldFont = cdcStatus.SelectObject(&m_fontBoldSmaller);
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
		// <== Downloading Chunk Detail Display [SiRoB] - Stulle

		case 5: {	// file info
			CRect rcDraw(*lpRect);
			rcDraw.bottom--;
			rcDraw.top++;

			//Xman Code Improvement
			/*
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
				pClient->DrawStatusBar(&cdcStatus,  &rec_status,(pCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar());
				pCtrlItem->dwUpdated = dwTicks + (rand() % 128);
			}
			else
				hOldBitmap = cdcStatus.SelectObject(pCtrlItem->status);
			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY);
			cdcStatus.SelectObject(hOldBitmap);
			*/
			int iWidth = rcDraw.Width();
			int iHeight = rcDraw.Height();
			CDC cdcStatus;
			cdcStatus.CreateCompatibleDC(dc);
			int cx = 0;
			if (pCtrlItem->status != (HBITMAP)NULL)
				cx = pCtrlItem->status.GetBitmapDimension().cx; 
			DWORD dwTicks = GetTickCount();
			if(pCtrlItem->status == (HBITMAP)NULL || pCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !pCtrlItem->dwUpdated) { 
				pCtrlItem->status.DeleteObject(); 
				pCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
				pCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
				cdcStatus.SelectObject(pCtrlItem->status); 

				RECT rec_status; 
				rec_status.left = 0; 
				rec_status.top = 0; 
				rec_status.bottom = iHeight; 
				rec_status.right = iWidth; 
				pClient->DrawStatusBar(&cdcStatus,  &rec_status,(pCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 

				//Xman client percentage (font idea by morph)
				CString buffer;
				// ==> Show Client Percentage optional [Stulle] - Stulle
				/*
				if (thePrefs.GetUseDwlPercentage() && pCtrlItem->type == AVAILABLE_SOURCE)
				*/
				if (thePrefs.GetShowClientPercentage() && pCtrlItem->type == AVAILABLE_SOURCE)
				// <== Show Client Percentage optional [Stulle] - Stulle
				{
					if(pClient->GetHisCompletedPartsPercent_Down() >=0)
					{
						COLORREF oldclr = cdcStatus.SetTextColor(RGB(0,0,0));
						int iOMode = cdcStatus.SetBkMode(TRANSPARENT);
						buffer.Format(_T("%i%%"), pClient->GetHisCompletedPartsPercent_Down());
						CFont *pOldFont = cdcStatus.SelectObject(&m_fontBoldSmaller);
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
					}
				}
				//Xman end

				pCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
			} else 
				cdcStatus.SelectObject(pCtrlItem->status); 

			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
			//Xman end
			break;
		}

// ==> {Color} [Max] 
		case 8:
		{
			COLORREF crOldTxtColor = NULL; 
			if (pCtrlItem->type == AVAILABLE_SOURCE){
				switch (pClient->GetDownloadState()) {
					case DS_CONNECTING:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_CONNECTED:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_WAITCALLBACK:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(210,210,10));
						break;
					case DS_ONQUEUE:
						if( pClient->IsRemoteQueueFull() ){
							crOldTxtColor = dc->SetTextColor((COLORREF)RGB(10,130,160));
						}
						else {
							if ( pClient->GetRemoteQueueRank()){
								int	m_iDifference = pClient->GetDiffQR();
								if(m_iDifference == 0){
									crOldTxtColor = dc->SetTextColor((COLORREF)RGB(60,10,240));
								}
								else if(m_iDifference > 0){
									crOldTxtColor = dc->SetTextColor((COLORREF)RGB(234,28,28));
								}
								else if(m_iDifference < 0){
									crOldTxtColor = dc->SetTextColor((COLORREF)RGB(10,180,50));
								}
							}
							else{
								crOldTxtColor = dc->SetTextColor((COLORREF)RGB(50,80,140));
							}
						}
						break;
					case DS_DOWNLOADING:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(192,0,0));
						break;
					case DS_REQHASHSET:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(245,240,100));
						break;
					case DS_NONEEDEDPARTS:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(30,200,240)); 
						break;
					case DS_LOWTOLOWIP:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					case DS_TOOMANYCONNS:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
						break;
					default:
						crOldTxtColor = dc->SetTextColor((COLORREF)RGB(135,135,135)); 
				}
			}
			else {
				crOldTxtColor = dc->SetTextColor((COLORREF)RGB(200,80,200));
			}

			dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);

			dc->SetTextColor(crOldTxtColor); 
			break;
		}
// <== {Color} [Max] 

		case 9:		// remaining time & size
		case 10:	// last seen complete
		case 11:	// last received
		case 12:	// category
		case 13:	// added on
			break;
		//Xman Xtreme-Downloadmanager: DiffQR (under AVG-QR)
		case 14:
			if (pClient->GetDownloadState()==DS_ONQUEUE && pClient->GetRemoteQueueRank()>0) 
			{
				const COLORREF crOldTxtColor=dc->GetTextColor();
				sint32    iDifference = pClient->GetDiffQR();
				if(iDifference == 0)
					dc->SetTextColor((COLORREF)RGB(60,10,240));
				if(iDifference > 0)
					dc->SetTextColor((COLORREF)RGB(240,125,10));
				if(iDifference < 0)
					dc->SetTextColor((COLORREF)RGB(10,180,50));
				dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
				dc->SetTextColor(crOldTxtColor);
			}
			break;
			//Xman end

		default:
			// ==> Design Settings [eWombat/Stulle] - Stulle
			/*
			//Xman show LowIDs
			COLORREF crOldBackColor = dc->GetBkColor();
			if(pClient->HasLowID())
				dc->SetBkColor(RGB(255,250,200));
			//Xman end
			*/
			// <== Design Settings [eWombat/Stulle] - Stulle
			dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			// ==> Design Settings [eWombat/Stulle] - Stulle
			/*
			dc->SetBkColor(crOldBackColor); //Xman show LowIDs
			*/
			// <== Design Settings [eWombat/Stulle] - Stulle
			break;
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
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
	//Xman narrow font at transferwindow
	/*
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused);
	*/
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true);
	//Xman end
	*/
	InitItemMemDC(dc, lpDrawItemStruct, bCtrlFocused, true, style_b_downloadlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
	CRect cur_rec(lpDrawItemStruct->rcItem);
	CRect rcClient;
	GetClientRect(&rcClient);
	CtrlItem_Struct *content = (CtrlItem_Struct *)lpDrawItemStruct->itemData;
	//Xman Show active downloads bold
	/*
	if (m_pFontBold)
	*/
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	if (m_pFontBold && thePrefs.GetShowActiveDownloadsBold())
	*/
	// <== Design Settings [eWombat/Stulle] - Stulle
	//Xman end Show active downloads bold
	{
		//Xman narrow font at transferwindow
		/*
		if (content->type == FILE_TYPE && ((const CPartFile *)content->value)->GetTransferringSrcCount())
			dc.SelectObject(m_pFontBold);
		else if ((content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE) && (((const CUpDownClient *)content->value)->GetDownloadState() == DS_DOWNLOADING))
			dc.SelectObject(m_pFontBold);
		*/
		// ==> Design Settings [eWombat/Stulle] - Stulle
		/*
		if (content->type == FILE_TYPE && ((const CPartFile *)content->value)->GetTransferringSrcCount())
			dc.SelectObject(thePrefs.UseNarrowFont() ? &m_fontNarrowBold : m_pFontBold);
		else if ((content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE) && (((const CUpDownClient *)content->value)->GetDownloadState() == DS_DOWNLOADING))
			dc.SelectObject(thePrefs.UseNarrowFont() ? &m_fontNarrowBold : m_pFontBold);
		//Xman end
		*/
		// <== Design Settings [eWombat/Stulle] - Stulle
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

	// ==> show global HL - Stulle
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
	
	strRes = GetResString(IDS_DL_SOURCES);
	if (thePrefs.GetShowGlobalHL())
		strRes.AppendFormat(_T(" [%u]"),GlobalHardLimit);

	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);
	// <== show global HL - Stulle
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
			item->dwUpdatedchunk = 0; // Downloading Chunk Detail Display [SiRoB] - Stulle
			item->status.DeleteObject();
			item->statuschunk.DeleteObject(); // Downloading Chunk Detail Display [SiRoB] - Stulle
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
			int iFilesA4AFAuto = 0; //Xman Xtreme Downloadmanager: Auto-A4AF-check
			int	iFilesToImport = 0; //MORPH - Added by SiRoB, Import Parts - added by zz_fly
			// ==> Follow The Majority [AndCycle/Stulle] - Stulle
			UINT uFollowTheMajorityMenuItem = 0;
			CString buffer = NULL;
			// <== Follow The Majority [AndCycle/Stulle] - Stulle

			UINT uPrioMenuItem = 0;
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
				 //MORPH START - Added by SiRoB, Import Parts - added by zz_fly
				iFilesToImport += pFile->GetFileOp() == PFOP_SR13_IMPORTPARTS ? 1 : 0;
				 //MORPH END   - Added by SiRoB, Import Parts
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
				iFilesA4AFAuto += (!bFileDone && pFile->IsA4AFAuto()) ? 1 : 0; //Xman Xtreme Downloadmanager: Auto-A4AF-check

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

				// ==> Follow The Majority [AndCycle/Stulle] - Stulle
				UINT uCurFollowTheMajorityMenuItem = 0;
				if (pFile->GetFollowTheMajority() == -1)
					uCurFollowTheMajorityMenuItem = MP_FOLLOWTHEMAJORITY;
				else
					uCurFollowTheMajorityMenuItem = MP_FOLLOWTHEMAJORITY+1 + pFile->GetFollowTheMajority();

				if (bFirstItem)
					uFollowTheMajorityMenuItem = uCurFollowTheMajorityMenuItem ;
				else if (uFollowTheMajorityMenuItem != uCurFollowTheMajorityMenuItem)
					uFollowTheMajorityMenuItem = 0;
				// <== Follow The Majority [AndCycle/Stulle] - Stulle

				bFirstItem = false;
			}
			//Xman from Stulle
			m_FileMenu.EnableMenuItem((UINT_PTR)m_DropMenu.m_hMenu, (iSelectedItems > 0 && iFilesToStop > 0) ? MF_ENABLED : MF_GRAYED); // enable only when it makes sense - Stulle
			//Xman end

			m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
			m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

			// enable commands if there is at least one item which can be used for the action
			m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesToCancel > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);
			
			bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
			m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);
            
			// ==> XP Style Menu [Xanatos] - Stulle
			/*
			CMenu PreviewWithMenu;
			PreviewWithMenu.CreateMenu();
			*/
			CTitleMenu PreviewWithMenu;
			PreviewWithMenu.CreateMenu();
			PreviewWithMenu.AddMenuTitle(GetResString(IDS_MENU_PREVIEW), true, false);
			// <== XP Style Menu [Xanatos] - Stulle
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

			//MORPH START - Added by SiRoB, Import Parts [SR13] - added by zz_fly
			if(thePrefs.IsExtControlsEnabled()){
				// ==> XP Style Menu [Xanatos] - Stulle
				/*
				m_FileMenu.ModifyMenuAndIcon(MP_SR13_ImportParts, MF_STRING, MP_SR13_ImportParts,(iFilesToImport > 0) ? GetResString(IDS_IMPORTPARTS_STOP) :GetResString(IDS_IMPORTPARTS), _T("FILEIMPORTPARTS"));
				*/
				m_FileMenu.RemoveMenu(MP_SR13_ImportParts,MF_BYCOMMAND);
				m_FileMenu.InsertMenu(MP_SR13_ImportParts,MF_STRING|MF_BYPOSITION,MP_SR13_ImportParts,(iFilesToImport > 0) ? GetResString(IDS_IMPORTPARTS_STOP) :GetResString(IDS_IMPORTPARTS),_T("FILEIMPORTPARTS"));
				// <== XP Style Menu [Xanatos] - Stulle
				m_FileMenu.EnableMenuItem(MP_SR13_ImportParts, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);			
			}
			//m_FileMenu.EnableMenuItem(MP_SR13_InitiateRehash, (iSelectedItems == 1 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
			//MORPH END   - Added by SiRoB, Import Parts [SR13]

			int total;
			m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab, total) > 0 ? MF_ENABLED : MF_GRAYED);

			if (m_SourcesMenu && thePrefs.IsExtControlsEnabled()) {
				m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_ENABLED);
				m_SourcesMenu.CheckMenuItem(MP_ALL_A4AF_AUTO, (iSelectedItems == 1 && iFilesNotDone == 1 && iFilesA4AFAuto == 1) ? MF_CHECKED : MF_UNCHECKED); //Xman Xtreme Downloadmanager: Auto-A4AF-check
				m_SourcesMenu.EnableMenuItem(MP_ADDSOURCE, (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED);
				// ==> File Settings [sivka/Stulle] - Stulle
				/*
				m_SourcesMenu.EnableMenuItem(MP_SETSOURCELIMIT, (iFilesNotDone == iSelectedItems) ? MF_ENABLED : MF_GRAYED);
				*/
				// <== File Settings [sivka/Stulle] - Stulle
			}

			// ==> Follow The Majority [AndCycle/Stulle] - Stulle
			m_FileMenu.EnableMenuItem((UINT_PTR)m_FollowTheMajorityMenu.m_hMenu, (iFilesNotDone > 0) ? MF_ENABLED : MF_GRAYED);
			buffer.Format(_T(" (%s)"),thePrefs.IsFollowTheMajorityEnabled()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
			m_FollowTheMajorityMenu.RemoveMenu(MP_FOLLOWTHEMAJORITY,MF_BYCOMMAND);
			m_FollowTheMajorityMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_FOLLOWTHEMAJORITY,GetResString(IDS_DEFAULT) + buffer);
			m_FollowTheMajorityMenu.CheckMenuRadioItem(MP_FOLLOWTHEMAJORITY, MP_FOLLOWTHEMAJORITY_1, uFollowTheMajorityMenuItem, 0);
			// <== Follow The Majority [AndCycle/Stulle] - Stulle

			// ==> File Settings [sivka/Stulle] - Stulle
			if (thePrefs.IsExtControlsEnabled()) {
				m_FileMenu.EnableMenuItem(MP_SIVKA_FILE_SETTINGS, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
//				m_FileMenu.EnableMenuItem((UINT_PTR)m_DropMenu.m_hMenu, (iSelectedItems > 0 && iFilesToStop > 0) ? MF_ENABLED : MF_GRAYED);
			}
			// <== File Settings [sivka/Stulle] - Stulle

			// ==> Copy feedback feature [MorphXT] - Stulle
			m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK, iSelectedItems > 0? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK_US, iSelectedItems > 0? MF_ENABLED : MF_GRAYED);
			// <== Copy feedback feature [MorphXT] - Stulle

			m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

			//Xman manual file allocation (Xanatos)
			bool ispreallomenu=false;
			if( thePrefs.IsExtControlsEnabled())
			{
				m_FileMenu.AppendMenu(MF_STRING | (iFilesNotDone > 0 && file1->IncompleteAllocateSpace()) ? MF_ENABLED : MF_GRAYED, MP_PREALOCATE, _T("Preallocate discspace")); 
				ispreallomenu=true;
			}
			//Xman end

			m_FileMenu.EnableMenuItem(MP_MASSRENAME, iSelectedItems > 0? MF_ENABLED : MF_GRAYED); //Xman Mass Rename (Morph)

			CTitleMenu WebMenu;
			WebMenu.CreateMenu();
			// ==> XP Style Menu [Xanatos] - Stulle
			/*
			WebMenu.AddMenuTitle(NULL, true);
			*/
			WebMenu.AddMenuTitle(GetResString(IDS_WEBSERVICES), true, false);
			// <== XP Style Menu [Xanatos] - Stulle
			int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
			UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

			// create cat-submenue
			// ==> XP Style Menu [Xanatos] - Stulle
			/*
			CMenu CatsMenu;
			CatsMenu.CreateMenu();
			*/
			CTitleMenu CatsMenu;
			CatsMenu.CreateMenu();
			CatsMenu.AddMenuTitle(GetResString(IDS_CAT), false, false);
			// <== XP Style Menu [Xanatos] - Stulle
			FillCatsMenu(CatsMenu, iFilesInCats);
			m_FileMenu.AppendMenu(MF_POPUP, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT), _T("CATEGORY"));
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			/*
			//Xman checkmark to catogory at contextmenu of downloadlist
			if(iSelectedItems == 1)
				CatsMenu.CheckMenuItem(MP_ASSIGNCAT+file1->GetConstCategory(),MF_CHECKED);
			//Xman end
			*/
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			CTitleMenu mnuOrder;
			if (this->GetSelectedCount() > 1) {
				mnuOrder.CreatePopupMenu();
				mnuOrder.AddMenuTitle(GetResString(IDS_CAT_SETORDER),false,false);
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERAUTOINC, GetResString(IDS_CAT_MNUAUTOINC));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERSTEPTHRU, GetResString(IDS_CAT_MNUSTEPTHRU));
				mnuOrder.AppendMenu(MF_STRING, MP_CAT_ORDERALLSAME, GetResString(IDS_CAT_MNUALLSAME));
				m_FileMenu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)mnuOrder.m_hMenu, GetResString(IDS_CAT_SETORDER), _T("FILELINEARPRIO"));
			}
			else {
				m_FileMenu.AppendMenu(MF_STRING, MP_CAT_SETRESUMEORDER, GetResString(IDS_CAT_SETORDER), _T("FILELINEARPRIO"));
			}
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

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
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			//Xman manual file allocation (Xanatos)
			if(ispreallomenu)
				VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			//Xman end
			if (iPreviewMenuEntries && thePrefs.IsExtControlsEnabled() && !thePrefs.GetExtraPreviewWithMenu())
				VERIFY( m_PreviewMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
			else if (iPreviewMenuEntries)
				VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount()-1,MF_BYPOSITION);
			if (mnuOrder) VERIFY( mnuOrder.DestroyMenu() );
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
			//Xman Xtreme Downloadmanager
			if (client && client->GetDownloadState() == DS_DOWNLOADING)
				ClientMenu.AppendMenu(MF_STRING,MP_STOP_CLIENT,GetResString(IDS_STOP_CLIENT), _T("EXIT"));
			//Xman end
			//Xman friendhandling
			ClientMenu.AppendMenu(MF_SEPARATOR); 
			//Xman end
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
			//Xman friendhandling
			ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
			ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
			ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (client && client->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
			ClientMenu.AppendMenu(MF_SEPARATOR); 
			//Xman end
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
			if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
				ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
			ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));


			// ==> XP Style Menu [Xanatos] - Stulle
			/*
			CMenu A4AFMenu;
			A4AFMenu.CreateMenu();
			*/
			CTitleMenu A4AFMenu;
			A4AFMenu.CreateMenu();
			A4AFMenu.AddMenuTitle(GetResString(IDS_A4AF), false, false);
			// <== XP Style Menu [Xanatos] - Stulle
			if (thePrefs.IsExtControlsEnabled()) {
				//Xman Xtreme Downloadmanager
				/*
// ZZ:DownloadManager -->
#ifdef _DEBUG
                if (content->type == UNAVAILABLE_SOURCE) {
                    A4AFMenu.AppendMenu(MF_STRING,MP_A4AF_CHECK_THIS_NOW,GetResString(IDS_A4AF_CHECK_THIS_NOW));
                }
# endif
// <-- ZZ:DownloadManager
				*/
				if (content->type == UNAVAILABLE_SOURCE)
					A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_THIS,GetResString(IDS_SWAP_A4AF_TO_THIS)); // Added by sivka [Ambdribant]
				if (content->type == AVAILABLE_SOURCE && !(client->m_OtherNoNeeded_list.IsEmpty() && client->m_OtherRequests_list.IsEmpty()))
					A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_OTHER,GetResString(IDS_SWAP_A4AF_TO_OTHER)); // Added by sivka
				//Xman end
				// ==> XP Style Menu [Xanatos] - Stulle
				/*
				if (A4AFMenu.GetMenuItemCount()>0)
				*/
				if (A4AFMenu.GetMenuItemCount()>1)
				// <== XP Style Menu [Xanatos] - Stulle
					ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_A4AF));
			}
			// - show requested files (sivka/Xman)
			ClientMenu.AppendMenu(MF_SEPARATOR);
			//zz_fly :: code improvment by DolphinX
			/*
			ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED),MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); // Added by sivka
			*/
			ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED"));
			//zz_fly :: end
			//ClientMenu.AppendMenu(MF_STRING,MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); // Added by sivka
			//Xman end

			GetPopupMenuPos(*this, point);
			ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
			
			VERIFY( A4AFMenu.DestroyMenu() );
			VERIFY( ClientMenu.DestroyMenu() );
		}
	}
	else{	// nothing selected
		int total;
		//Xman from Stulle
		m_FileMenu.EnableMenuItem((UINT_PTR)m_DropMenu.m_hMenu, MF_GRAYED); // enable only when it makes sense - Stulle
		//Xman end
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
		m_FileMenu.EnableMenuItem(MP_MASSRENAME,MF_GRAYED);//Xman Mass Rename (Morph)

		//MORPH START - Added by SiRoB, Import Parts [SR13] - added by zz_fly
		if (thePrefs.IsExtControlsEnabled())
			m_FileMenu.EnableMenuItem(MP_SR13_ImportParts,MF_GRAYED);
		//m_FileMenu.EnableMenuItem(MP_SR13_InitiateRehash,MF_GRAYED);
		//MORPH END   - Added by SiRoB, Import Parts [SR13]
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab,total) > 0 ? MF_ENABLED : MF_GRAYED);
		// ==> Follow The Majority [AndCycle/Stulle] - Stulle
		if (m_FollowTheMajorityMenu)
			m_FileMenu.EnableMenuItem((UINT_PTR)m_FollowTheMajorityMenu.m_hMenu, MF_GRAYED);
		// <== Follow The Majority [AndCycle/Stulle] - Stulle
		// ==> File Settings [sivka/Stulle] - Stulle
		if (thePrefs.IsExtControlsEnabled()) {
			m_FileMenu.EnableMenuItem(MP_SIVKA_FILE_SETTINGS, MF_GRAYED);
//			m_FileMenu.EnableMenuItem((UINT_PTR)m_DropMenu.m_hMenu, MF_GRAYED);
		}
		// <== File Settings [sivka/Stulle] - Stulle
		m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);
		if (m_SourcesMenu)
			m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_GRAYED);
		// ==> Copy feedback feature [MorphXT] - Stulle
		m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_COPYFEEDBACK_US, MF_GRAYED);
		// <== Copy feedback feature [MorphXT] - Stulle
		m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

		// also show the "Web Services" entry, even if its disabled and therefore not useable, it though looks a little 
		// less confusing this way.
		CTitleMenu WebMenu;
		WebMenu.CreateMenu();
		// ==> XP Style Menu [Xanatos] - Stulle
		/*
		WebMenu.AddMenuTitle(NULL, true);
		*/
		WebMenu.AddMenuTitle(GetResString(IDS_WEBSERVICES), true, false);
		// <== XP Style Menu [Xanatos] - Stulle
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

// ==> XP Style Menu [Xanatos] - Stulle
/*
void CDownloadListCtrl::FillCatsMenu(CMenu& rCatsMenu, int iFilesInCats)
*/
void CDownloadListCtrl::FillCatsMenu(CTitleMenu& rCatsMenu, int iFilesInCats)
// <== XP Style Menu [Xanatos] - Stulle
{
	ASSERT(rCatsMenu.m_hMenu);
	if (iFilesInCats == (-1))
	{
		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		/*
		iFilesInCats = 0;
		*/
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
					// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
					/*
					iFilesInCats += (!pFile->HasDefaultCategory()) ? 1 : 0; 
					*/
					if (iFilesInCats == -1)
						iFilesInCats = pFile->GetCategory();
					else if (iFilesInCats != pFile->GetCategory())
						iFilesInCats = -2;
					// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
				}
			}
		}
	}
	// ==> more icons - Stulle
	/*
	rCatsMenu.AppendMenu(MF_STRING, MP_NEWCAT, GetResString(IDS_NEW) + _T("..."));	
	*/
	rCatsMenu.AppendMenu(MF_STRING, MP_NEWCAT, GetResString(IDS_NEW) + _T("..."), _T("CATADD"));
	// <== more icons - Stulle
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	CString label = GetResString(IDS_CAT_UNASSIGN);
	label.Remove('(');
	label.Remove(')'); // Remove brackets without having to put a new/changed ressource string in
	rCatsMenu.AppendMenu(MF_STRING | ((iFilesInCats == 0) ? MF_GRAYED : MF_ENABLED), MP_ASSIGNCAT, label);
	*/
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	if (thePrefs.GetCatCount() > 1)
	{
		rCatsMenu.AppendMenu(MF_SEPARATOR);
		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		/*
		for (int i = 1; i < thePrefs.GetCatCount(); i++){
		*/
		CString label;
		for (int i = 0; i < thePrefs.GetCatCount(); i++){
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			label = thePrefs.GetCategory(i)->strTitle;
			label.Replace(_T("&"), _T("&&") );
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			/*
			rCatsMenu.AppendMenu(MF_STRING, MP_ASSIGNCAT + i, label);
			*/
			rCatsMenu.AppendMenu(MF_STRING | (iFilesInCats==i)?MF_CHECKED:MF_UNCHECKED, MP_ASSIGNCAT + i, label);
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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

				//Xman Mass Rename (Morph)
				case MP_MASSRENAME: 
				{
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
									file->SetFollowTheMajority(false); // Follow The Majority [AndCycle/Stulle] - Stulle
									file->SetFileName(newname);
									file->SetFilePath(newpath);
									file->SetFullName(newpath);
								} else {
									// Use the "Format"-Syntax of AddLogLine here instead of
									// CString.Format+AddLogLine, because if "%"-characters are
									// in the string they would be misinterpreted as control sequences!
									AddLogLine(false,_T("Successfully renamed .part file '%s' to '%s'"), file->GetFileName(), newname);
									file->SetFollowTheMajority(false); // Follow The Majority [AndCycle/Stulle] - Stulle
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
				//Xman end

				// ==> File Settings [sivka/Stulle] - Stulle
				case MP_SIVKA_FILE_SETTINGS:
					if(selectedCount > 0)
					{
						theApp.downloadqueue->InitTempVariables(selectedList.GetHead());
						CSivkaFileSettings dialog;
						dialog.SetPrefs(&thePrefs);
						dialog.DoModal();

						while(!selectedList.IsEmpty()) {
							if(thePrefs.GetTakeOverFileSettings())
							{
								theApp.downloadqueue->UpdateFileSettings(selectedList.GetHead());
								UpdateItem(selectedList.GetHead());
							}
							selectedList.RemoveHead();
						}
						if(thePrefs.GetTakeOverFileSettings())
							theApp.downloadqueue->SaveFileSettings();
					}
					break;
				// <== File Settings [sivka/Stulle] - Stulle
				// ==> advanced manual dropping - Stulle
				case MP_DROPLOWTOLOWIPSRCS:
					SetRedraw(false);
					while(!selectedList.IsEmpty())
					{ 
						selectedList.GetHead()->RemoveLow2LowIPSourcesManual();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_DROPUNKNOWNERRORBANNEDSRCS:
					SetRedraw(false);
					while(!selectedList.IsEmpty())
					{ 
						selectedList.GetHead()->RemoveUnknownErrorBannedSourcesManual();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_DROPHIGHQRSRCSXMAN:
					SetRedraw(false);
					while(!selectedList.IsEmpty())
					{ 
						selectedList.GetHead()->RemoveHighQRSourcesManualXman();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_DROPHIGHQRSRCSSIVKA:
					SetRedraw(false);
					while(!selectedList.IsEmpty())
					{ 
						selectedList.GetHead()->RemoveHighQRSourcesManualSivka();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				case MP_CLEANUP_NNS_FQS_NONE_ERROR_BANNED_LOWTOLOWIP:
					SetRedraw(false);
					while(!selectedList.IsEmpty())
					{ 
						selectedList.GetHead()->CleanUp_NNS_FQS_NONE_ERROR_BANNED_LOWTOLOWIP_Sources();
						selectedList.RemoveHead();
					}
					SetRedraw(true);
					break;
				// <== advanced manual dropping - Stulle
				// ==> Copy feedback feature [MorphXT] - Stulle
 				case MP_COPYFEEDBACK:
				case MP_COPYFEEDBACK_US:
				{
					CString feed,tmp;
					uint64 uTransferredSum = 0;
					uint64 uTransferredAllSum = 0;
					int iCount = 0;
					POSITION pos = selectedList.GetHeadPosition();

					if(wParam == MP_COPYFEEDBACK_US)
					{
						// ==> Feedback personalization [Stulle] - Stulle
						/*
						feed.AppendFormat(_T("Feedback from %s on [%s]\r\n"),thePrefs.GetUserNick(),theApp.m_strModLongVersion);
						*/
						tmp.Format(_T("Feedback from %s on [%s]"),GetColoredText(thePrefs.GetUserNick(),style_f_names),GetColoredText(theApp.m_strModLongVersion,style_f_names));
						feed.Append(GetColoredText(tmp,style_f_label));
						feed.Append(_T("\r\n"));
						// <== Feedback personalization [Stulle] - Stulle
					}
					else
					{
						// ==> Feedback personalization [Stulle] - Stulle
						/*
						feed.AppendFormat(GetResString(IDS_FEEDBACK_FROM),thePrefs.GetUserNick(), theApp.m_strModLongVersion);
						feed.Append(_T("\r\n"));
						*/
						tmp.Format(GetResString(IDS_FEEDBACK_FROM),GetColoredText(thePrefs.GetUserNick(),style_f_names),GetColoredText(theApp.m_strModLongVersion,style_f_names));
						feed.Append(GetColoredText(tmp,style_f_label));
						feed.Append(_T("\r\n"));
						// <== Feedback personalization [Stulle] - Stulle
					}

					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						feed.Append(file->GetFeedback(wParam == MP_COPYFEEDBACK_US));

						if(pos != NULL) // Feedback personalization [Stulle] - Stulle
							feed.Append(_T("\r\n"));

						uTransferredSum += file->statistic.GetTransferred();
						uTransferredAllSum += file->statistic.GetAllTimeTransferred();
						iCount++;
					}

					if(iCount>1)
					{
						feed.Append(_T("\r\n"));
						if(wParam == MP_COPYFEEDBACK_US)
						{
							// ==> Feedback personalization [Stulle] - Stulle
							/*
							feed.AppendFormat(_T("Transferred (all files): %s (%s)\r\n"),CastItoXBytes(uTransferredSum,false,false,3,true),CastItoXBytes(uTransferredAllSum,false,false,3,true));
							*/
							feed.AppendFormat(_T("Transferred (all files): %s (%s)"),GetColoredText(CastItoXBytes(uTransferredSum,false,false,3,true),style_f_transferred),GetColoredText(CastItoXBytes(uTransferredAllSum,false,false,3,true),style_f_transferred));
							// <== Feedback personalization [Stulle] - Stulle
						}
						else
						{
							// ==> Feedback personalization [Stulle] - Stulle
							/*
							feed.AppendFormat(_T("%s: %s (%s)\r\n"),GetResString(IDS_FEEDBACK_ALL_TRANSFERRED),CastItoXBytes(uTransferredSum,false,false,3),CastItoXBytes(uTransferredAllSum,false,false,3));
							*/
							feed.AppendFormat(_T("%s: %s (%s)"),GetResString(IDS_FEEDBACK_ALL_TRANSFERRED),GetColoredText(CastItoXBytes(uTransferredSum,false,false,3,true),style_f_transferred),GetColoredText(CastItoXBytes(uTransferredAllSum,false,false,3,true),style_f_transferred));
							// <== Feedback personalization [Stulle] - Stulle
						}
					}
					feed.Append(GetColoredText(_T(""),-style_f_label)); // Feedback personalization [Stulle] - Stulle
					feed.Append(_T("\r\n"));
					//Todo: copy all the comments too
					theApp.CopyTextToClipboard(feed);
					break;
				}
				// <== Copy feedback feature [MorphXT] - Stulle
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

				//Xman manual file allocation (Xanatos)
				case MP_PREALOCATE:
					while (!selectedList.IsEmpty()){
						if(selectedList.GetHead()->IncompleteAllocateSpace())
							selectedList.GetHead()->AllocateNeededSpace();
						selectedList.RemoveHead();
					}
					break;
				//Xman end

				//Xman Xtreme Downloadmanager
				case MP_ALL_A4AF_TO_THIS:
					{
						SetRedraw(false);
						if (selectedCount == 1 
							&& (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
						{
							//theApp.downloadqueue->DisableAllA4AFAuto();
							POSITION pos1, pos2;
							for (pos1 = file->A4AFsrclist.GetHeadPosition();(pos2=pos1)!=NULL;)
							{
								file->A4AFsrclist.GetNext(pos1);
								CUpDownClient *cur_source = file->A4AFsrclist.GetAt(pos2);
								if( cur_source->GetDownloadState() != DS_DOWNLOADING
									&& cur_source->GetRequestFile() 
									&& ( (!cur_source->GetRequestFile()->IsA4AFAuto()) || cur_source->GetDownloadState() == DS_NONEEDEDPARTS) //Xman Xtreme Downloadmanager: Auto-A4AF-check
									&& !cur_source->IsSwapSuspended(file) )
								{
									cur_source->SwapToAnotherFile(true, false, false, file,true);
								}
							}
						}
						SetRedraw(true);
						this->UpdateItem(file);						
						break;
					}
				case MP_DROPNONEEDEDSRCS: { 
					if(selectedCount > 1){
						while (!selectedList.IsEmpty()) {
							selectedList.GetHead()->RemoveNoNeededPartsSources();//DS_NONEEDEDPARTS DL-6
							selectedList.RemoveHead();
						}
						break;
					}
					file->RemoveNoNeededPartsSources();
					break;					
										  }
				case MP_DROPQUEUEFULLSRCS: { 
					if(selectedCount > 1){
						while (!selectedList.IsEmpty()) {
							selectedList.GetHead()->RemoveQueueFullSources();
							selectedList.RemoveHead(); 
						}
						break;
					}
					file->RemoveQueueFullSources();
					break;
				}
				//Xman Anti-Leecher
				case MP_DROPLEECHER: { 
					if(selectedCount > 1){
						while (!selectedList.IsEmpty()) {
							selectedList.GetHead()->RemoveLeecherSources();
							selectedList.RemoveHead(); 
						}
						break;
					}
					file->RemoveLeecherSources();
					break;
			    	}
				//Xman end
				case MP_ALL_A4AF_TO_OTHER:
					{
						SetRedraw(false);
						if (selectedCount == 1 
							&& (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
						{
							//theApp.downloadqueue->DisableAllA4AFAuto();
							for(POSITION pos = file->srclist.GetHeadPosition(); pos != NULL; ){
								CUpDownClient* cur_src = file->srclist.GetNext(pos);						
								cur_src->SwapToAnotherFile(false, false, false, NULL,true);	
							}
						}
						SetRedraw(true);
						break;
					}
				//Xman end
				//Xman Xtreme Downloadmanager: Auto-A4AF-check
				case MP_ALL_A4AF_AUTO:
					file->SetA4AFAuto(!file->IsA4AFAuto());
					break;
				//Xman end

				case MPG_F2:
					if (GetAsyncKeyState(VK_CONTROL) < 0 || selectedCount > 1) {
						// when ctrl is pressed -> filename cleanup
						if (IDYES==AfxMessageBox(GetResString(IDS_MANUAL_FILENAMECLEANUP),MB_YESNO))
							while (!selectedList.IsEmpty()){
								CPartFile *partfile = selectedList.GetHead();
								if (partfile->IsPartFile()) {
									file->SetFollowTheMajority(false); // Follow The Majority [AndCycle/Stulle] - Stulle
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
								file->SetFollowTheMajority(false); // Follow The Majority [AndCycle/Stulle] - Stulle
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
				//MORPH START - Added by SiRoB, Import Parts [SR13] - added by zz_fly
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
				// ==> File Settings [sivka/Stulle] - Stulle
				/*
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
				*/
				// <== File Settings [sivka/Stulle] - Stulle
				case MP_ADDSOURCE: {
					if (selectedCount > 1)
						break;
					CAddSourceDlg as;
					as.SetFile(file);
					as.DoModal();
					break;
				}
				// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
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
						
					inputOrder.SetLabels(GetResString(IDS_CAT_SETORDER), GetResString(IDS_CAT_EXPAUTOINC), _T("0"));
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
				// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
				// ==> Follow The Majority [AndCycle/Stulle] - Stulle
				case MP_FOLLOWTHEMAJORITY:
				case MP_FOLLOWTHEMAJORITY_0:
				case MP_FOLLOWTHEMAJORITY_1:
				{
					SetRedraw(false);
					while(!selectedList.IsEmpty())
					{ 
						selectedList.GetHead()->SetFollowTheMajority(wParam - MP_FOLLOWTHEMAJORITY_0);
						selectedList.RemoveHead();
					}
					theApp.downloadqueue->SaveFileSettings();
					SetRedraw(true);
					break;
				}
				// <== Follow The Majority [AndCycle/Stulle] - Stulle
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
			CPartFile* file = (CPartFile*)content->owner; //Xman Xtreme Downloadmanager

			switch (wParam){
				//Xman Xtreme Downloadmanager
				case MP_STOP_CLIENT: 
					StopSingleClient(client);
					break;		
				case MP_SWAP_A4AF_TO_THIS: { 
					if(file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if(!client->GetDownloadState() == DS_DOWNLOADING)
						{
							client->SwapToAnotherFile(true, true, false, file,true);
							UpdateItem(file);
						}
					}
					break;
					}
				case MP_SWAP_A4AF_TO_OTHER:
					if ((client != NULL)  && !(client->GetDownloadState() == DS_DOWNLOADING))
						client->SwapToAnotherFile(true, true, false, NULL,true);
					break;
				//Xman end	
				case MP_SHOWLIST:
					client->RequestSharedFileList();
					break;
				case MP_MESSAGE:
					theApp.emuledlg->chatwnd->StartSession(client);
					break;
				case MP_ADDFRIEND:
					if (theApp.friendlist->AddFriend(client))
						UpdateItem(client);
					break;
				//Xman friendhandling
				case MP_REMOVEFRIEND:
					if (client && client->IsFriend())
					{
						theApp.friendlist->RemoveFriend(client->m_Friend);
						UpdateItem(client);
					}
					break;
				case MP_FRIENDSLOT: 
					if (client)
					{
						bool IsAlready;				
						IsAlready = client->GetFriendSlot();
						theApp.friendlist->RemoveAllFriendSlots();
						if( !IsAlready )
							client->SetFriendSlot(true);
						UpdateItem(client);
					}
					break;
				//Xman end
				case MP_DETAIL:
				case MPG_ALTENTER:
					ShowClientDialog(client);
					break;
				case MP_BOOT:
					if (client->GetKadPort() && client->GetKadVersion() > 1)
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort());
					break;
				// - show requested files (sivka/Xman)
				case MP_LIST_REQUESTED_FILES: { 
					if (client != NULL)
					{
						client->ShowRequestedFiles(); 
					}
					break;
				  }
			  //Xman end
//Xman Xtreme Downloadmanager
/*
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
*/
//Xman end
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
	if (GetSortItem() != pNMListView->iSubItem)
	{
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
	}
	else
		sortAscending = !GetSortAscending();

	// Ornis 4-way-sorting
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
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0 : 100) + adder);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0 : 100) + adder);

	// Save new preferences
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CtrlItem_Struct *item1 = (CtrlItem_Struct *)lParam1;
	const CtrlItem_Struct *item2 = (CtrlItem_Struct *)lParam2;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwOrgSort = lParamSort;
	*/
	//Xman end

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
		{
			comp = Compare((const CPartFile *)(item1->parent->value), (const CPartFile *)(item2->parent->value), lParamSort);
			return sortMod * comp;
		}
		if (item1->type != item2->type)
			return item1->type - item2->type;

		const CUpDownClient *client1 = (const CUpDownClient *)item1->value;
		const CUpDownClient *client2 = (const CUpDownClient *)item2->value;
		comp = Compare(client1, client2, lParamSort);
	}

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	//call secondary sortorder, if this one results in equal
	int dwNextSort;
	if (comp == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetDownloadList()->GetNextSortOrder(dwOrgSort)) != -1)
		return SortProc(lParam1, lParam2, dwNextSort);
	*/
	//SLUGFILLER End
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
			//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			/*
			comp = CompareUnsigned(file1->GetDatarate(), file2->GetDatarate());
			*/
			comp = CompareUnsigned(file1->GetDownloadDatarate(), file2->GetDownloadDatarate());
			//Xman end
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
			time_t f1 = file1->getTimeRemaining();
			time_t f2 = file2->getTimeRemaining();
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

		case 90: //Remaining SIZE asc
			comp = CompareUnsigned64(file1->GetFileSize() - file1->GetCompletedSize(), file2->GetFileSize() - file2->GetCompletedSize());
			break;
		
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

		case 12:
			//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
			// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			/*
			comp = CompareLocaleStringNoCase((const_cast<CPartFile*>(file1)->GetCategory() != 0) ? thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->strTitle:GetResString(IDS_ALL),
											 (const_cast<CPartFile*>(file2)->GetCategory() != 0) ? thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->strTitle:GetResString(IDS_ALL) );
			*/
			if (file1->GetCategory() > file2->GetCategory())
				comp = 1;
			else if (file1->GetCategory() < file2->GetCategory())
				comp = -1;
			else
				comp = 0;
			// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			break;

		case 13: // addeed on asc
			if (file1->GetCrCFileDate() > file2->GetCrCFileDate())
				comp = 1;
			else if(file1->GetCrCFileDate() < file2->GetCrCFileDate())
				comp = -1;
			break;

		//Xman Xtreme-Downloadmanager: AVG-QR
		case 14:
			comp = CompareUnsigned(file1->GetAvgQr(), file2->GetAvgQr());
			break;
		//Xman end

		// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
		case 15: // Mod
			/*
			if (CPartFile::RightFileHasHigherPrio(file2, file1))
				comp=1;
			else if (CPartFile::RightFileHasHigherPrio(file1, file2))
				comp=-1;
			else
				comp=0;
			*/
			{
				UINT dlMode1 = thePrefs.GetDlMode();
				if (dlMode1 && thePrefs.GetCategory(file1->GetCategory())->m_iDlMode)
					dlMode1 = thePrefs.GetCategory(file1->GetCategory())->m_iDlMode;
				UINT dlMode2 = thePrefs.GetDlMode();
				if (dlMode2 && thePrefs.GetCategory(file2->GetCategory())->m_iDlMode)
					dlMode2 = thePrefs.GetCategory(file2->GetCategory())->m_iDlMode;
				if(dlMode1 == 2)
				{
					if(dlMode2 == 2)
					{
						if (file1->GetCatResumeOrder() < file2->GetCatResumeOrder())
							comp = 1;
						else if (file1->GetCatResumeOrder() > file2->GetCatResumeOrder())
							comp = -1;
						else
							comp = 0;
					}
					else
						comp = 1;
				}
				else if (dlMode2 == 2)
					comp = -1;
				else
					comp = 0;
			}
			break;
		// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle

		// ==> show # of dropped sources - Stulle
		case 16:
		{
			comp = CompareUnsigned(file1->GetShowDroppedSrc(), file2->GetShowDroppedSrc());
			break;
		}
		// <== show # of dropped sources - Stulle
	}
	return comp;
}

int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort)
{
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
			// ==> Show Client UL and DL in Transferred column of DownloadListCtrl [SiRoB] - Stulle
			if (!client1->Credits())
				return 1;
			else if (!client2->Credits())
				return -1;
			return CompareUnsigned64(client2->Credits()->GetDownloadedTotal(), client1->Credits()->GetDownloadedTotal());
			// <== Show Client UL and DL in Transferred column of DownloadListCtrl [SiRoB] - Stulle
		case 3://completed asc
			// ==> Downloading Chunk Detail Display [SiRoB] - Stulle
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
			// <== Downloading Chunk Detail Display [SiRoB] - Stulle

		case 4: //speed asc
			return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());

		case 5: //progress asc
			// ==> Sort progress bars by percentage [Fafner/Xman] - Stulle
			/*
			return CompareUnsigned(client1->GetAvailablePartCount(), client2->GetAvailablePartCount());
			*/

			if (client1->GetHisCompletedPartsPercent_Down() == client2->GetHisCompletedPartsPercent_Down())
				return 0;
			else
				return client1->GetHisCompletedPartsPercent_Down() > client2->GetHisCompletedPartsPercent_Down()?1:-1;
			// <== Sort progress bars by percentage [Fafner/Xman] - Stulle

		case 6:
			//Xman
			// Maella -Support for tag ET_MOD_VERSION 0x55-
			/*
			if (client1->GetClientSoft() == client2->GetClientSoft())
				return client1->GetVersion() - client2->GetVersion();
			return -(client1->GetClientSoft() - client2->GetClientSoft()); // invert result to place eMule's at top
			*/
			if (client1->GetClientSoft() == client2->GetClientSoft())
			{
				if(client2->GetVersion() == client1->GetVersion() && (client1->GetClientSoft() == SO_EMULE || client1->GetClientSoft() == SO_AMULE))
					return client2->DbgGetFullClientSoftVer().CompareNoCase( client1->DbgGetFullClientSoftVer());
				else
					return client1->GetVersion() - client2->GetVersion();
			}
			else
				return -(client1->GetClientSoft() - client2->GetClientSoft());
			// Maella end
		
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
		//Xman DiffQR
		case 14:
			if (client1->GetRemoteQueueRank() == 0)
				return 1;
			if (client2->GetRemoteQueueRank() == 0)
				return -1;
			return client1->GetDiffQR() - client2->GetDiffQR();
		//Xman end
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
	//Xman Xtreme Downloadmanager
	if (m_DropMenu) 
		VERIFY( m_DropMenu.DestroyMenu() );
	//Xman End
	// ==> Follow The Majority [AndCycle/Stulle] - Stulle
	if (m_FollowTheMajorityMenu)
		VERIFY( m_FollowTheMajorityMenu.DestroyMenu() );
	// <== Follow The Majority [AndCycle/Stulle] - Stulle

	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );

	m_FileMenu.CreatePopupMenu();
	m_FileMenu.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE), true);

	//Xman Xtreme Downloadmanager
	m_DropMenu.CreateMenu();
	m_DropMenu.AddMenuTitle(_T("DROP"), true, false); // XP Style Menu [Xanatos] - Stulle
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPNONEEDEDSRCS, GetResString(IDS_DROPNONEEDEDSRCS)); 
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPQUEUEFULLSRCS, GetResString(IDS_DROPQUEUEFULLSRCS)); 
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPLEECHER, GetResString(IDS_DROPLEECHER));  //Xman Anti-Leecher
	// ==> advanced manual dropping - Stulle
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPLOWTOLOWIPSRCS, _T("Drop LowIP to LowIP Sources"));
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPUNKNOWNERRORBANNEDSRCS, _T("Drop Unknown, Error and Banned Sources"));
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPHIGHQRSRCSXMAN, _T("Drop High Queue Rating Sources (Xman method)"));
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPHIGHQRSRCSSIVKA, _T("Drop High Queue Rating Sources (Sivka method)"));
	m_DropMenu.AppendMenu(MF_STRING, MP_CLEANUP_NNS_FQS_NONE_ERROR_BANNED_LOWTOLOWIP, _T("CleanUp => NNS, FQS, UNK, ERR, BAN & L2L"));
	// <== advanced manual dropping - Stulle


	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_DropMenu.m_hMenu, GetResString(IDS_SubMenu_Drop),_T("DROPICON") );
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	//Xman end

	// ==> Follow The Majority [AndCycle/Stulle] - Stulle
	m_FollowTheMajorityMenu.CreateMenu();
	m_FollowTheMajorityMenu.AddMenuTitle(GetResString(IDS_FOLLOWTHEMAJORITY), true, false); // XP Style Menu [Xanatos] - Stulle
	m_FollowTheMajorityMenu.AppendMenu(MF_STRING,MP_FOLLOWTHEMAJORITY,	GetResString(IDS_DEFAULT));
	m_FollowTheMajorityMenu.AppendMenu(MF_STRING,MP_FOLLOWTHEMAJORITY_0,	GetResString(IDS_DISABLED));
	m_FollowTheMajorityMenu.AppendMenu(MF_STRING,MP_FOLLOWTHEMAJORITY_1,	GetResString(IDS_ENABLED));
	// <== Follow The Majority [AndCycle/Stulle] - Stulle

	// Add 'Download Priority' sub menu
	//
	m_PrioMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PrioMenu.AddMenuTitle(NULL, true);
	*/
	m_PrioMenu.AddMenuTitle(GetResString(IDS_PRIORITY), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"), _T("FILEPRIORITY"));

	// Add file commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	m_FileMenu.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));
	m_FileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
	m_FileMenu.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL), _T("DELETE"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_OPEN, GetResString(IDS_DL_OPEN), _T("OPENFILE"));
	
	// Extended: Submenu with Preview options, Normal: Preview and possibly 'Preview with' item 
	if (thePrefs.IsExtControlsEnabled())
	{
		m_PreviewMenu.CreateMenu();
		// ==> XP Style Menu [Xanatos] - Stulle
		/*
		m_PreviewMenu.AddMenuTitle(NULL, true);
		*/
		m_PreviewMenu.AddMenuTitle(GetResString(IDS_MENU_PREVIEW), true, false);
		// <== XP Style Menu [Xanatos] - Stulle
		m_PreviewMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
		// ==> XP Style Menu [Xanatos] - Stulle
		/*
		m_PreviewMenu.AppendMenu(MF_STRING, MP_PAUSEONPREVIEW, GetResString(IDS_PAUSEONPREVIEW));
		if (!thePrefs.GetPreviewPrio())
    		m_PreviewMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS));
		*/
		m_PreviewMenu.AppendMenu(MF_STRING, MP_PAUSEONPREVIEW, GetResString(IDS_PAUSEONPREVIEW), _T("FILEDOWNLOADPREVIEWPAUSE"));
		if (!thePrefs.GetPreviewPrio())
	    	m_PreviewMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS), _T("FILEDOWNLOADPREVIEWFIRST"));
		// <== XP Style Menu [Xanatos] - Stulle
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PreviewMenu.m_hMenu, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
	}
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
	
	m_FileMenu.AppendMenu(MF_STRING, MP_METINFO, GetResString(IDS_DL_INFO), _T("FILEINFO"));
	m_FileMenu.AppendMenu(MF_STRING, MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), _T("FILECOMMENTS"));
	//MORPH START - Added by SiRoB, Import Parts [SR13] - added by zz_fly
	if(thePrefs.IsExtControlsEnabled())
		m_FileMenu.AppendMenu(MF_STRING,MP_SR13_ImportParts, GetResString(IDS_IMPORTPARTS), _T("FILEIMPORTPARTS"));
 	//m_FileMenu.AppendMenu(MF_STRING,MP_SR13_InitiateRehash, GetResString(IDS_INITIATEREHASH), _T("FILEINITIATEREHASH"));
	//MORPH END   - Added by SiRoB, Import Parts [SR13]
	//Xman Mass Rename (Morph)
	if (thePrefs.IsExtControlsEnabled()) m_FileMenu.AppendMenu(MF_STRING,MP_MASSRENAME, GetResString(IDS_MR), _T("FILEMASSRENAME")); //Xman Mass Rename (Morph)
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR), _T("CLEARCOMPLETE"));

	// Add (extended user mode) 'Source Handling' sub menu
	//
	if (thePrefs.IsExtControlsEnabled()) {
		m_SourcesMenu.CreateMenu();
		// ==> XP Style Menu [Xanatos] - Stulle
		/*
		m_SourcesMenu.AddMenuTitle(NULL, true);
		*/
		m_SourcesMenu.AddMenuTitle(GetResString(IDS_DL_SOURCES), true, false);
		// <== XP Style Menu [Xanatos] - Stulle		
		//Xman Xtreme Downloadmanager
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_AUTO, GetResString(IDS_ALL_A4AF_AUTO)); //Xman Xtreme Downloadmanager: Auto-A4AF-check
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_THIS, GetResString(IDS_ALL_A4AF_TO_THIS)); 
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_OTHER, GetResString(IDS_ALL_A4AF_TO_OTHER)); 
		//Xman end
		// ==> XP Style Menu [Xanatos] - Stulle
		/*
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY));
		*/
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY), _T("FILEADDSRC"));
		// <== XP Style Menu [Xanatos] - Stulle		
		// ==> File Settings [sivka/Stulle] - Stulle
		/*
		m_SourcesMenu.AppendMenu(MF_STRING, MP_SETSOURCELIMIT, GetResString(IDS_SETPFSLIMIT));
		*/
		// <== File Settings [sivka/Stulle] - Stulle
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_A4AF));
	}
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// ==> Follow The Majority [AndCycle/Stulle] - Stulle
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_FollowTheMajorityMenu.m_hMenu, GetResString(IDS_FOLLOWTHEMAJORITY), _T("RENAME"));
	// <== Follow The Majority [AndCycle/Stulle] - Stulle

	// ==> File Settings [sivka/Stulle] - Stulle
	if (thePrefs.IsExtControlsEnabled()){
		m_FileMenu.AppendMenu(MF_STRING,MP_SIVKA_FILE_SETTINGS, GetResString(IDS_SIVKAFILESETTINGS),_T("DROPDEFAULTS"));
//		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_DropMenu.m_hMenu, _T("Sources Handling (DROP)"));
	}
		m_FileMenu.AppendMenu(MF_SEPARATOR);
	// <== File Settings [sivka/Stulle] - Stulle

	// Add 'Copy & Paste' commands
	//
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_FileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK"));
	m_FileMenu.AppendMenu(MF_STRING, MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD), _T("PASTELINK"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// ==> Copy feedback feature [MorphXT] - Stulle
	m_FileMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK, GetResString(IDS_COPYFEEDBACK), _T("COPY"));
	m_FileMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK_US, GetResString(IDS_COPYFEEDBACK_US), _T("COPY"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	// <== Copy feedback feature [MorphXT] - Stulle

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

//Xman see all sources
/* //Xman no need for an additional function
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
*/
void CDownloadListCtrl::ShowFilesCount()
{
	UINT iCount = 0;
	UINT	countsources=0; 
	UINT  countreadyfiles=0;
	for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0; theApp.downloadqueue->filelist.GetNext(pos)){
		CPartFile* cur_file = theApp.downloadqueue->filelist.GetAt(pos);
		if (cur_file->CheckShowItemInGivenCat(curTab))
		{	
			++iCount;
			countsources += cur_file->GetSourceCount();
			EPartFileStatus status=cur_file->GetStatus();
			if(status!=PS_COMPLETE && status!=PS_PAUSED && status!=PS_HASHING && status!=PS_WAITINGFORHASH && status!=PS_COMPLETING && status!=PS_INSUFFICIENT && status!=PS_ERROR)
				countreadyfiles++;
		}
	}
	theApp.emuledlg->transferwnd->UpdateFilesCount(iCount, countsources, countreadyfiles);
//Xman End
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

					//Xman Xtreme Downloadmanager
					CString askbuffer;
					uint32 uJitteredFileReaskTime=client->GetJitteredFileReaskTime();
					if(client->GetDownloadState()==DS_NONEEDEDPARTS)
						// ==> Timer for ReAsk File Sources [Stulle] - Stulle
						if(thePrefs.GetReAskTimeDif() > 0 && client->GetModClient() != MOD_SCAR && client->GetModClient() != MOD_MEPHISTO && client->GetModClient() != MOD_XTREME)
							uJitteredFileReaskTime = 2 * (FILEREASKTIME + SEC2MS(30));
						else
						// <== Timer for ReAsk File Sources [Stulle] - Stulle
						uJitteredFileReaskTime *=2;

					if(client->HasTooManyFailedUDP() || client->GetDownloadState()==DS_NONEEDEDPARTS || (client->HasLowID() && !(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())))
					{
						if ( (uJitteredFileReaskTime+client->GetLastAskedTime()) < ::GetTickCount() )
							askbuffer=_T(": 0\n");
						else
							if(client->GetDownloadState()==DS_NONEEDEDPARTS || uJitteredFileReaskTime+client->GetLastAskedTime() <= client->GetNextTCPAskedTime())
								askbuffer.Format(_T(": %s\n"),CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000));
							else
							{
								askbuffer.Format(_T(": %s (%s)\n"), CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000), CastSecondsToHM((client->GetNextTCPAskedTime()-::GetTickCount())/1000));
							}
					}
					else
					{
						if (client->GetNextTCPAskedTime()<=::GetTickCount() || (uJitteredFileReaskTime+client->GetLastAskedTime()) < ::GetTickCount() )
							askbuffer=_T(": 0\n");
						else
							askbuffer.Format(_T(": %s (%s)\n"), CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000), CastSecondsToHM((client->GetNextTCPAskedTime()-::GetTickCount())/1000));
					}
					//Xman end

					info.Format(GetResString(IDS_USERINFO)
								+ GetResString(IDS_SERVER) + _T(": %s:%u\n\n")
								//Xman Xtreme Downloadmanager
								/*
								+ GetResString(IDS_NEXT_REASK) + _T(": %s"),
								client->GetUserName() ? client->GetUserName() : (_T('(') + GetResString(IDS_UNKNOWN) + _T(')')),
								ipstr(server), client->GetServerPort(),
								CastSecondsToHM(client->GetTimeUntilReask(client->GetRequestFile()) / 1000));
					if (thePrefs.IsExtControlsEnabled())
						info.AppendFormat(_T(" (%s)"), CastSecondsToHM(client->GetTimeUntilReask(content->owner) / 1000));
								*/
								+ GetResString(IDS_NEXT_REASK) + askbuffer,
								client->GetUserName() ? client->GetUserName() : (_T('(') + GetResString(IDS_UNKNOWN) + _T(')')),
								ipstr(server), client->GetServerPort());
								//Xman end
					info += _T('\n');
					info.AppendFormat(GetResString(IDS_SOURCEINFO), client->GetAskedCountDown(), client->GetAvailablePartCount());
					//Xman Xtreme Downloadmanager
					info.AppendFormat(_T("\nUDP reask possible: %s"), client->HasTooManyFailedUDP() || (client->HasLowID() && !(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())) ? _T("no") : _T("yes")); //Xman Xtreme-Downloadmanager 
					if(client->HasLowID())
					{
						if(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())
							info.AppendFormat(_T("\nclient has buddy"));
						else
							info.AppendFormat(_T("\nclient has no buddy"));
						if (client->GetLowIDReaskPening())
							info.Append(_T(", reask pending"));
					}
					//Xman end

					//Xman Anti-Leecher
					//>>> Anti-XS-Exploit (Xman)
					info.AppendFormat(_T("\n XS-Exploiter: %s. Req:%u Ans:%u"), client->IsXSExploiter() ? _T("yes") : _T("no"), client->GetXSAnswers() > client->GetXSReqs() ? client->GetXSAnswers() : client->GetXSReqs() , client->GetXSAnswers());
					//Xman end
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

//Xman Xtreme Downloadmanager
void CDownloadListCtrl::StopSingleClient(CUpDownClient* single)  
{
	if (single != NULL && single->GetDownloadState() == DS_DOWNLOADING) {
		if(single->socket!=NULL)
		{
			single->SendCancelTransfer();
		}
		single->SetDownloadState(DS_ONQUEUE,_T("download aborted by user"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
	}
}
//Xman end
#ifdef PRINT_STATISTIC

uint32 CtrlItem_Struct::amount;

void CDownloadListCtrl::PrintStatistic()
{
	AddLogLine(false, _T("DownloadlistControl: Listitems: %u"), m_ListItems.size());
	AddLogLine(false, _T("DownloadlistControl: CtrlItem_Structs: %u"), CtrlItem_Struct::amount);
}
#endif