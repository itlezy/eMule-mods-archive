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
#include "Defaults.h"// X: [IP] - [Import Parts],[POFC] - [PauseOnFileComplete]
#include "shahashset.h"// X: [IP] - [Import Parts]
#include "SharedFilesWnd.h"
#ifdef _DEBUG
#include "log.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CDownloadListCtrl

#define DLC_BARUPDATE 512

#define RATING_ICON_WIDTH	12 //16->12

static const UINT colStrID[]={
	 IDS_DL_FILENAME
	,IDS_DL_SIZE
	,IDS_DL_TRANSF
	,IDS_DL_TRANSFCOMPL
	,IDS_DL_SPEED
	,IDS_DL_PROGRESS
	,IDS_DL_SOURCES
	,IDS_PRIORITY
	,IDS_STATUS
	,IDS_DL_REMAINS
	,IDS_LASTSEENCOMPL
	,IDS_FD_LASTCHANGE
	,IDS_CAT
	,IDS_ADDEDON
	,IDS_AVGQR 
};

//IMPLEMENT_DYNAMIC(CtrlItem_Struct, CObject)

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
	m_tooltip = new CToolTipCtrlX;
	SetGeneralPurposeFind(true);
	SetSkinKey(L"DownloadsLv");
	m_dwLastAvailableCommandsCheck = 0;
	m_availableCommandsDirty = true;
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	if (m_DropMenu)
		VERIFY( m_DropMenu.DestroyMenu() ); //Xman Xtreme Downloadmanager
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
	delete m_tooltip;
}

void CDownloadListCtrl::Init()
{
	SetPrefsKey(_T("DownloadListCtrl"));
	SetStyle();
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );
	
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SetFileIconToolTip(true);
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
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
	InsertColumn(9, GetResString(IDS_DL_REMAINS),		LVCFMT_LEFT,  110);
	CString lsctitle = GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(_T(':'));
	InsertColumn(10, lsctitle,							LVCFMT_LEFT,  150,						-1, true);
	lsctitle = GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(_T(':'));
	InsertColumn(11, lsctitle,							LVCFMT_LEFT,  120,						-1, true);
	InsertColumn(12, GetResString(IDS_CAT),				LVCFMT_LEFT,  100,						-1, true);
	InsertColumn(13, GetResString(IDS_ADDEDON),			LVCFMT_LEFT,  120);
	InsertColumn(14, GetResString(IDS_AVGQR),			LVCFMT_RIGHT, 70); //Xman Xtreme-Downloadmanager AVG-QR
	
	SetAllIcons();
	CreateMenues();//Localize();// X: [RUL] - [Remove Useless Localize]
	LoadSettings();
	curTab = thePrefs.lastTranWndCatID; // X: [RCI] - [Remember Catalog ID]

	//Xman Show active downloads bold
	{
		if (thePrefs.GetUseSystemFontForMainControls())
		{
			CFont *pFont = GetFont();
			LOGFONT lfFont = {0};
			pFont->GetLogFont(&lfFont);
			lfFont.lfWeight = FW_BOLD;
			m_fontBold.CreateFontIndirect(&lfFont);
			m_pFontBold = &m_fontBold;
			//Xman client percentage
//			lfFont.lfHeight = 11;
//			m_fontBoldSmaller.CreateFontIndirect(&lfFont);
			//Xman end
		}
		else
			m_pFontBold = &theApp.m_fontDefaultBold;
	}
	//Xman end

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

//Xman Show correct Icons
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
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));	//5
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));		//6
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));		//7
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));		//8
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));		//9
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));	//10
	m_ImageList.Add(CTempIconLoader(_T("KAD_RATING_SEARCH")));	//11 // rating for comments are searched on kad
	//m_ImageList.Add(CTempIconLoader(_T("Server")));
	theApp.SetClientIcon(m_ImageList);//12-31+3
	// Apply the image list also to the listview control, even if we use our own 'DrawItem'.
	// This is needed to give the listview control a chance to initialize the row height.
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	VERIFY( ApplyImageList(m_ImageList) == NULL );
}
//Xman end

void CDownloadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;
/*
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

	strRes = GetResString(IDS_ADDEDON);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);

	//Xman Xtreme Downloadmanager AVG-QR
	strRes = GetResString(IDS_AVGQR); 
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);
*/
	for (int icol=0;icol<pHeaderCtrl->GetItemCount();++icol) {
		strRes=GetResString(colStrID[icol]);
		if(icol > 9 || icol < 12)
			strRes.Remove(_T(':'));
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	CreateMenues();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	// Create new Item
    CtrlItem_Struct* newitem = new CtrlItem_Struct;
    newitem->owner = NULL;
    newitem->type = FILE_TYPE;
    newitem->value = toadd;
    newitem->parent = NULL;
	newitem->dwUpdated = 0; 

	// The same file shall be added only once
	ASSERT(m_ListItems.find(toadd) == m_ListItems.end());
	m_ListItems.insert(ListItemsPair(toadd, newitem));

	if (toadd->CheckShowItemInGivenCat(curTab))
		InsertItem(LVIF_PARAM | LVIF_TEXT, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)newitem);

	theApp.emuledlg->transferwnd->UpdateFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
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
				cur_item->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE/*newitem->type*/;
				cur_item->dwUpdated = 0;
				bFound = true;
				if(notavailable)// X: [CI] - [Code Improvement]
					return;
			}
			else if(notavailable == false){
				// The state 'Available' is exclusive
				cur_item->type = UNAVAILABLE_SOURCE;
				cur_item->dwUpdated = 0;
			}
		}

		if(bFound == true){
			//delete newitem; 
			return;
		}
	}
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
	if (!CemuleDlg::IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::iterator, ListItems::iterator> rangeIt = m_ListItems.equal_range(source);
	for(ListItems::iterator it = rangeIt.first; it != rangeIt.second; ){
		CtrlItem_Struct* delItem  = it->second;
		if(owner == NULL || owner == delItem->owner){
			// Remove it from the m_ListItems			
#if defined(_STLP_WIN32) && !defined(HAVE_BOOST)
			m_ListItems.erase(it++);
#else
			it = m_ListItems.erase(it);
#endif

			if(owner == NULL || owner->srcarevisible){// X: [CI] - [Code Improvement]
				// Remove it from the CListCtrl
	 			LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)delItem;
				int result = FindItem(&find);
				if (result != -1)
					DeleteItem(result);
			}

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
	if (!CemuleDlg::IsRunning())
		return bResult;
	// Retrieve all entries matching the File or linked to the file
	// Remark: The 'asked another files' clients must be removed from here
	ASSERT(toremove != NULL);
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* delItem = it->second;
		if(delItem->owner == toremove || delItem->value == (void*)toremove){
			// Remove it from the m_ListItems
			it = m_ListItems.erase(it);

			if(delItem->owner == NULL ||  delItem->owner->srcarevisible){// X: [CI] - [Code Improvement]
				// Remove it from the CListCtrl
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)delItem;
				int result = FindItem(&find);
				if (result != -1)
					DeleteItem(result);
			}

			// finally it could be delete
			delete delItem;
			bResult = true;
		}
		else {
			it++;
		}
	}
	theApp.emuledlg->transferwnd->UpdateFilesCount();
	return bResult;
}

void CDownloadListCtrl::UpdateItem(void* toupdate)
{
	if (!CemuleDlg::IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || !/*theApp.emuledlg->transferwnd->GetDownloadList()->*/IsWindowVisible())
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toupdate);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;
		if (updateItem->owner && !updateItem->owner->srcarevisible)// X: [CI] - [Code Improvement]
			continue;
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
			// X-Ray :: SessionDownload :: Start
			if (thePrefs.GetUseSessionDownload())
				_sntprintf(pszText, cchTextMax, L"%s (%s)", CastItoXBytes(lpPartFile->GetTransferred(), false, false), CastItoXBytes(lpPartFile->GetTransferredSession(), false, false));
			// X-Ray :: SessionDownload :: End
			break;

		case 3:		// transferred complete
			_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetCompletedSize(), false, false), cchTextMax);
			break;

//morph4u min. queuerank in downloadlistctrl +
		case 4:{	// speed
			if (lpPartFile->GetTransferringSrcCount())
			{
				_tcsncpy(pszText, CastItoXBytes(lpPartFile->GetDownloadDatarate(), false, true), cchTextMax);
			}
			else if (thePrefs.GetMinQR())
			{
				CString buffer;
				UINT qr = lpPartFile->GetMinQR();
				if (qr != 0 && qr != ~0)
				{
					buffer.Format(_T(""));
					if (qr == ~1)
						buffer.Append(GetResString(IDS_QUEUEFULL));
					else if (qr == ~2)
						buffer.Append(GetResString(IDS_NONEEDEDPARTS));
					else
						buffer.AppendFormat(_T("%i MinQR"), qr);    
					_tcsncpy(pszText, buffer, cchTextMax);
				}
			}
			break;
		}
//morph4u min. queuerank in downloadlistctrl -

		case 5: 	// progress
			if(!thePrefs.GetUseSessionDownload())
				_sntprintf(pszText, cchTextMax, _T("%.2f%%"), lpPartFile->GetPercentCompleted());
	// X-Ray :: SessionDownload :: Start
			else
		 	    _sntprintf(pszText, cchTextMax, _T("%.2f%% / %.2f%%"), lpPartFile->GetPercentCompleted(), (lpPartFile->GetPercentCompleted() - lpPartFile->GetPercentCompletedInitial()));	 
	// X-Ray :: SessionDownload :: End
			break;

		case 6:	{	// sources
			CString strBuffer;
			size_t sc = lpPartFile->GetSourceCount();
// ZZ:DownloadManager -->
			if (!(lpPartFile->GetStatus() == PS_PAUSED && sc == 0) && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				size_t ncsc = lpPartFile->GetNotCurrentSourcesCount();
				strBuffer.Format(_T("%i"), sc - ncsc);
				if (ncsc > 0)
					strBuffer.AppendFormat(_T("/%i"), sc);
				if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetSrcA4AFCount() > 0)
					strBuffer.AppendFormat(_T("+%i"), lpPartFile->GetSrcA4AFCount());
				if (lpPartFile->GetTransferringSrcCount() > 0)
					strBuffer.AppendFormat(_T(" (%i)"), lpPartFile->GetTransferringSrcCount());
			}
// <-- ZZ:DownloadManager
			if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
				strBuffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}

		case 7:		// prio
/*			switch (lpPartFile->GetDownPriority())
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
			}*/
		{
			static const UINT StrID[]={IDS_PRIOAUTOLOW,IDS_PRIOLOW,IDS_PRIOAUTONORMAL,IDS_PRIONORMAL,IDS_PRIOAUTOHIGH,IDS_PRIOHIGH};
			INT_PTR downpriority=lpPartFile->GetDownPriority()<<1;
			ASSERT(	PR_LOW==0 && PR_NORMAL== 1 && PR_HIGH == 2 && downpriority>=PR_LOW && downpriority<=2*PR_HIGH);
			if(!lpPartFile->IsAutoDownPriority())
				++downpriority;
			_tcsncpy(pszText, GetResString(StrID[downpriority]), cchTextMax);
		}
			break;

		case 8:
			_tcsncpy(pszText, lpPartFile->getPartfileStatus(), cchTextMax);
			break;

		case 9:		// remaining time & size
			if (lpPartFile->GetStatus() != PS_COMPLETING && lpPartFile->GetStatus() != PS_COMPLETE)
			{
				uint_ptr restTime;
				if (!thePrefs.UseSimpleTimeRemainingComputation())
					restTime = lpPartFile->getTimeRemaining();
				else
					restTime = lpPartFile->getTimeRemainingSimple();
				_sntprintf(pszText, cchTextMax, _T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes(lpPartFile->leftsize, false, false));
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

			_sntprintf(pszText, cchTextMax, _T("%s (%s)"), ((lpPartFile->lastseencomplete == NULL)?GetResString(IDS_NEVER):lpPartFile->lastseencomplete.Format(thePrefs.GetDateTimeFormat4Lists())), strBuffer);
			break;
		}

		case 11: // last receive
			_tcsncpy(pszText, ((lpPartFile->GetFileDate() != 0 && lpPartFile->GetCompletedSize() > (uint64)0) ? lpPartFile->GetCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()) : GetResString(IDS_NEVER)), cchTextMax);
			break;

		case 12: // cat
			if (lpPartFile->GetCategory() != 0)
				_tcsncpy(pszText,  thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle, cchTextMax);
			break;
		case 13: // added on
			_tcsncpy(pszText, ((lpPartFile->GetCrFileDate() != NULL) ? lpPartFile->GetCrCFileDate().Format(thePrefs.GetDateTimeFormat4Lists()) : _T("?")), cchTextMax);
			break;
		case 14:		// Xman Xtreme Downloadmanager: AVG-QR
		{
			//Xman Xtreme Downloadmanager
			if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE )
				_sntprintf(pszText, cchTextMax, _T("%u"), lpPartFile->GetAvgQr());
			//Xman end
			break;
		}
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPCRECT lpRect, UINT uDrawTextAlignment, CtrlItem_Struct *lpCtrlItem)
{
	/*const*/ CPartFile *lpPartFile = (CPartFile*)lpCtrlItem->value;
	TCHAR szItem[1024];
	GetFileItemDisplayText(lpPartFile, nColumn, szItem, _countof(szItem));
	switch (nColumn)
	{
		case 0: {	// file name
			CRect rcDraw(lpRect);
			int iIconPosY = (rcDraw.Height() > 19) ? ((rcDraw.Height() - 16) / 2) : 1;
			int iImage = theApp.GetFileTypeSystemImageIdx(szItem/*lpPartFile->GetFileName()*/);
			if (theApp.GetSystemImageList() != NULL)
				::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), rcDraw.left, rcDraw.top + iIconPosY, ILD_TRANSPARENT);
			rcDraw.left += 16;

            // X-Ray :: FileStatusIcons :: Start
			if (thePrefs.IsFileStatusIcons()){
				uint8 iIcon;
				uint32 nOverlayImage = 0; //preview icon 
				switch (lpPartFile->GetPartFileStatus()){
					case PS_DOWNLOADING:
						iIcon = 4+ 12;
						break;
					case PS_WAITINGFORSOURCE:
						iIcon = 7+ 12;
						break;
					case PS_PAUSED:
						iIcon = 5+ 12;
						break;			
                    case PS_ERROR:
						iIcon = 8+ 12;
						break;
					case PS_STOPPED:
					default:
						iIcon = 6+ 12;
						break;
				}
                
                if (lpPartFile->IsReadyForPreview() && lpPartFile->IsMovie()) //preview icon
                        nOverlayImage |= 4; //preview icon

				POINT ipoint= {rcDraw.left-2, rcDraw.top+1}; 
				m_ImageList.Draw(dc, iIcon, ipoint, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(nOverlayImage)); //preview icon

				rcDraw.left += 14;
			}
			// X-Ray :: FileStatusIcons :: End

			if (thePrefs.ShowRatingIndicator() && (lpPartFile->HasComment() || lpPartFile->HasRating() || lpPartFile->IsKadCommentSearchRunning())){
				rcDraw.left += sm_i2IconOffset;
				POINT point = {rcDraw.left, rcDraw.top + iIconPosY};
				m_ImageList.Draw(dc, lpPartFile->UserRating(true) + 5, point, ILD_NORMAL);
				rcDraw.left += RATING_ICON_WIDTH;
			}

			rcDraw.left += sm_iLabelOffset;
			dc->DrawText(szItem, -1, &rcDraw, MLC_DT_TEXT);
			break;
		}

		case 5: {	// progress
			CRect rcDraw(*lpRect);
			rcDraw.bottom--;
			rcDraw.top++;

			// added
			//Xman Code Improvement
			int iWidth = rcDraw.Width();
			int iHeight = rcDraw.Height();
			//if (lpCtrlItem->status == (HBITMAP)NULL)
			//	VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL));
			CDC cdcStatus;
			//HGDIOBJ hOldBitmap;
			cdcStatus.CreateCompatibleDC(dc);
			int cx = 0;
			if (lpCtrlItem->status != (HBITMAP)NULL)
				cx = lpCtrlItem->status.GetBitmapDimension().cx; 
			const DWORD dwTicks = GetTickCount();
			if(lpCtrlItem->status == (HBITMAP)NULL || lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !lpCtrlItem->dwUpdated) {
				lpCtrlItem->status.DeleteObject(); 
				lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
				lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
				cdcStatus.SelectObject(lpCtrlItem->status); 

				RECT rec_status = {0, 0, iWidth, iHeight}; 
				lpPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar()); 

				lpCtrlItem->dwUpdated = dwTicks + (t_rng->getUInt32() % 128); 
			} else 
				cdcStatus.SelectObject(lpCtrlItem->status);

			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
			//cdcStatus.SelectObject(hOldBitmap);
			//Xman end

			if (szItem[0] != 0/*thePrefs.GetUseDwlPercentage()*/) {
				COLORREF oldclr = dc->SetTextColor(RGB(255,255,255));
				int iOMode = dc->SetBkMode(TRANSPARENT);
				dc->DrawText(szItem, -1, &rcDraw, (MLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
				dc->SetBkMode(iOMode);
				dc->SetTextColor(oldclr);
			}
			break;
		}

		default:
			if(szItem[0] != 0)
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
			if (pClient->GetUserName())
				_tcsncpy(pszText, pClient->GetUserName(), cchTextMax);
			break;
	
		case 1:		// size
			/*switch (pClient->GetSourceFrom())
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
				case SF_SLS:
					_tcsncpy(pszText, GetResString(IDS_SLS), cchTextMax);
					break;
				//Xman end
			}*/
			{
				static const UINT StrID[]={IDS_SERVER,IDS_KADEMLIA,IDS_SE,IDS_PASSIVE,IDS_SW_LINK,IDS_SLS};
				_tcsncpy(pszText, GetResString(StrID[pClient->GetSourceFrom()]), cchTextMax);
			}
			break;

		case 2:		// transferred
		case 3:		// completed
			// - 'Transferred' column: Show transferred data
			// - 'Completed' column: If 'Transferred' column is hidden, show the amount of transferred data
			//	  in 'Completed' column. This is plain wrong (at least when receiving compressed data), but
			//	  users seem to got used to it.
			if (iSubItem == 2 || IsColumnHidden(2)) {
				if (pCtrlItem->type == AVAILABLE_SOURCE && pClient->GetTransferredDown())
					_tcsncpy(pszText, CastItoXBytes(pClient->GetTransferredDown(), false, false), cchTextMax);
			}
			break;

		case 4:		// speed
			if (pCtrlItem->type == AVAILABLE_SOURCE && pClient->GetDownloadDatarate())
				_tcsncpy(pszText, CastItoXBytes(pClient->GetDownloadDatarate(), false, true), cchTextMax);
			break;

		case 5: 	// file info
			if(thePrefs.GetUseDwlPercentage() && pClient->GetHisCompletedPartsPercent_Down() >=0)
				_sntprintf(pszText, cchTextMax, _T("%i%%"), pClient->GetHisCompletedPartsPercent_Down());
			break;

		case 6:		// sources
			_tcsncpy(pszText, pClient->DbgGetFullClientSoftVer(), cchTextMax);
			break;
		case 7:		// prio
	/*			if (lpUpDownClient->m_nDownloadState==DS_ONQUEUE){
				if (lpUpDownClient->m_bRemoteQueueFull){
					buffer = GetResString(IDS_QUEUEFULL);
					dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), MLC_DT_TEXT);
				}
				else{
					if (lpUpDownClient->m_nRemoteQueueRank){
						buffer.Format(_T("QR: %u"), lpUpDownClient->m_nRemoteQueueRank);
						dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), MLC_DT_TEXT);
					}
					else{
						dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), MLC_DT_TEXT);
					}
				}
			}
			else{
				dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), MLC_DT_TEXT);
			}*/
			// X: [IP2L] - [IP2Location]
			if(theApp.ip2country->IsIP2Country())
				_tcsncpy(pszText, pClient->GetCountryName(), cchTextMax);
			break;

		case 8: {	// status
			CString strBuffer;
			if (pCtrlItem->type == AVAILABLE_SOURCE) {
				strBuffer = pClient->GetDownloadStateDisplayString();
				if(thePrefs.IsExtControlsEnabled() && !(pClient->m_OtherRequests_list.IsEmpty() && pClient->m_OtherNoNeeded_list.IsEmpty())) { //Xman Xtreme Downloadmanager
					strBuffer.AppendChar(_T('*'));
				}
			}
			else {
				strBuffer = _T("A4AF"); //= GetResString(IDS_ASKED4ANOTHERFILE);
			//Xman end

// ZZ:DownloadManager -->
				if(thePrefs.IsExtControlsEnabled()) {
					if (pClient->IsInNoNeededList(pCtrlItem->owner))
						strBuffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(')');
					else if (pClient->GetDownloadState() == DS_DOWNLOADING)
						strBuffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(')');
					else if(const_cast<CUpDownClient *>(pClient)->IsSwapSuspended(pCtrlItem->owner)) //Xman 0.46b Bugfix
						strBuffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(')');

					if (pClient->GetRequestFile() && pClient->GetRequestFile()->GetFileName())
						strBuffer.AppendFormat(_T(": \"%s\""), pClient->GetRequestFile()->GetFileName());
				}
			}
			_tcsncpy(pszText, strBuffer, cchTextMax);
			break;
		}

	// ZZ:DownloadManager <--
		/*case 9:		// remaining time & size
			break;

		case 10:	// last seen complete
			break;

		case 11:	// last received
			break;

		case 12:	// category
			break;

		case 13:	// added on
			break;

		case 14:	//Xman Xtreme-Downloadmanager: DiffQR (under AVG-QR)
			break;*/
	}
	pszText[cchTextMax - 1] = _T('\0');
}

void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPCRECT lpRect, UINT uDrawTextAlignment, CtrlItem_Struct *lpCtrlItem) {
	CUpDownClient *lpUpDownClient = (CUpDownClient*)lpCtrlItem->value;
	TCHAR szItem[1024];
	GetSourceItemDisplayText(lpCtrlItem, nColumn, szItem, _countof(szItem));
	switch(nColumn) {
		case 0: {	// icon, name, status
			CRect cur_rec(*lpRect);
			int iIconPosY = (cur_rec.Height() > 19) ? ((cur_rec.Height() - 16) / 2) : 1;
			POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
			if (lpCtrlItem->type == AVAILABLE_SOURCE){
				switch (lpUpDownClient->GetDownloadState()) {
				case DS_CONNECTING:
					//m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
					//break;
				case DS_CONNECTED:
					//m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
					//break;
				case DS_WAITCALLBACKKAD:
				case DS_WAITCALLBACK:
					m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
					break;
				case DS_ONQUEUE:
					if(lpUpDownClient->IsRemoteQueueFull())
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
					else
						m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
					break;
				case DS_DOWNLOADING:
					//m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
					//break;
				case DS_REQHASHSET:
					m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
					break;
				case DS_NONEEDEDPARTS:
					//m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
					//break;
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
			cur_rec.left += 16 + sm_i2IconOffset;

			UINT uOvlImg = 0;
                    //morph4u share visible +
                    if (lpUpDownClient->GetUserName() && lpUpDownClient->GetViewSharedFilesSupport())
						uOvlImg |= 5;
                    //morph4u share visible -
			if ((lpUpDownClient->Credits() && lpUpDownClient->Credits()->GetCurrentIdentState(lpUpDownClient->GetIP()) == IS_IDENTIFIED))
				uOvlImg |= 1;
			//Xman changed: display the obfuscation icon for all clients which enabled it
			if(lpUpDownClient->IsObfuscatedConnectionEstablished() 
				|| (!(lpUpDownClient->socket != NULL && lpUpDownClient->socket->IsConnected())
				&& (lpUpDownClient->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (lpUpDownClient->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
				uOvlImg |= 2;

			point.x = cur_rec.left;
			int image;
#ifdef CLIENTANALYZER			
			if(lpUpDownClient->IsBadGuy())
				image=9 + 12;
			else
#endif			
			{
				image = lpUpDownClient->GetImageIndex() + 12;
				if (lpUpDownClient->credits && lpUpDownClient->credits->GetMyScoreRatio(lpUpDownClient->GetIP()) >  1.0f)
					++image;
			}
			m_ImageList.Draw(dc, image, point, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
            //Xman end			

			cur_rec.left += 16;

			//EastShare Start - added by AndCycle, IP to Country 
			if(theApp.ip2country->ShowCountryFlag() ){
				point.x = cur_rec.left += sm_i2IconOffset;
				theApp.ip2country->GetFlagImageList()->Draw(dc, lpUpDownClient->GetCountryFlagIndex(), point, ILD_NORMAL);
				cur_rec.left += 19;
			}
			//EastShare End - added by AndCycle, IP to Country

			cur_rec.left += sm_iLabelOffset;

            //MORPH START - Added by IceCream, [sivka: -A4AF counter, ahead of user nickname-]
			CString buffer;
			buffer.Format(_T("(%i) "),lpUpDownClient->m_OtherRequests_list.GetCount()+1+lpUpDownClient->m_OtherNoNeeded_list.GetCount());
			dc->DrawText(buffer, buffer.GetLength(), &cur_rec, MLC_DT_TEXT);
			cur_rec.left += dc->GetTextExtent(buffer).cx;
			//MORPH END   - Added by IceCream, [sivka: -A4AF counter, ahead of user nickname-]

                        //colors +
                        if (lpUpDownClient->GetUserName()){
						COLORREF crOldBackColor = dc->GetBkColor();
                        if (lpUpDownClient->IsFriend() && lpUpDownClient->GetFriendSlot())
			             	dc->SetBkColor(RGB(210,240,255)); //blue
						else if(lpUpDownClient->IsFriend())
							dc->SetBkColor(RGB(200,250,200)); //green
						_tcsncpy(szItem, lpUpDownClient->GetUserName(), _countof(szItem) - 1);
						szItem[_countof(szItem) - 1] = _T('\0');
						dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
						dc->SetBkColor(crOldBackColor);
					    }else
			            //colors -

			dc->DrawText(szItem, -1, &cur_rec, MLC_DT_TEXT);
			break;
		}

		case 5: {	// file info
			CRect rcDraw(*lpRect);
			rcDraw.bottom--;
			rcDraw.top++;

			//Xman Code Improvement
			int iWidth = rcDraw.Width();
			int iHeight = rcDraw.Height();
			//if (lpCtrlItem->status == (HBITMAP)NULL)
				//VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
			CDC cdcStatus;
			//HGDIOBJ hOldBitmap;
			cdcStatus.CreateCompatibleDC(dc);
			int cx = 0;
			if (lpCtrlItem->status != (HBITMAP)NULL)
				cx = lpCtrlItem->status.GetBitmapDimension().cx; 
			const DWORD dwTicks = GetTickCount();
			if(lpCtrlItem->status == (HBITMAP)NULL
				|| (lpCtrlItem->type == AVAILABLE_SOURCE // X: don't update UNAVAILABLE_SOURCE
					&& lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks)
				|| cx !=  iWidth 
				|| !lpCtrlItem->dwUpdated) { 
				lpCtrlItem->status.DeleteObject(); 
				lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
				lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
				cdcStatus.SelectObject(lpCtrlItem->status); 

				RECT rec_status = {0, 0, iWidth, iHeight};
				lpUpDownClient->DrawStatusBar(&cdcStatus,  &rec_status,(lpCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 

				//Xman client percentage (font idea by morph)
				if (szItem[0] != 0/*thePrefs.GetUseDwlPercentage()*/ && lpCtrlItem->type == AVAILABLE_SOURCE)
				{
					//if(lpUpDownClient->GetHisCompletedPartsPercent_Down() >=0)
					//{
					/*COLORREF oldclr = */cdcStatus.SetTextColor(RGB(0,0,0));
					/*int iOMode = */cdcStatus.SetBkMode(TRANSPARENT);
					/*CFont *pOldFont = */cdcStatus.SelectObject(GetFont());

#define	DrawClientPercentText		cdcStatus.DrawText(szItem, -1, &rec_status, ((MLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
					/*rec_status.top-=1;rec_status.bottom-=1;
					DrawClientPercentText;++rec_status.left;++rec_status.right;
					DrawClientPercentText;++rec_status.left;++rec_status.right;
					DrawClientPercentText;++rec_status.top;++rec_status.bottom;
					DrawClientPercentText;++rec_status.top;++rec_status.bottom;
					DrawClientPercentText;--rec_status.left;--rec_status.right;
					DrawClientPercentText;--rec_status.left;--rec_status.right;
					DrawClientPercentText;--rec_status.top;--rec_status.bottom;
					DrawClientPercentText;++rec_status.left;++rec_status.right;
					cdcStatus.SetTextColor(RGB(255,255,255));
					DrawClientPercentText;//--rec_status.left;--rec_status.right;*/
					--rec_status.left;--rec_status.right;
					DrawClientPercentText;++rec_status.left;++rec_status.right;--rec_status.top;--rec_status.bottom;
					DrawClientPercentText;++rec_status.left;++rec_status.right;++rec_status.top;++rec_status.bottom;
					DrawClientPercentText;--rec_status.left;--rec_status.right;++rec_status.top;++rec_status.bottom;
					DrawClientPercentText;--rec_status.top;--rec_status.bottom;
					cdcStatus.SetTextColor(RGB(255,255,255));
					DrawClientPercentText;//++rec_status.left;++rec_status.right;
					//cdcStatus.SelectObject(pOldFont);
					//cdcStatus.SetBkMode(iOMode);
						//cdcStatus.SetTextColor(oldclr);
					//}
				}
				//Xman end

				lpCtrlItem->dwUpdated = dwTicks + (t_rng->getUInt32() % 128); 
				if(lpUpDownClient->GetDownloadState() != DS_DOWNLOADING)// X: delay update
					lpCtrlItem->dwUpdated+=3*DLC_BARUPDATE;
			} else 
				cdcStatus.SelectObject(lpCtrlItem->status); 

			dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
			//cdcStatus.SelectObject(hOldBitmap);
			//Xman end
			break;
		}

        // XCA :: ColoredClientstate :: Start
		case 8:	{	// status
			COLORREF crOldTxtColor = dc->GetTextColor();
			if (lpCtrlItem->type == AVAILABLE_SOURCE)
			{
				switch (lpUpDownClient->GetDownloadState())
           {
					case DS_CONNECTING:
						dc->SetTextColor(RGB(255, 155, 0)); // orange
						break;

					case DS_CONNECTED:
						dc->SetTextColor(RGB(140, 230, 10)); // light green
						break;

					case DS_WAITCALLBACK:
						dc->SetTextColor(RGB(255, 220, 0)); // light orange
						break;

					case DS_ONQUEUE:
						if( lpUpDownClient->IsRemoteQueueFull())
							dc->SetTextColor(RGB(80, 80, 80)); // dark grey
								else
							dc->SetTextColor(RGB(0, 160, 255)); // blue
						break;

					case DS_DOWNLOADING:
						dc->SetTextColor(RGB(0, 160, 0)); // green
						break;

					case DS_REQHASHSET:
						dc->SetTextColor(RGB(245, 240, 50)); // light yellow
						break;

					case DS_NONEEDEDPARTS:
						dc->SetTextColor(RGB(200, 200, 200)); // light grey
						break;

					case DS_LOWTOLOWIP:
						dc->SetTextColor(RGB(135, 0, 135)); // purple
						break;

					case DS_TOOMANYCONNS:
						dc->SetTextColor(RGB(135, 135, 135)); // grey
						break;

					default:
						dc->SetTextColor(RGB(0, 0, 0)); // black
						break;
				}
			}
			dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			dc->SetTextColor(crOldTxtColor);
			break;
		}
		// XCA :: ColoredClientstate :: End

		default:
			if(szItem[0] != 0)
				dc->DrawText(szItem, -1, const_cast<LPRECT>(lpRect), MLC_DT_TEXT | uDrawTextAlignment);
			break;
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	RECT cur_rec = lpDrawItemStruct->rcItem;
	CtrlItem_Struct *content = (CtrlItem_Struct *)lpDrawItemStruct->itemData;
	CMemoryDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	//InitItemMemDC(dc, lpDrawItemStruct->rcItem, (content->type == FILE_TYPE)?((((CPartFile*)content->value)->xState > PFS_NORMAL)?((((CPartFile*)content->value)->xState >= POFC_WAITING)?RGB(200,255,200)/*POFC*/:RGB(255,200,200)/*Import Parts*/):((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow))/*:((((CUpDownClient*)content->value)->HasLowID())?RGB(255,250,200):m_crWindow)*/, lpDrawItemStruct->itemState);
	  InitItemMemDC(dc, lpDrawItemStruct->rcItem, ((lpDrawItemStruct->itemID % 2)?m_crEvenLine:m_crWindow), lpDrawItemStruct->itemState);

	RECT rcClient;// X: [DDHC] - [Don't Draw Hidden Column]
	GetClientRect(&rcClient);

	//Xman Show active downloads bold
	if (m_pFontBold && thePrefs.GetShowActiveDownloadsBold())
	{
		if (content->type == FILE_TYPE && ((const CPartFile *)content->value)->GetTransferringSrcCount())
			dc.SelectObject(m_pFontBold);
		else if ((content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE) && (((const CUpDownClient *)content->value)->GetDownloadState() == DS_DOWNLOADING))
			dc.SelectObject(m_pFontBold);
	}
	//Xman end Show active downloads bold

	BOOL notLast = lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start=0;
	int tree_end=0;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();

	cur_rec.right = cur_rec.left - sm_iSubItemInset;
	cur_rec.left += sm_iSubItemInset;

	if (content->type == FILE_TYPE)
	{
		if (!g_bLowColorDesktop && (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) {
			DWORD dwCatColor = thePrefs.GetCatColor(((/*const*/ CPartFile*)content->value)->GetCategory(), COLOR_WINDOWTEXT);
			if (dwCatColor > 0)
				dc.SetTextColor(dwCatColor);

        //colors +
			if (thePrefs.GetDownloadColor() && ((CPartFile*)content->value)->GetTransferringSrcCount() > 0)
                dc.FillBackground(RGB(255,250,200));
		//colors -
		}

		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			if(IsColumnHidden(iColumn)) continue;
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
				if (iColumn == 5) {
					//set up tree vars
					tree_start = cur_rec.left + 1;
					if(iColumnWidth <= 2*sm_iSubItemInset + 8)
						tree_end = cur_rec.left + iColumnWidth - 2*sm_iSubItemInset;
					else{
						tree_end = cur_rec.left + 8;
						if(iColumnWidth <= 2*sm_iSubItemInset + 16)
							continue;
						int iCurLeft = cur_rec.left;
						//normal column stuff
						cur_rec.left = tree_end + 1;
						DrawFileItem(dc, 5, &cur_rec, uDrawTextAlignment, content);
						cur_rec.left = iCurLeft;
					}
				}
				else
					DrawFileItem(dc, iColumn, &cur_rec, uDrawTextAlignment, content);
			}
			cur_rec.left += iColumnWidth;
			if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
				break;
		}
	}
	else// if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE)
	{
		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			if(IsColumnHidden(iColumn)) continue;
			UINT uDrawTextAlignment;
			int iColumnWidth = GetColumnWidth(iColumn, uDrawTextAlignment);
			cur_rec.right += iColumnWidth;
			if(iColumnWidth > 2*sm_iSubItemInset && cur_rec.right>0){// X: [DDHC] - [Don't Draw Hidden Column]
				if(iColumn == 5) {
					//set up tree vars
					tree_start = cur_rec.left + 1;
					if(iColumnWidth <= 2*sm_iSubItemInset + 8)
						tree_end = cur_rec.left + iColumnWidth - 2*sm_iSubItemInset;
					else{
						tree_end = cur_rec.left + 8;
						if(iColumnWidth > 2*sm_iSubItemInset + 16){
							int iCurLeft = cur_rec.left;
							//normal column stuff
							cur_rec.left = tree_end + 1;
							DrawSourceItem(dc, 5, &cur_rec, uDrawTextAlignment, content);
							cur_rec.left = iCurLeft;
						}
                  }
				} 
				else
					DrawSourceItem(dc, iColumn, &cur_rec, uDrawTextAlignment, content);
			}
			cur_rec.left += iColumnWidth;
			if(cur_rec.left>=rcClient.right)// X: [DDHC] - [Don't Draw Hidden Column]
				break;
		}
	}

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
}

void CDownloadListCtrl::HideSources(CPartFile* toCollapse)
{
	SetRedraw(false);
	size_t pre = 0;
	size_t post = 0;
	for (size_t i = 0; i < GetItemCount(); i++)
	{
		CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData(i);
		if (item != NULL && item->owner == toCollapse)
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
	if (point.x != -1 || point.y != -1) {// X: [BF] - [Bug Fix]
		RECT rcClient;
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);
		if (!PtInRect(&rcClient,point)) {
			Default();
			return;
		}
	}
	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content != NULL && content->type == FILE_TYPE)
		{
			// get merged settings
			bool bFirstItem = true;
			size_t iSelectedItems = 0;
			size_t iFilesNotDone = 0;
			size_t iFilesToPause = 0;
			size_t iFilesToStop = 0;
			size_t iFilesToResume = 0;
			size_t iFilesToOpen = 0;
            size_t iFilesGetPreviewParts = 0;
///            size_t iFilesPreviewType = 0;
			size_t iFilesToPreview = 0;
			size_t iFilesToCancel = 0;
			size_t iFilesCanPauseOnPreview = 0;
			size_t iFilesDoPauseOnPreview = 0;
			size_t iFilesInCats = 0;
			size_t iFilesA4AFAuto = 0; //Xman Xtreme Downloadmanager: Auto-A4AF-check
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
				iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
                iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
///                iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
				iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;
				iFilesCanPauseOnPreview += (/*pFile->IsPreviewableFileType() && */!pFile->IsReadyForPreview() && pFile->CanPauseFile()) ? 1 : 0;
				iFilesDoPauseOnPreview += (pFile->IsPausingOnPreview()) ? 1 : 0;
				iFilesInCats += (pFile->GetConstCategory() != 0) ? 1 : 0; 
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

				if (bFirstItem){
					uPrioMenuItem = uCurPrioMenuItem;
					bFirstItem = false;
				}
				else if (uPrioMenuItem != uCurPrioMenuItem)
					uPrioMenuItem = 0;

			}
			//Xman from Stulle
			m_FileMenu.EnableMenuItem((UINT)m_DropMenu.m_hMenu, (iSelectedItems > 0 && iFilesToStop > 0) ? MF_ENABLED : MF_GRAYED); // enable only when it makes sense - Stulle
			//Xman end
			m_FileMenu.EnableMenuItem((UINT)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
			m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

			// enable commands if there is at least one item which can be used for the action
			m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesToCancel > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);
			
			bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
			m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);

			bool bDetailsEnabled = (iSelectedItems > 0);
			m_FileMenu.EnableMenuItem(MP_METINFO, bDetailsEnabled ? MF_ENABLED : MF_GRAYED);
			if (thePrefs.IsDoubleClickEnabled() && bOpenEnabled)
				m_FileMenu.SetDefaultItem(MP_OPEN);
			else if (!thePrefs.IsDoubleClickEnabled() && bDetailsEnabled)
				m_FileMenu.SetDefaultItem(MP_METINFO);
			else
				m_FileMenu.SetDefaultItem((UINT)-1);
			m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, (iSelectedItems >= 1 /*&& iFilesNotDone == 1*/) ? MF_ENABLED : MF_GRAYED);

			m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab/*, total*/) > 0 ? MF_ENABLED : MF_GRAYED);// X: [CI] - [Code Improvement]



			m_FileMenu.EnableMenuItem(thePrefs.m_bShowCopyEd2kLinkCmd ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);
            
			CMenu PreviewWithMenu;
			PreviewWithMenu.CreateMenu();
			size_t iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(PreviewWithMenu, (iSelectedItems == 1) ? file1 : NULL);

			if( thePrefs.IsExtControlsEnabled()){
				if(!thePrefs.GetPreviewPrio()) {
					m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1/*&& iFilesPreviewType == 1*/&& iFilesToPreview == 0 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);// X: [AC] - [ActionChange] Allow all file type(as well as archive file) to download the preview part
					m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesGetPreviewParts == 1) ? MF_CHECKED : MF_UNCHECKED);
				}
				m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_PAUSEONPREVIEW, iFilesCanPauseOnPreview > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.CheckMenuItem(MP_PAUSEONPREVIEW, (iSelectedItems > 0 && iFilesDoPauseOnPreview == iSelectedItems) ? MF_CHECKED : MF_UNCHECKED);
				if (iPreviewMenuEntries > 0 && !thePrefs.GetExtraPreviewWithMenu())
					m_FileMenu.InsertMenu(1, MF_POPUP | MF_BYPOSITION | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
				else if (iPreviewMenuEntries > 0)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | MF_BYCOMMAND | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
				if (m_SourcesMenu) {
					m_FileMenu.EnableMenuItem((UINT)m_SourcesMenu.m_hMenu, MF_ENABLED);
					m_SourcesMenu.CheckMenuItem(MP_ALL_A4AF_AUTO, (iSelectedItems == 1 && iFilesNotDone == 1 && iFilesA4AFAuto == 1) ? MF_CHECKED : MF_UNCHECKED); //Xman Xtreme Downloadmanager: Auto-A4AF-check
					m_SourcesMenu.EnableMenuItem(MP_ADDSOURCE, (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED);
					m_SourcesMenu.EnableMenuItem(MP_SETSOURCELIMIT, (iFilesNotDone == iSelectedItems) ? MF_ENABLED : MF_GRAYED);
					m_SourcesMenu.EnableMenuItem(MP_C0SC, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);// X: [C0SC] - [Clear0SpeedClient]
				}
				m_FileMenu.EnableMenuItem(MP_FLUSHBUFFER, (iFilesToStop > 0 && !file1->IsAllocating()) ? MF_ENABLED : MF_GRAYED); // X: [FB] - [FlushBuffer]
				m_FileMenu.EnableMenuItem(MP_PREALOCATE, (iFilesNotDone > 0 && file1->IncompleteAllocateSpace()) ? MF_ENABLED : MF_GRAYED); 
				m_FileMenu.AppendMenu(MF_STRING | (iSelectedItems == 1 && iFilesToStop ==1 && ((CPartFile*)file1)->GetAICHRecoveryHashSet()->CanTrustMajority()) ? MF_ENABLED : MF_GRAYED, MP_AICHHASH, _T("Trust AICH Hash"));
			}
			else {
				m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				if (iPreviewMenuEntries)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | MF_BYCOMMAND | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewWithMenu.m_hMenu, GetResString(IDS_PREVIEWWITH));
			}

			CTitleMenu WebMenu;
			WebMenu.CreateMenu();
			WebMenu.AddMenuTitle(NULL, true);
			size_t iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
			UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;
			m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES));

			// create cat-submenue
			CMenu CatsMenu;
			CatsMenu.CreateMenu();
			FillCatsMenu(CatsMenu, iFilesInCats);
			m_FileMenu.AppendMenu(MF_POPUP, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT));
			//Xman checkmark to catogory at contextmenu of downloadlist
			if(iSelectedItems == 1)
				CatsMenu.CheckMenuItem(MP_ASSIGNCAT+file1->GetConstCategory(),MF_CHECKED);
			//Xman end

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
			if( thePrefs.IsExtControlsEnabled())
				VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );// X: [IP] - [Import Parts]
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
			if (iPreviewMenuEntries && thePrefs.IsExtControlsEnabled() && !thePrefs.GetExtraPreviewWithMenu())
				VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewWithMenu.m_hMenu, MF_BYCOMMAND) );
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
			ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS));
			ClientMenu.SetDefaultItem(MP_DETAIL);
			//Xman Xtreme Downloadmanager
			if (client && client->GetDownloadState() == DS_DOWNLOADING)
				ClientMenu.AppendMenu(MF_STRING,MP_STOP_CLIENT,GetResString(IDS_STOP_CLIENT));
			//xman end
			//Xman friendhandling
			ClientMenu.AppendMenu(MF_SEPARATOR); 
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND));
			ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND));
			ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? (client->GetFriendSlot() ? MF_CHECKED : MF_UNCHECKED) : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT));
			ClientMenu.AppendMenu(MF_SEPARATOR); 
			//Xman end
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG));
			ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES));
			if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
				ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0 && client->GetKadVersion() > 1) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
			ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND));
                       
			CMenu A4AFMenu;
			A4AFMenu.CreateMenu();
			if (thePrefs.IsExtControlsEnabled()) {
				//Xman Xtreme Downloadmanager
				//if (content->type == UNAVAILABLE_SOURCE) {
				//A4AFMenu.AppendMenu(MF_STRING,MP_A4AF_CHECK_THIS_NOW,GetResString(IDS_A4AF_CHECK_THIS_NOW));
				if (content->type == UNAVAILABLE_SOURCE)
					A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_THIS,GetResString(IDS_SWAP_A4AF_TO_THIS)); // Added by sivka [Ambdribant]
				if (content->type == AVAILABLE_SOURCE && !(client->m_OtherNoNeeded_list.IsEmpty() && client->m_OtherRequests_list.IsEmpty()))
					A4AFMenu.AppendMenu(MF_STRING,MP_SWAP_A4AF_TO_OTHER,GetResString(IDS_SWAP_A4AF_TO_OTHER)); // Added by sivka

				//}
				//Xman end
				if (A4AFMenu.GetMenuItemCount()>0)
					ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_A4AF));
			}
			// - show requested files (sivka/Xman)
			ClientMenu.AppendMenu(MF_SEPARATOR);
			ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED)); // Added by sivka
			//Xman end

			GetPopupMenuPos(*this, point);
			ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

			VERIFY( A4AFMenu.DestroyMenu() );
			VERIFY( ClientMenu.DestroyMenu() );
		}
	}
	else{	// nothing selected
		//Xman from Stulle
		m_FileMenu.EnableMenuItem((UINT)m_DropMenu.m_hMenu, MF_GRAYED); // enable only when it makes sense - Stulle
		//Xman end
		m_FileMenu.EnableMenuItem((UINT)m_PrioMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CANCEL, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);

	if (thePrefs.IsExtControlsEnabled()) {
			if (!thePrefs.GetPreviewPrio())
			{
				m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_GRAYED);
				m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_UNCHECKED);
			}
			m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_PAUSEONPREVIEW, MF_GRAYED);
        }
		else {
		m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
		}
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab/*,total*/) > 0 ? MF_ENABLED : MF_GRAYED);// X: [CI] - [Code Improvement]
		m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);
		m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
		if (thePrefs.IsExtControlsEnabled()) {
			if (m_SourcesMenu)
				m_FileMenu.EnableMenuItem((UINT)m_SourcesMenu.m_hMenu, MF_GRAYED);
			m_FileMenu.EnableMenuItem(MP_FLUSHBUFFER, MF_GRAYED);// X: [FB] - [FlushBuffer]
			m_FileMenu.EnableMenuItem(MP_PREALOCATE, MF_GRAYED);
		}

		// also show the "Web Services" entry, even if its disabled and therefore not useable, it though looks a little 
		// less confusing this way.
		/*CTitleMenu WebMenu;
		WebMenu.CreateMenu();
		WebMenu.AddMenuTitle(NULL, true);
		theWebServices.GetFileMenuEntries(&WebMenu);
		m_FileMenu.AppendMenu(MF_POPUP | MF_GRAYED, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES));*/

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
		//m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
		//VERIFY( WebMenu.DestroyMenu() );
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
					iFilesInCats += (pFile->GetConstCategory() != 0) ? 1 : 0; 
				}
			}
		}
	}
	rCatsMenu.AppendMenu(MF_STRING, MP_NEWCAT, GetResString(IDS_NEW) + _T("..."));	
	CString label = GetResString(IDS_CAT_UNASSIGN);
	//label.Remove('(');
	//label.Remove(')'); // Remove brackets without having to put a new/changed ressource string in
	rCatsMenu.AppendMenu(MF_STRING | ((iFilesInCats == 0) ? MF_GRAYED : MF_ENABLED), MP_ASSIGNCAT, label);
	if (thePrefs.GetCatCount() > 1)
	{
		rCatsMenu.AppendMenu(MF_SEPARATOR);
		for (size_t i = 1; i < thePrefs.GetCatCount(); i++){
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
				theApp.PasteClipboard(curTab);// X: [AC] - [ActionChange] use transferwnd's select cat as default cat
			return TRUE;
		case MP_FIND:
			OnFindStart();
			return TRUE;
		case MP_TOGGLEDTOOLBAR:
			thePrefs.SetDownloadToolbar(true);
			theApp.emuledlg->transferwnd->ShowToolbar(true);
			return TRUE;
//>>> WiZaRd::OpenIncoming
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW); 
			return TRUE;
//<<< WiZaRd::OpenIncoming
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
			size_t selectedCount = 0;
			CAtlList<CPartFile*> selectedList; 
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
									fileList.Append(_T('\n') + CString(cur_file->GetFileName()));
								else if(cFiles == iMaxDisplayFiles && pos != NULL)
									fileList.Append(_T("\n..."));
							}
							else if (cur_file->GetStatus() == PS_COMPLETE)
								removecompl = true;
						}
						CString quest = GetResString((selectedCount == 1)?IDS_Q_CANCELDL2:IDS_Q_CANCELDL);// X: [CI] - [Code Improvement]
						if ((removecompl && !validdelete) || (validdelete && AfxMessageBox(quest + fileList, MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDYES))
						{
							bool bRemovedItems = false;
							while (!selectedList.IsEmpty())
							{
								CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
								HideSources(partfile);
								switch (partfile->GetStatus())
								{
									//case PS_WAITINGFORHASH:
									//case PS_HASHING:
									case PS_COMPLETING:
										//bRemovedItems = true;
										break;
									case PS_COMPLETE:
										if (wParam == MP_CANCEL){
											bool delsucc = ShellDeleteFile(partfile->GetFilePath());
											if (delsucc){
												theApp.sharedfiles->RemoveFile(partfile, true);
												if(!thePrefs.m_bHistoryShowShared){
													theApp.emuledlg->sharedfileswnd->historylistctrl.AddFile(partfile);
													theApp.emuledlg->sharedfileswnd->ShowFilesCount(true);
												}
											}
											else{
												CString strError;
												strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), partfile->GetFilePath(), GetErrorMessage(GetLastError()));
												AfxMessageBox(strError);
											}
										}
										RemoveFile(partfile);
										bRemovedItems = true;
										break;
									case PS_PAUSED:
										partfile->DeleteFile();
										bRemovedItems = true;
										break;
									default:
										if (/*partfile->GetCategory() && */partfile->xState!=POFC_WAITING)// X: [POFC] - [PauseOnFileComplete]
											theApp.downloadqueue->StartNextFileIfPrefs(partfile->GetCategory());
										partfile->DeleteFile();
										bRemovedItems = true;
										break;
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
						CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_HIGH);
					}
					SetRedraw(true);
					break;
				case MP_PRIOLOW:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_LOW);
					}
					SetRedraw(true);
					break;
				case MP_PRIONORMAL:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_NORMAL);
					}
					SetRedraw(true);
					break;
				case MP_PRIOAUTO:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						partfile->SetAutoDownPriority(true);
						partfile->SetDownPriority(PR_HIGH);
					}
					SetRedraw(true);
					break;

				case MP_C0SC:// X: [C0SC] - [Clear0SpeedClient]
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.RemoveHead();
						partfile->ClearClient();
					}
					SetRedraw(true);
					break;

				case MP_PAUSE:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						if (partfile->CanPauseFile()){
							partfile->PauseFile();
						}
					}
					SetRedraw(true);
					break;
				case MP_RESUME:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						if (partfile->CanResumeFile()){
							if (partfile->GetStatus() == PS_INSUFFICIENT)
								partfile->ResumeFileInsufficient();
							else
								partfile->ResumeFile();
						}
					}
					SetRedraw(true);
					break;
				case MP_STOP:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						if (partfile->CanStopFile()){
							HideSources(partfile);
							partfile->StopFile();
						}
					}
					SetRedraw(true);
					theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					break;
				case MP_CLEARCOMPLETED:
					SetRedraw(false);
					ClearCompleted();
					SetRedraw(true);
					break;

				case MP_AICHHASH:// X: [IP] - [Import Parts]
					if (selectedCount == 1)
						file->GetAICHRecoveryHashSet()->TrustMajority();
					break;
				case MP_FLUSHBUFFER:{// X: [FB] - [FlushBuffer]
					SetRedraw(false);
					DWORD curTick = ::GetTickCount();
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.RemoveHead();
						// Avoid flushing while copying preview file
						if((partfile->m_nTotalBufferData > 0) && !partfile->m_bPreviewing){
							partfile->m_nNextFlushBufferTime = curTick;
							curTick += 1000;
						}
					}
					SetRedraw(true);
					break;
				}
				//Xman manual file allocation (Xanatos)
				case MP_PREALOCATE:
					while (!selectedList.IsEmpty()){
						CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
						if(partfile->IncompleteAllocateSpace())
							partfile->AllocateNeededSpace();
					}
					break;
				//Xman end

				//Xman Xtreme Downloadmanager
				case MP_ALL_A4AF_TO_THIS:
					{
						SetRedraw(false);
						bool redraw = false;
						if (selectedCount == 1 
							&& (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY))
						{
							//theApp.downloadqueue->DisableAllA4AFAuto();

							for (POSITION pos = file->A4AFsrclist.GetHeadPosition();pos!=NULL;){ // X: [CI] - [Code Improvement]
								CUpDownClient *cur_source = file->A4AFsrclist.GetNext(pos);
								if( cur_source->GetDownloadState() != DS_DOWNLOADING
									&& cur_source->GetRequestFile() 
									&& ( (!cur_source->GetRequestFile()->IsA4AFAuto()) || cur_source->GetDownloadState() == DS_NONEEDEDPARTS) //Xman Xtreme Downloadmanager: Auto-A4AF-check
									&& !cur_source->IsSwapSuspended(file) )
								{
									redraw = cur_source->SwapToAnotherFile(true, false, false, file,true);
								}
							}

						}
						SetRedraw(true);
						if(redraw)
							file->UpdateDisplayedInfo(true);
						break;
					}
				case MP_DROPNONEEDEDSRCS: { 
					if(selectedCount > 1){
						while (!selectedList.IsEmpty()) {
							selectedList.RemoveHead()->RemoveNoNeededPartsSources();//DS_NONEEDEDPARTS DL-6// X: [CI] - [Code Improvement]
						}
						break;
					}
					file->RemoveNoNeededPartsSources();
					break;					
										  }
				case MP_DROPQUEUEFULLSRCS: { 
					if(selectedCount > 1){
						while (!selectedList.IsEmpty()) {
							selectedList.RemoveHead()->RemoveQueueFullSources();// X: [CI] - [Code Improvement]
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
							selectedList.RemoveHead()->RemoveLeecherSources();// X: [CI] - [Code Improvement]
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
								if(cur_src->GetDownloadState() != DS_DOWNLOADING)// X: skip downloading source
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
								CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
								if (partfile->IsPartFile()) {
									partfile->SetFileName(CleanupFilename(partfile->GetFileName()));
								}
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
								//file->UpdateDisplayedInfo();
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
						str += ((CAbstractFile*)selectedList.RemoveHead())->GetED2kLink();// X: [CI] - [Code Improvement]
					}
					theApp.CopyTextToClipboard(str);
					break;
				}
				case MP_SEARCHRELATED:{
					CAtlList<CAbstractFile*> abstractFileList;
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
						abstractFileList.AddTail(selectedList.GetNext(pos));
					theApp.emuledlg->searchwnd->SearchRelatedFiles(abstractFileList);
					theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
					break;
				}
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
						if (/*pPartFile->IsPreviewableFileType() && */!pPartFile->IsReadyForPreview())
							pPartFile->SetPauseOnPreview(!bAllPausedOnPreview);
						
					}					
					break;
				}		
				case MP_VIEWFILECOMMENTS:
					ShowFileDialog(IDD_COMMENTLST);
					break;
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
							CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
							partfile->SetPrivateMaxSources(newlimit);
							partfile->UpdateDisplayedInfo(true);
						}
					}
					break;
				}
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
				default:
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){
						theWebServices.RunURL(file, wParam);
					}
					else if ((wParam >= MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99) || wParam == MP_NEWCAT){
						UINT nCatNumber;
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
							CPartFile *partfile = selectedList.RemoveHead();// X: [CI] - [Code Improvement]
							partfile->SetCategory(nCatNumber);
							partfile->UpdateDisplayedInfo(true);
						}
						SetRedraw(TRUE);
						UpdateCurrentCategoryView();
						if (thePrefs.ShowCatTabInfos())
							theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					}
					else if (wParam>=MP_PREVIEW_APP_MIN && wParam<=MP_PREVIEW_APP_MAX){
						thePreviewApps.RunApp(file, (UINT)wParam);
					}
					break;
			}
		}
		else{
			CUpDownClient* client = (CUpDownClient*)content->value;

			// <CB Mod : ContextMenu>
			// It may happen that the client is removed from the queue when the a selection has been done
			if (!client)
				return true;
			// </CB Mod : ContextMenu>
		
			CPartFile* file = (CPartFile*)content->owner; //Xman Xtreme Downloadmanager

			switch (wParam){

				//Xman Xtreme Downloadmanager
				case MP_STOP_CLIENT: 
					client->StopClient();
					break;		
				case MP_SWAP_A4AF_TO_THIS: { 
					if(file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if(!client->GetDownloadState() == DS_DOWNLOADING)
						{
							if(client->SwapToAnotherFile(true, true, false, file,true))
								file->UpdateDisplayedInfo(true);
						}
					}
					break;
										   }
				case MP_SWAP_A4AF_TO_OTHER:
					if (/*(client != NULL)  && */!(client->GetDownloadState() == DS_DOWNLOADING)){
						if(client->SwapToAnotherFile(true, true, false, NULL,true))
							file->UpdateDisplayedInfo(true);
					}
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
						client->UpdateDisplayedInfo(true);
					break;
				//Xman friendhandling
				case MP_REMOVEFRIEND:
					if (/*client && */client->IsFriend())
					{
						theApp.friendlist->RemoveFriend(client->m_Friend);
						client->UpdateDisplayedInfo(true);
					}
					break;
				case MP_FRIENDSLOT: 
					//if (client)
					{
						bool IsAlready;				
						IsAlready = client->GetFriendSlot();
						theApp.friendlist->RemoveAllFriendSlots();
						if( !IsAlready )
							client->SetFriendSlot(true);
						client->UpdateDisplayedInfo(true);
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
					//if (client != NULL)
					//{
						client->ShowRequestedFiles(); 
					//}
					break;
				  }
			  //Xman end
/*
// ZZ:DownloadManager -->
				case MP_A4AF_CHECK_THIS_NOW:
					if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if (client->GetDownloadState() != DS_DOWNLOADING)
						{
							client->SwapToAnotherFile(_T("Manual init of source check. Test to be like ProcessA4AFClients(). CDownloadListCtrl::OnCommand() MP_SWAP_A4AF_DEBUG_THIS"), false, false, false, NULL, true, true, true); // ZZ:DownloadManager
							UpdateItem(file);
						}
					}
					break;
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
			/*case 9:
				// Keep the current 'm_bRemainSort' for that column, but reset to 'ascending'
				sortAscending = true;
				break;*/
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

	//int dwOrgSort = lParamSort; // SLUGFILLER: multiSort remove - handled in parent class
	bool sortMod = true;
	if (lParamSort >= 100)
	{
		sortMod = false;
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
			return sortMod ? comp:-comp;
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
	if (comp == 0 && (dwNextSort = theApp.emuledlg->transferwnd->GetDownloadList()->GetNextSortOrder(dwOrgSort)) != (-1)){
		return SortProc(lParam1, lParam2, dwNextSort);
	}
	else
	*/
	return sortMod ? comp:-comp;
}

void CDownloadListCtrl::ClearCompleted(size_t incat){
	if (incat == (size_t)-2)
		incat = curTab;

	// Search for completed file(s)
	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if(file->IsPartFile() == false && (incat==(size_t)-1 || file->CheckShowItemInGivenCat(incat)) ){
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
		for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); )
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
	SetExtendedStyle(thePrefs.IsDoubleClickEnabled()?
		LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP:
		LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);
}

void CDownloadListCtrl::OnListModified(NMHDR *pNMHDR, LRESULT * /*pResult*/)
{
	NMLISTVIEW *pNMListView = (NMLISTVIEW *)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);
	m_availableCommandsDirty = true;
}

int CDownloadListCtrl::Compare(const CPartFile *file1, const CPartFile *file2, LPARAM lParamSort)
{
	switch (lParamSort)
	{
		case 0: //filename asc
			return CompareLocaleStringNoCase(file1->GetFileName(),file2->GetFileName());
		case 1: //size asc
			return CompareUnsigned64(file1->GetFileSize(), file2->GetFileSize());
		case 2: //transferred asc
			return CompareUnsigned64(file1->GetTransferred(), file2->GetTransferred());
		case 3: //completed asc
			return CompareUnsigned64(file1->GetCompletedSize(), file2->GetCompletedSize());
		case 4: //speed asc
			return CompareUnsigned(file1->GetDownloadDatarate(), file2->GetDownloadDatarate()); //Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		case 5: //progress asc
			return CompareFloat(file1->GetPercentCompleted(), file2->GetPercentCompleted());
		case 6: //sources asc
			return CompareUnsigned_PTR(file1->GetSourceCount(), file2->GetSourceCount());
		case 7: //priority asc
			return CompareUnsigned(file1->GetDownPriority(), file2->GetDownPriority());
		case 8: //Status asc 
			return CompareUnsigned_PTR(file1->getPartfileStatusRang(),file2->getPartfileStatusRang());
		case 9: //Remaining Time asc
		{
			//Make ascending sort so we can have the smaller remaining time on the top 
			//instead of unknowns so we can see which files are about to finish better..
			size_t f1, f2;
			if (!thePrefs.UseSimpleTimeRemainingComputation()){
				f1 = file1->getTimeRemaining();
				f2 = file2->getTimeRemaining();
			}
			else{
				f1 = file1->getTimeRemainingSimple();
				f2 = file2->getTimeRemainingSimple();
			}
			//Same, do nothing.
			if (f1 == f2)
				return 0;

			//If descending, put first on top as it is unknown
			//If ascending, put first on bottom as it is unknown
			if (f1 == (size_t)-1)
				return 1;

			//If descending, put second on top as it is unknown
			//If ascending, put second on bottom as it is unknown
			if (f2 == (size_t)-1)
				return -1;

			//If descending, put first on top as it is bigger.
			//If ascending, put first on bottom as it is bigger.
			return CompareUnsigned_PTR(f1, f2);
		}

		case 90: //Remaining SIZE asc
			return CompareUnsigned64(file1->leftsize, file2->leftsize);
		case 10: //last seen complete asc
			if (file1->lastseencomplete > file2->lastseencomplete)
				return 1;
			if (file1->lastseencomplete < file2->lastseencomplete)
				return -1;
		case 11: //last received Time asc
			if (file1->GetFileDate() > file2->GetFileDate())
				return 1;
			if(file1->GetFileDate() < file2->GetFileDate())
				return -1;

		case 12:
			//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
			return CompareLocaleStringNoCase(	/*(const_cast<CPartFile*>(file1)->GetCategory()!=0)?*/thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->strTitle/*:GetResString(IDS_ALL)*/,
											/*(const_cast<CPartFile*>(file2)->GetCategory()!=0)?*/thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->strTitle/*:GetResString(IDS_ALL) */);// X: [UIC] - [UIChange] change cat0 Title

		case 13: // addeed on asc
			if (file1->GetCrCFileDate() > file2->GetCrCFileDate())
				return 1;
			if(file1->GetCrCFileDate() < file2->GetCrCFileDate())
				return -1;
		//Xman Xtreme-Downloadmanager: AVG-QR
		case 14:
			return CompareUnsigned(file1->GetAvgQr(), file2->GetAvgQr());
		//Xman end
	}
	return 0;
 }

int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort)
{
	switch (lParamSort)
	{
		case 0: //name asc
			if (client1->GetUserName() && client2->GetUserName())
				return CompareLocaleStringNoCase(client1->GetUserName(), client2->GetUserName());
			if (!client1->GetUserName())
				return 1; // place clients with no usernames at bottom
			if (!client2->GetUserName())
				return -1; // place clients with no usernames at bottom
			return 0;

		case 1: //size but we use status asc
			return client1->GetSourceFrom() - client2->GetSourceFrom();

		case 2://transferred asc
		case 3://completed asc
			return CompareUnsigned(client1->GetTransferredDown(), client2->GetTransferredDown());

		case 4: //speed asc
			return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());

		case 5: //progress asc
			if(client1->GetHisCompletedPartsPercent_Down() == client2->GetHisCompletedPartsPercent_Down()){// X: more accuracy
				if(client1->GetPartStatus() == NULL)
					return 0;
				ASSERT(client2->GetPartStatus() != NULL);
				int res = CompareUnsigned(client1->GetAvailablePartCount(), client2->GetAvailablePartCount());
				if(res != 0)
					return res;
				if(client1->GetPartStatus()[client1->GetPartCount()-1] == client2->GetPartStatus()[client2->GetPartCount()-1])
					return 0;
				if(client1->GetPartStatus()[client1->GetPartCount()-1])
					return -1;
				return 1;
			}
			if(client1->GetHisCompletedPartsPercent_Down() > client2->GetHisCompletedPartsPercent_Down())
				return 1;
			return -1;
		//Xman
		// Maella -Support for tag ET_MOD_VERSION 0x55-
		case 6:
			if( client1->GetClientSoft() == client2->GetClientSoft() ){
				if(client2->GetVersion() == client1->GetVersion() && (client1->GetClientSoft() == SO_EMULE || client1->GetClientSoft() == SO_AMULE))
					return client1->DbgGetFullClientSoftVer().CompareNoCase( client2->DbgGetFullClientSoftVer());
				return CompareUnsigned(client1->GetVersion(), client2->GetVersion());
			}
			return CompareUnsigned(client2->GetClientSoft(), client1->GetClientSoft());
		// Maella end
		case 7: //qr asc
/*			if (client1->m_nDownloadState == DS_DOWNLOADING){
				if (client2->m_nDownloadState == DS_DOWNLOADING)
					return 0; 
				else
					return -1;
			}
			else if (client2->m_nDownloadState == DS_DOWNLOADING)
				return 1;
			if (client1->m_nRemoteQueueRank == 0 
				&& client1->m_nDownloadState == DS_ONQUEUE && client1->m_bRemoteQueueFull == true)
				return 1;
			if (client2->m_nRemoteQueueRank == 0 
				&& client2->m_nDownloadState == DS_ONQUEUE && client2->m_bRemoteQueueFull == true)
				return -1;
			if (client1->m_nRemoteQueueRank == 0)
				return 1;
			if (client2->m_nRemoteQueueRank == 0)
				return -1;
			return CompareUnsigned(client1->m_nRemoteQueueRank, client2->m_nRemoteQueueRank);*/
			if(client1->GetCountryFlagIndex() == client2->GetCountryFlagIndex()){
				if(client1->m_structUserLocation == client2->m_structUserLocation)
					return 0;
				return CompareLocaleString(client1->m_structUserLocation->locationName, client2->m_structUserLocation->locationName);
			}
			return CompareLocaleString(client1->m_structUserLocation->country->LongCountryName, client2->m_structUserLocation->country->LongCountryName);// X: [IP2L] - [IP2Location]
		case 8:
			if (client1->GetDownloadState() != client2->GetDownloadState())
				return client1->GetDownloadState() - client2->GetDownloadState();
			if(client1->GetDownloadState() == DS_ONQUEUE){
				if (client1->IsRemoteQueueFull() && client2->IsRemoteQueueFull())
					return 0;
				if (client1->IsRemoteQueueFull()||client1->GetRemoteQueueRank() == 0)
					return 1;
				if (client2->IsRemoteQueueFull()||client2->GetRemoteQueueRank() == 0)
					return -1;
				return CompareUnsigned(client1->GetRemoteQueueRank(), client2->GetRemoteQueueRank());
			}
			//else
				//return 0;
		//Xman DiffQR
		//case 14:
/*			if (client1->m_nRemoteQueueRank == 0)
				return 1;
			if (client2->m_nRemoteQueueRank == 0)
				return -1;
			return client1->GetDiffQR() - client2->GetDiffQR();*/
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
								&& pt.x >= sm_iSubItemInset + 16 
								&& pt.x <= sm_iSubItemInset + 16 + RATING_ICON_WIDTH)
								ShowFileDialog(IDD_COMMENTLST);
							else if (thePrefs.GetPreviewOnIconDblClk()
									 && pt.x >= sm_iSubItemInset 
									 && pt.x < sm_iSubItemInset + 16) {
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
	if (m_DropMenu)
		VERIFY( m_DropMenu.DestroyMenu() ); //Xman Xtreme Downloadmanager
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
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPNONEEDEDSRCS, GetResString(IDS_DROPNONEEDEDSRCS)); 
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPQUEUEFULLSRCS, GetResString(IDS_DROPQUEUEFULLSRCS)); 
	m_DropMenu.AppendMenu(MF_STRING, MP_DROPLEECHER, GetResString(IDS_DROPLEECHER));  //Xman Anti-Leecher


	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_DropMenu.m_hMenu, GetResString(IDS_SubMenu_Drop));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	//Xman end

	// Add 'Download Priority' sub menu
	//
	m_PrioMenu.CreateMenu();
	m_PrioMenu.AddMenuTitle(NULL, true);
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(')'));

	// Add file commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE));
	m_FileMenu.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP));
	m_FileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME));
	m_FileMenu.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_MAIN_BTN_CANCEL));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_OPEN, GetResString(IDS_DL_OPEN));
	
	// Extended: Submenu with Preview options, Normal: Preview and possibly 'Preview with' item 
	if (thePrefs.IsExtControlsEnabled())
	{
		m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW));
		m_FileMenu.AppendMenu(MF_STRING, MP_PAUSEONPREVIEW, GetResString(IDS_PAUSEONPREVIEW));
		if (!thePrefs.GetPreviewPrio())
    		m_FileMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS));
	}
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW));
		m_FileMenu.AppendMenu(MF_SEPARATOR);

	m_FileMenu.AppendMenu(MF_STRING, MP_METINFO, GetResString(IDS_DL_INFO));
	m_FileMenu.AppendMenu(MF_STRING, MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR));


	// Add (extended user mode) 'Source Handling' sub menu
	//
	if (thePrefs.IsExtControlsEnabled()) {
		m_SourcesMenu.CreateMenu();
		//Xman Xtreme Downloadmanager
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_AUTO, GetResString(IDS_ALL_A4AF_AUTO)); //Xman Xtreme Downloadmanager: Auto-A4AF-check
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_THIS, GetResString(IDS_ALL_A4AF_TO_THIS)); 
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ALL_A4AF_TO_OTHER, GetResString(IDS_ALL_A4AF_TO_OTHER)); 
		//Xman end
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY));
		m_SourcesMenu.AppendMenu(MF_STRING, MP_SETSOURCELIMIT, GetResString(IDS_SETPFSLIMIT));
		m_SourcesMenu.AppendMenu(MF_STRING, MP_C0SC, GetResString(IDS_CLEAR0KCLIENT));// X: [C0SC] - [Clear0SpeedClient]
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_A4AF));
	}
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Add 'Copy & Paste' commands
	//
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_FileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1));
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK));
	m_FileMenu.AppendMenu(MF_STRING, MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Search commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_FIND, GetResString(IDS_FIND));
	m_FileMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED));
	if( thePrefs.IsExtControlsEnabled()){
		m_FileMenu.AppendMenu(MF_STRING, MP_FLUSHBUFFER, GetResString(IDS_TRY2FLUSH)); // X: [FB] - [FlushBuffer]
		m_FileMenu.AppendMenu(MF_STRING, MP_PREALOCATE, GetResString(IDS_PREALLDISCSPACE)); 
	}

	// Web-services and categories will be added on-the-fly..
}

CString CDownloadListCtrl::getTextList()
{
	CString out;

	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			const CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);

			CString temp;
			temp.Format(_T("\n%s\t [%.2f%%] %i/%i - %s"),
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
				&& pt.x >= sm_iSubItemInset + 16 
				&& pt.x <= sm_iSubItemInset + 16 + RATING_ICON_WIDTH)
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

size_t CDownloadListCtrl::GetCompleteDownloads(size_t cat/*, int& total*/)// X: [CI] - [Code Improvement]
{
	size_t count = 0;
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			/*const*/ CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (cat == (size_t)-1 || file->CheckShowItemInGivenCat(cat))
			{
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

void CDownloadListCtrl::ChangeCategory(size_t newsel){

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
	thePrefs.lastTranWndCatID = curTab = newsel; // X: [RCI] - [Remember Catalog ID]
	theApp.emuledlg->transferwnd->UpdateFilesCount();
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

void CDownloadListCtrl::GetDisplayedFiles(CAtlArray<CPartFile*> *list){
	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			list->Add(file);
		}
	}	
}

void CDownloadListCtrl::MoveCompletedfilesCat(size_t from, size_t to)
{
	size_t mycat;

	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator.
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (!file->IsPartFile()){
				mycat=file->GetCategory();
				if ((from>to && mycat>=to && mycat<=from)// X: [CI] - [Code Improvement]
					|| (from<to && mycat>=from && mycat<=to)) {
					file->SetCategory(mycat == from?to:((from < to)?mycat - 1:mycat + 1)); 
				}
			}
		}
	}
}

void CDownloadListCtrl::OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (CemuleDlg::IsRunning()) {
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
						uJitteredFileReaskTime *=2;

					if(client->HasTooManyFailedUDP() || client->GetDownloadState()==DS_NONEEDEDPARTS || (client->HasLowID() && !(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())))
					{
						if ( (uJitteredFileReaskTime+client->GetLastAskedTime()) < ::GetTickCount() )
							askbuffer=_T(": 0\n");
						else
							if(client->GetDownloadState()==DS_NONEEDEDPARTS || uJitteredFileReaskTime+client->GetLastAskedTime() <= client->GetNextTCPAskedTime())
								askbuffer.Format(_T(": %s\n"),CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000));
							else
								askbuffer.Format(_T(": %s (%s)\n"), CastSecondsToHM((uJitteredFileReaskTime+client->GetLastAskedTime()-::GetTickCount())/1000), CastSecondsToHM((client->GetNextTCPAskedTime()-::GetTickCount())/1000));
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
								//MORPH START - Extra User Infos
								+ GetResString(IDS_CD_CSOFT) + _T(": %s\n")
								+ GetResString(IDS_COUNTRY) + _T(": %s\n")
								//MORPH END   - Extra User Infos
								+ GetResString(IDS_SERVER) + _T(": %s:%u\n\n")
								+ GetResString(IDS_NEXT_REASK) + askbuffer, //Xman Xtreme Downloadmanager
								client->GetUserName() ? client->GetUserName() : (_T('(') + GetResString(IDS_UNKNOWN) + _T(')')),
					                        //MORPH START - Extra User Infos
								client->GetClientSoftVer(),
								client->GetCountryName(),
								//MORPH END   - Extra User Infos
		                                 	        ipstr(server), client->GetServerPort()); //Xman Xtreme Downloadmanager
					info += _T('\n');
					info.AppendFormat(GetResString(IDS_SOURCEINFO), client->GetAskedCountDown(), client->GetAvailablePartCount());
					//Xman Xtreme Downloadmanager
					info.AppendFormat(_T("\nUDP reask possible: %s"), client->HasTooManyFailedUDP() || (client->HasLowID() && !(client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())) ? _T("no") : _T("yes")); //Xman Xtreme-Downloadmanager 
					if(client->HasLowID())
					{
						info.Append((client->GetBuddyIP() && client->GetBuddyPort() && client->HasValidBuddyID())?_T("\nclient has buddy"):_T("\nclient has no buddy"));
						if (client->GetLowIDReaskPening())
							info.Append(_T(", reask pending"));
					}
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

			info.AppendChar(TOOLTIP_AUTOFORMAT_SUFFIX_CH);
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

void* CDownloadListListCtrlItemWalk::GetPrevSelectableItem()
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	ASSERT( m_eItemType != (ItemType)-1 );

	size_t iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pDownloadListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pDownloadListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem > 0)
			{
				iItem--;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item != NULL && (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE)))
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return (void*)ctrl_item->value;
				}
			}
		}
	}
	return NULL;
}

void* CDownloadListListCtrlItemWalk::GetNextSelectableItem()
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	ASSERT( m_eItemType != (ItemType)-1 );

	size_t iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pDownloadListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pDownloadListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while ((size_t)(iItem+1) < iItemCount)
			{
				iItem++;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item != NULL && (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE)))
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return (void*)ctrl_item->value;
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

//Xman end
#ifdef _DEBUG
uint32 CtrlItem_Struct::amount;

void CDownloadListCtrl::PrintStatistic()
{
	AddLogLine(false, _T("DownloadlistControl: Listitems: %u"), m_ListItems.size());
	AddLogLine(false, _T("DownloadlistControl: CtrlItem_Structs: %u"), CtrlItem_Struct::amount);
}
#endif

CImageList *CDownloadListCtrl::CreateDragImage(int /*iItem*/, LPPOINT lpPoint)
{
	const size_t iMaxSelectedItems = 30;
	size_t iSelectedItems = 0;
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
				rcSelectedItems.left = sm_iSubItemInset;
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

bool CDownloadListCtrl::ReportAvailableCommands(CAtlList<int>& liAvailableCommands)
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
			size_t iSelectedItems = 0;
			//size_t iFilesNotDone = 0;
			size_t iFilesToPause = 0;
			size_t iFilesToStop = 0;
			size_t iFilesToResume = 0;
			size_t iFilesToOpen = 0;
            //size_t iFilesGetPreviewParts = 0;
            //size_t iFilesPreviewType = 0;
			size_t iFilesToPreview = 0;
			size_t iFilesToCancel = 0;
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
				//iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
                //iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
                //iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
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
			if (iSelectedItems > 0)
			{
				if (iSelectedItems == 1 && iFilesToOpen == 1)
					liAvailableCommands.AddTail(MP_OPEN);
				if (iSelectedItems == 1 && iFilesToPreview == 1)
					liAvailableCommands.AddTail(MP_PREVIEW);
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
	//int total;
	if (GetCompleteDownloads(curTab/*, total*/) > 0)
		liAvailableCommands.AddTail(MP_CLEARCOMPLETED);
	if (GetItemCount() > 0)
		liAvailableCommands.AddTail(MP_FIND);
	liAvailableCommands.AddTail(MP_HM_OPENINC); //>>> WiZaRd::OpenIncoming
	return true;
}
