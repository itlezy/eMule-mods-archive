//this file is part of eMule
// added by itsonlyme
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

// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->

#include "StdAfx.h"
#include "emule.h"
#include "emuledlg.h"
#include "SearchList.h"
#include "KnownFile.h"
#include "PartFile.h"
#include "SharedFilesPage.h"
#include "FileDetailDialog.h"
#include "SharedFilesCtrl.h"
#include "SearchListCtrl.h"
#include "PreviewDlg.h"
#include "CommentDialog.h"
#include "IrcWnd.h"
#include "InputBox.h" // NEO: VSF - [VirtualSharedFiles]
#include "Neo/CP/PreferencesDlg.h" // NEO: VSF - [VirtualSharedFiles]
#include "ClientList.h"
#include "SharedFileList.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "MenuCmds.h"
#include "WebServices.h"
#include "Log.h"
#include "UserMsgs.h"
#ifdef A4AF_CATS // NEO: MAC - [MorphA4AFCategories] -- Xanatos -->
#include "TransferWnd.h"
#include "Neo/GUI/SelCategoryDlg.h"
#endif // A4AF_CATS // NEO: MAC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "Collection.h"
#include "CollectionCreateDialog.h"
#include "CollectionViewDialog.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "sockets.h"
#include "server.h"
#include "Neo/NeoPreferences.h"
#include "Neo\GUI\XPMenu\MenuXP.h" // NEO: NMX - [NeoMenuXP] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)

#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
)

#define		MP_SFP_REFRESH_ROOT		1
#define		MP_SFP_REFRESH_DIR		2

enum EBLCType {
	BLCT_UNKNOWN = 0,
	BLCT_ROOT = 1,
	BLCT_FILE = 2,
	BLCT_DIR = 3,
	BLCT_LOCALFILE = 4
};

struct BLCItem_struct {
	BLCItem_struct ();
	~BLCItem_struct ();
	EBLCType		m_eItemType;
	CString			m_fullPath;
	CString			m_origPath;
	CString			m_name;
	CSearchFile*	m_file;
	CKnownFile*		m_knownFile;
};

BLCItem_struct::BLCItem_struct ()
{
	m_eItemType = BLCT_UNKNOWN;
	m_file = NULL;
	m_knownFile = NULL;
}

BLCItem_struct::~BLCItem_struct ()
{
	if (m_file)
		delete m_file;
}

//////////////////////////////////////////////////////////////////////////////
// CSharedFilesPage dialog

IMPLEMENT_DYNAMIC(CSharedFilesPage, CResizablePage)

BEGIN_MESSAGE_MAP(CSharedFilesPage, CResizablePage)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_EXT_OPTS, OnExtOptsDblClick)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_EXT_OPTS, OnItemExpanding)
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	ON_NOTIFY_EX_RANGE(UDM_TOOLTIP_DISPLAY, 0, 0xFFFF, OnToolTipNotify)
#else
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	ON_NOTIFY(NM_RCLICK, IDC_EXT_OPTS, OnExtOptsRightClick)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_WM_MEASUREITEM() // NEO: NMX - [NeoMenuXP] <-- Xanatos --
END_MESSAGE_MAP()

CSharedFilesPage::CSharedFilesPage()
	: CResizablePage(CSharedFilesPage::IDD, 0)
	, m_ctrlTree(theApp.m_iDfltImageListColorFlags)
{
	m_paClients = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_SHAREDFILES);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_htiRoot = NULL;
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	m_toolTip = NULL;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	m_bLocalFiles = false;
	m_timer = 0;
}

CSharedFilesPage::~CSharedFilesPage()
{
}

void CSharedFilesPage::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTree);
}

BOOL CSharedFilesPage::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_EXT_OPTS,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_WARNING,TOP_LEFT,TOP_RIGHT);

	Localize();

	m_ctrlTree.ModifyStyle(0, TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT, 0);
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.Create(this);
	m_ttip.AddTool(&m_ctrlTree, _T(""));	
	//m_othertips.Create(this);
	SetTTDelay();	
#else
	m_toolTip = new CToolTipCtrl;
	m_toolTip->Create(this, TTS_ALWAYSTIP);
	m_toolTip->SetDelayTime(TTDT_AUTOPOP, 10000);
	m_toolTip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000); 
	m_toolTip->SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX); // recognize \n chars!
	m_toolTip->AddTool(&m_ctrlTree);
	m_ctrlTree.SetToolTips(m_toolTip);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	m_iImgRoot = 8;
	m_iImgShDir = 8;
	m_iImgDir = 8;

	CImageList* imageList = m_ctrlTree.GetImageList(TVSIL_NORMAL);
	if (imageList) {
		m_iImgRoot = imageList->Add(CTempIconLoader(_T("SharedFiles")));
		m_iImgShDir = imageList->Add(CTempIconLoader(_T("SharedDir")));
		m_iImgDir = imageList->Add(CTempIconLoader(_T("FOLDERS")));
	}

	VERIFY( (m_timer = SetTimer(301, 1000, 0)) != NULL );

	return TRUE;
}

BOOL CSharedFilesPage::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		if (m_htiRoot)
			UpdateTree(_T(""));
		else
			FillTree();
		m_ctrlTree.Expand(m_htiRoot, TVE_EXPAND);
		m_ctrlTree.SendMessage(WM_VSCROLL, SB_TOP);
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CSharedFilesPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CSharedFilesPage::Localize()
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	CString buffer;
	if (m_bLocalFiles)
		buffer.Format(GetResString(IDS_X_LOCAL_SHARED_FILES));
	else if (client) {
		if (client->GetDeniesShare())
			buffer.Format(GetResString(IDS_X_DENIESSHARE), client->GetUserName());
		else
			buffer.Format(GetResString(IDS_X_SHAREDFILES2), client->GetUserName());
	}
	else
		buffer.Format(GetResString(IDS_SHAREDFILES));
	GetDlgItem(IDC_WARNING)->SetWindowText(buffer);

	m_strCaption = GetResString(IDS_SHAREDFILES);
	m_psp.pszTitle = m_strCaption;

	if (m_htiRoot) {
		CString buffer;
		if (!m_bLocalFiles)
			buffer.Format(GetResString(IDS_X_USERSSHAREDFILES), client->GetUserName());
		else
			buffer.Format(GetResString(IDS_X_USERSSHAREDFILES), thePrefs.GetUserNick());
		m_ctrlTree.SetItemText(m_htiRoot, buffer);
	}
}

void CSharedFilesPage::OnDestroy()
{
	m_ctrlTree.DeleteAllItems();
	m_ctrlTree.DestroyWindow();
	for (POSITION pos = m_BLCItem_list.GetHeadPosition(); pos != NULL; ){
		BLCItem_struct *BLCItem = m_BLCItem_list.GetNext(pos);
		delete BLCItem;
	}
	m_BLCItem_list.RemoveAll();
	m_HTIs_map.RemoveAll();
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	if (m_toolTip) {
		m_toolTip->DestroyToolTipCtrl();
		m_toolTip = NULL;
	}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}

	CResizablePage::OnDestroy();
}

void CSharedFilesPage::OnTimer(UINT /*nIDEvent*/)
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	if (!client)
		return;	// no client

	for (POSITION pos = client->m_Dirs2Update.GetHeadPosition();pos != 0;){
		POSITION rempos = pos;
		CString dir = client->m_Dirs2Update.GetNext(pos);
		client->m_Dirs2Update.RemoveAt(rempos);
		UpdateTree(dir);
		if(dir.IsEmpty()){
			client->m_Dirs2Update.RemoveAll();
			break;
		}
	}
}

void CSharedFilesPage::OnExtOptsDblClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (m_ctrlTree.GetSelectedItem() == NULL)
		return;
	HTREEITEM hti = m_ctrlTree.GetSelectedItem();

	BLCItem_struct *BLCItem = (BLCItem_struct *)m_ctrlTree.GetUserItemData(hti);
	ASSERT(BLCItem);

	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	if (!client)
		return;	// no client

	switch (BLCItem->m_eItemType) {
	case BLCT_ROOT:
		if (client->RequestSharedFileList()) {
			client->SetDeniesShare(false);
			Localize();
			m_ctrlTree.SetItemText(hti, m_ctrlTree.GetItemText(hti) + GetResString(IDS_X_GETTING_FILE_LIST));
		}
		break;
	case BLCT_DIR:
		if (client->SendDirRequest(BLCItem->m_fullPath))
			m_ctrlTree.SetItemText(hti, m_ctrlTree.GetItemText(hti) + GetResString(IDS_X_GETTING_FILE_LIST));
		break;
	case BLCT_FILE:
		if (GetKeyState(VK_MENU) & 0x8000){
			if (BLCItem->m_file){
				CSimpleArray<CAbstractFile*> paFiles;
				paFiles.Add(BLCItem->m_file);
				// NEO: MLD - [ModelesDialogs]
				CFileDetailDialog* dialog = new CFileDetailDialog(&paFiles); 
				dialog->OpenDialog();
				// NEO: MLD END
				//CFileDetailDialog dialog(&paFiles);
				//dialog.DoModal();
			}
		}
		else {
			if (BLCItem->m_file){
#ifdef A4AF_CATS // NEO: MAC - [MorphA4AFCategories] -- Xanatos -->
				int useCat = 0;
				bool	bCreatedNewCat = false;
				if (thePrefs.SelectCatForNewDL())
				{
					CSelCategoryDlg* getCatDlg = new CSelCategoryDlg((CWnd*)theApp.emuledlg);
					getCatDlg->DoModal();

					// Returns 0 on 'Cancel', otherwise it returns the selected category
					// or the index of a newly created category.  Users can opt to add the
					// links into a new category.
					useCat = getCatDlg->GetInput();
					bCreatedNewCat = getCatDlg->CreatedNewCat();
					bool	bCanceled = getCatDlg->WasCancelled();
					delete getCatDlg;
					if (bCanceled)
						return;
				}
				else if (thePrefs.UseAutoCat())
					useCat = theApp.downloadqueue->GetAutoCat(CString(BLCItem->m_file->GetFileName()), BLCItem->m_file->GetFileSize());
					
				if(!useCat && thePrefs.UseActiveCatForLinks())
					useCat = theApp.emuledlg->transferwnd->GetActiveCategory();

				uint8 bPaused = (uint8)thePrefs.AddNewFilesPaused();

				if (thePrefs.SmallFileDLPush() && BLCItem->m_file->GetFileSize() < thePrefs.SmallFileDLPushSizeB())
					theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file, bPaused, useCat, 0);
				else if (thePrefs.AutoSetResumeOrder())
					theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file, bPaused, useCat, theApp.downloadqueue->GetMaxCatResumeOrder(useCat)+1);
				else
					theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file, bPaused, useCat, theApp.downloadqueue->GetMaxCatResumeOrder(useCat));

				// This bit of code will resume the number of files that the user specifies in preferences (Off by default)
				if (thePrefs.StartDLInEmptyCats() && bCreatedNewCat && bPaused)
					for (int i = 0; i < thePrefs.StartDLInEmptyCatsAmount(); i++)
						if (!theApp.downloadqueue->StartNextFile(useCat)) break;
#else
				theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file);
#endif // A4AF_CATS // NEO: MAC END <-- Xanatos 
			}
			RECT itemRect;
			m_ctrlTree.GetItemRect(hti, &itemRect, false);
			m_ctrlTree.InvalidateRect(&itemRect, false);
		}
		break;
	}
}

void CSharedFilesPage::OnExtOptsRightClick(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	CPoint pt;
	::GetCursorPos(&pt);
	int menuX = pt.x, menuY = pt.y;
	HTREEITEM hItem = GetItemUnderMouse();
	if (!hItem) return;
	m_ctrlTree.Select(hItem, TVGN_CARET);
	if (m_ctrlTree.GetItemData(hItem)) {
		BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hItem);
		if (BLCItem) {
			CString buffer = GetResString(IDS_CMT_REFRESH);
			buffer.Remove('&');
			switch (BLCItem->m_eItemType) {
			case BLCT_ROOT:{
				CTitleMenu menu;
				menu.CreatePopupMenu();
				// NEO: VSF - [VirtualSharedFiles]
				/*if (m_bLocalFiles) {
					menu.AddMenuTitle(GetResString(IDS_X_VDS_MM), true);
					menu.AppendMenu(MF_STRING, MP_IOM_VIRTPREFS, GetResString(IDS_X_VDS_ADVANCED), _T("VIRTUALDIR"));
				}
				// NEO: VSF END
				else*/ 
				if (!m_bLocalFiles){
					menu.AddMenuTitle(buffer);
					menu.AppendMenu(MF_STRING, MP_SFP_REFRESH_ROOT, GetResString(IDS_X_REFRESH_ROOT));
				}
				menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, menuX, menuY, this);
				VERIFY( menu.DestroyMenu() );
				*pResult = 0;
				return;
				}
			case BLCT_DIR:
				{
				//CTitleMenu menu;
				CMenuXP menu; // NEO: NMX - [NeoMenuXP] <-- Xanatos --
				menu.CreatePopupMenu();
				// NEO: VSF - [VirtualSharedFiles]
				if (m_bLocalFiles) {
					menu.AddMenuTitle(GetResString(IDS_X_VDS_MM), true);
					menu.AppendMenu(MF_STRING, MP_IOM_VIRTDIR, GetResString(IDS_X_VDS_MDIR));
					menu.AppendMenu(MF_STRING, MP_IOM_VIRTSUBDIR, GetResString(IDS_X_VDS_MSDIR));
					menu.EnableMenuItem(MP_IOM_VIRTDIR, !BLCItem->m_origPath.IsEmpty() ? MF_ENABLED : MF_GRAYED);
					menu.EnableMenuItem(MP_IOM_VIRTSUBDIR, !BLCItem->m_origPath.IsEmpty() ? MF_ENABLED : MF_GRAYED);
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING, MP_IOM_VIRTREMOVE, GetResString(IDS_X_VDS_REMOVE));
					//menu.AppendMenu(MF_STRING, MP_IOM_VIRTPREFS, GetResString(IDS_X_VDS_ADVANCED), _T("VIRTUALDIR"));
					bool bVirtRemove = false;
					CString virt;
					CString path = BLCItem->m_origPath;
					path.MakeLower();
					path.TrimRight(_T('\\'));
					bVirtRemove = thePrefs.m_dirToVDir_map.Lookup(path, virt);
					if (!bVirtRemove)
						bVirtRemove = thePrefs.m_dirToVDirWithSD_map.Lookup(path, virt);
					menu.EnableMenuItem(MP_IOM_VIRTREMOVE, bVirtRemove ? MF_ENABLED : MF_GRAYED);
				}
				// NEO: VSF END
				else {
					menu.AddMenuTitle(buffer);
					menu.AppendMenu(MF_STRING, MP_SFP_REFRESH_DIR, GetResString(IDS_X_REFRESH_DIR));
				}
				menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, menuX, menuY, this);
				VERIFY( menu.DestroyMenu() );
				*pResult = 0;
				return;
				}
			case BLCT_FILE:
				{
				//CTitleMenu menu;
				CMenuXP menu; // NEO: NMX - [NeoMenuXP] <-- Xanatos --
				menu.CreatePopupMenu();
				menu.AddMenuTitle(GetResString(IDS_FILE), true);
				menu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DOWNLOAD), _T("RESUME"));

				if (thePrefs.IsExtControlsEnabled())
					menu.AppendMenu(MF_STRING, MP_RESUMEPAUSED, GetResString(IDS_DOWNLOAD) + _T(" (") + GetResString(IDS_PAUSED) + _T(")"));

				menu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
				menu.EnableMenuItem(MP_PREVIEW, BLCItem->m_file->IsPreviewPossible() ? MF_ENABLED : MF_GRAYED);
				if (thePrefs.IsExtControlsEnabled())
					menu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
				menu.AppendMenu(MF_STRING,MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), _T("FILECOMMENTS"));
				menu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED), _T("KadFileSearch"));
				menu.EnableMenuItem(MP_SEARCHRELATED, (theApp.serverconnect->IsConnected() && theApp.serverconnect->GetCurrentServer() != NULL && theApp.serverconnect->GetCurrentServer()->GetRelatedSearchSupport())?MF_ENABLED:MF_DISABLED);
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
				menu.AppendMenu(MF_STRING,MP_GETHTMLED2KLINK, GetResString(IDS_DL_LINK2), _T("ED2KLINK"));
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING, MP_FIND, GetResString(IDS_FIND), _T("Search"));
				menu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
				menu.EnableMenuItem(MP_PREVIEW, BLCItem->m_file->IsPreviewPossible() ? MF_ENABLED : MF_GRAYED);


				CTitleMenu WebMenu;
				WebMenu.CreateMenu();
				WebMenu.AddMenuTitle(NULL, true);
				int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
				UINT flag2 = (iWebMenuEntries == 0) ? MF_GRAYED : MF_STRING;
				menu.AppendMenu(MF_POPUP | flag2, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

				menu.SetDefaultItem( ( !thePrefs.AddNewFilesPaused() || !thePrefs.IsExtControlsEnabled() )?MP_RESUME:MP_RESUMEPAUSED);

				menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, menuX, menuY, this);

				VERIFY( WebMenu.DestroyMenu() );
				VERIFY( menu.DestroyMenu() );
				*pResult = 0;
				return;
				}
			case BLCT_LOCALFILE:
				{
				CKnownFile* file = BLCItem->m_knownFile;

				// NEO: NMX - [NeoMenuXP] -- Xanatos -->
				//CTitleMenu menu;
				CMenuXP		menu;
				menu.CreatePopupMenu();
				CMenuXP		CollectionsMenu;
				CTitleMenu	PrioMenu;
				CTitleMenu	PermMenu;
				CMenuXP		VirtualDirMenu;
				// NEO: NMX END <-- Xanatos --

				// add priority switcher
				PrioMenu.CreateMenu();
				PrioMenu.AddMenuTitle(NULL, true);
				PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
				PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
				PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
				PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
				PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_X_PRIOVERYHIGH));  // NEO: MOD - [ForSRS]
				PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP
				PrioMenu.AppendMenu(MF_SEPARATOR);
				PrioMenu.AppendMenu(MF_STRING,MP_PRIORELEASE, GetResString(IDS_PRIORELEASE)); // NEO: SRS - [SmartReleaseSharing]

				UINT uCurPrioMenuItem = 0;
				if (file->IsAutoUpPriority())
					uCurPrioMenuItem = MP_PRIOAUTO;
				else if (file->GetUpPriority() == PR_VERYLOW)
					uCurPrioMenuItem = MP_PRIOVERYLOW;
				else if (file->GetUpPriority() == PR_LOW)
					uCurPrioMenuItem = MP_PRIOLOW;
				else if (file->GetUpPriority() == PR_NORMAL)
					uCurPrioMenuItem = MP_PRIONORMAL;
				else if (file->GetUpPriority() == PR_HIGH)
					uCurPrioMenuItem = MP_PRIOHIGH;
				else if (file->GetUpPriority() == PR_VERYHIGH)
					uCurPrioMenuItem = MP_PRIOVERYHIGH;
				else
					ASSERT(0);

				PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uCurPrioMenuItem, 0);
				PrioMenu.CheckMenuItem(MP_PRIORELEASE, file->IsReleasePriority() ? MF_CHECKED : MF_UNCHECKED); // NEO: SRS - [SmartReleaseSharing]


				PermMenu.CreateMenu();
				PermMenu.AddMenuTitle(NULL, true);
				PermMenu.AppendMenu(MF_STRING,MP_PERMDEFAULT, GetResString(IDS_X_PERM_DEFAULT));
				PermMenu.AppendMenu(MF_STRING,MP_PERMNONE, GetResString(IDS_X_PERM_HIDDEN));
				PermMenu.AppendMenu(MF_STRING,MP_PERMFRIENDS, GetResString(IDS_X_PERM_FRIENDS));
				PermMenu.AppendMenu(MF_STRING,MP_PERMALL, GetResString(IDS_X_PERM_PUBLIC));

				UINT uCurPermMenuItem = 0;
				if (file->GetPermissions() == PERM_DEFAULT)
					uCurPermMenuItem = MP_PERMDEFAULT;
				else if (file->GetPermissions() == PERM_ALL)
					uCurPermMenuItem = MP_PERMALL;
				else if (file->GetPermissions() == PERM_FRIENDS)
					uCurPermMenuItem = MP_PERMFRIENDS;
				else if (file->GetPermissions() == PERM_NONE)
					uCurPermMenuItem = MP_PERMNONE;
				else
					ASSERT(0);

				PermMenu.CheckMenuRadioItem(PERM_DEFAULT, MP_PERMNONE, uCurPermMenuItem, 0);
				// NEO: SSP END

				CollectionsMenu.CreateMenu();
				CollectionsMenu.AddMenuTitle(NULL, true);
				CollectionsMenu.AppendMenu(MF_STRING,MP_CREATECOLLECTION, GetResString(IDS_CREATECOLLECTION), _T("COLLECTION_ADD"));
				if (file->m_pCollection) {
					CollectionsMenu.AppendMenu(MF_STRING,MP_MODIFYCOLLECTION, GetResString(IDS_MODIFYCOLLECTION), _T("COLLECTION_EDIT"));
					CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION), _T("COLLECTION_VIEW"));
					CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION), _T("COLLECTION_SEARCH"));
				}

				// NEO: VSF - [VirtualSharedFiles]
				VirtualDirMenu.CreateMenu();
				VirtualDirMenu.AddMenuTitle(NULL, true);
				VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTFILE, GetResString(IDS_X_VDS_MFILE), _T("VDFILE"));
				VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTDIR, GetResString(IDS_X_VDS_MDIR), _T("VDDIR"));
				VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTSUBDIR, GetResString(IDS_X_VDS_MSDIR), _T("VDSUBDIR"));
				VirtualDirMenu.AppendMenu(MF_SEPARATOR);
				VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTREMOVE, GetResString(IDS_X_VDS_REMOVE), _T("VDREMOVE"));
				//VirtualDirMenu.AppendMenu(MF_STRING, MP_IOM_VIRTPREFS, GetResString(IDS_X_VDS_ADVANCED), _T("VDSET"));

				bool bVirtRemove = false;
				CString virt, fileID;
				fileID.Format(_T("%I64u:%s"), (uint64)file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
				bVirtRemove = thePrefs.m_fileToVDir_map.Lookup(fileID, virt);
				if (!bVirtRemove) {
					CString path = file->GetPath();
					path.MakeLower();
					path.TrimRight(_T('\\'));
					bVirtRemove = thePrefs.m_dirToVDir_map.Lookup(path, virt);
					if (!bVirtRemove)
						bVirtRemove = thePrefs.m_dirToVDirWithSD_map.Lookup(path, virt);
				}
				VirtualDirMenu.EnableMenuItem(MP_IOM_VIRTREMOVE, bVirtRemove ? MF_ENABLED : MF_GRAYED);
				// NEO: VSF END

				menu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);
				menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
				menu.AppendMenu(MF_STRING|MF_SEPARATOR); 
				// NEO: SSP - [ShowSharePermissions]
				menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)PermMenu.m_hMenu, GetResString(IDS_PERMISSION), _T("SHAREPERM"));
				menu.EnableMenuItem((UINT_PTR)PermMenu.m_hMenu, NeoPrefs.UseShowSharePermissions() ? MF_ENABLED : MF_GRAYED);
				// NEO: SSP END
				menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)VirtualDirMenu.m_hMenu, GetResString(IDS_X_VDS_VIRTDIRTITLE), _T("VIRTUALDIR")); // NEO: VSF - [VirtualSharedFiles]
				menu.AppendMenu(MF_STRING|MF_SEPARATOR);
				menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION), _T("COLLECTION"));
				menu.AppendMenu(MF_STRING|MF_SEPARATOR); 

				menu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
				if (thePrefs.GetShowCopyEd2kLinkCmd())
					menu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_COPY), _T("ED2KLINK") );
				else
					menu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
				//menu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS"));
				menu.AppendMenu(MF_STRING,MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), _T("FILECOMMENTS"));	// XC - [ExtendedComments]
				menu.AppendMenu(MF_STRING|MF_SEPARATOR); 


				CTitleMenu WebMenu;
				WebMenu.CreateMenu();
				WebMenu.AddMenuTitle(NULL, true);
				int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
				UINT flag2 = (iWebMenuEntries == 0) ? MF_GRAYED : MF_STRING;
				menu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

				menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, menuX, menuY, this);

				VERIFY( WebMenu.DestroyMenu() );
				VERIFY( VirtualDirMenu.DestroyMenu() );
				VERIFY( PermMenu.DestroyMenu() );
				VERIFY( PrioMenu.DestroyMenu() );
				VERIFY( CollectionsMenu.DestroyMenu() );
				VERIFY( menu.DestroyMenu() );
				*pResult = 0;
				return;
				}
			}
		}
	}
}

void CSharedFilesPage::OnItemExpanding (NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hti = pNMTreeView->itemNew.hItem;

	*pResult = 0;

	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	if (!client)
		return;	// no client

	BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hti);
	switch (BLCItem->m_eItemType) {
	case BLCT_ROOT:
		if (client->RequestSharedFileList()) {
			client->SetDeniesShare(false);
			Localize();
			m_ctrlTree.SetItemText(hti, m_ctrlTree.GetItemText(hti) + GetResString(IDS_X_GETTING_FILE_LIST));
		}
		break;
	case BLCT_DIR:
		// send dir list request
		if (client->SendDirRequest(BLCItem->m_fullPath))
			m_ctrlTree.SetItemText(hti, m_ctrlTree.GetItemText(hti) + GetResString(IDS_X_GETTING_FILE_LIST));
		break;
	}
}

BOOL CSharedFilesPage::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	HTREEITEM hItem = m_ctrlTree.GetSelectedItem();
	if (hItem == NULL) return false;
	if (!m_ctrlTree.GetItemData(hItem)) return false;
	BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hItem);
	if (!BLCItem) return false;
	switch (wParam)
	{
		case MP_SFP_REFRESH_ROOT: {
			if (BLCItem->m_eItemType == BLCT_ROOT) {
				if (client) {
					if (client->RequestSharedFileList(true)) {
						client->SetDeniesShare(false);
						Localize();
						m_ctrlTree.SetItemText(hItem, m_ctrlTree.GetItemText(hItem) + GetResString(IDS_X_GETTING_FILE_LIST));
					}
				}
			}
			break;
		}
		case MP_SFP_REFRESH_DIR: 
			if (BLCItem->m_eItemType == BLCT_DIR) {
				if (client) {
					if (client->SendDirRequest(BLCItem->m_fullPath, true))
						m_ctrlTree.SetItemText(hItem, m_ctrlTree.GetItemText(hItem) + GetResString(IDS_X_GETTING_FILE_LIST));
				}
			}
			break;
		case MP_RESUMEPAUSED:
		case MP_RESUME:
			if (BLCItem->m_eItemType == BLCT_FILE) {
				if (BLCItem->m_file)
					theApp.downloadqueue->AddSearchToDownload(BLCItem->m_file, wParam==MP_RESUMEPAUSED);
				RECT itemRect;
				m_ctrlTree.GetItemRect(hItem, &itemRect, false);
				m_ctrlTree.InvalidateRect(&itemRect, false);
			}
			break;
		case MPG_ALTENTER:
		case MP_DETAIL:
			if (BLCItem->m_eItemType == BLCT_FILE) 
			{
				if (BLCItem->m_file) {
					CSimpleArray<CAbstractFile*> paFiles;
					paFiles.Add(BLCItem->m_file);
					// NEO: MLD - [ModelesDialogs]
					CFileDetailDialog* dialog = new CFileDetailDialog(&paFiles); 
					dialog->OpenDialog();
					// NEO: MLD END
					//CFileDetailDialog dialog(&paFiles);
					//dialog.DoModal();
				}
			}
			else if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CSimpleArray<CAbstractFile*> paFiles;
				paFiles.Add(BLCItem->m_knownFile);
				// NEO: MLD - [ModelesDialogs]
				CFileDetailDialog* dialog = new CFileDetailDialog(&paFiles); 
				dialog->OpenDialog();
				// NEO: MLD END
				//CFileDetailDialog dialog(&paFiles);
				//dialog.DoModal();
			}
			break;

		case MP_CREATECOLLECTION:
		{
			if (BLCItem->m_eItemType == BLCT_LOCALFILE)
			{
				CCollection* pCollection = new CCollection();
				pCollection->AddFileToCollection(BLCItem->m_knownFile,true);
				// NEO: MLD - [ModelesDialogs]
				CCollectionCreateDialog* dialog = new CCollectionCreateDialog(); 
				dialog->SetCollection(pCollection,true);
				dialog->OpenDialog();
				// NEO: MLD END
				//CCollectionCreateDialog dialog;
				//dialog.SetCollection(pCollection,true);
				//dialog.DoModal();
				//delete pCollection;
			}
			break;
		}
		case MP_SEARCHAUTHOR:
		{
			if (BLCItem->m_eItemType == BLCT_LOCALFILE && BLCItem->m_knownFile->m_pCollection)
			{
				SSearchParams* pParams = new SSearchParams;
				pParams->strExpression = BLCItem->m_knownFile->m_pCollection->GetCollectionAuthorKeyString();
				pParams->eType = SearchTypeKademlia;
				pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
				pParams->strSpecialTitle = BLCItem->m_knownFile->m_pCollection->m_sCollectionAuthorName;
				if (pParams->strSpecialTitle.GetLength() > 50){
					pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
				}
				theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
			}
			break;
		}
		case MP_VIEWCOLLECTION:
		{
			if (BLCItem->m_eItemType == BLCT_LOCALFILE && BLCItem->m_knownFile->m_pCollection)
			{
				// NEO: MLD - [ModelesDialogs]
				CCollectionViewDialog* dialog = new CCollectionViewDialog(); 
				dialog->SetCollection(BLCItem->m_knownFile->m_pCollection);
				dialog->OpenDialog();
				// NEO: MLD END
				//CCollectionViewDialog dialog;
				//dialog.SetCollection(BLCItem->m_knownFile->m_pCollection);
				//dialog.DoModal();
			}
			break;
		}
		case MP_MODIFYCOLLECTION:
		{
			if (BLCItem->m_eItemType == BLCT_LOCALFILE && BLCItem->m_knownFile->m_pCollection)
			{
				// NEO: MLD - [ModelesDialogs]
				CCollectionCreateDialog* dialog = new CCollectionCreateDialog(); 
				CCollection* pCollection = new CCollection(BLCItem->m_knownFile->m_pCollection);
				dialog->SetCollection(pCollection,false);
				dialog->OpenDialog();
				// NEO: MLD END
				//CCollectionCreateDialog dialog;
				//CCollection* pCollection = new CCollection(BLCItem->m_knownFile->m_pCollection);
				//dialog.SetCollection(pCollection,false);
				//dialog.DoModal();
				//delete pCollection;
			}
			break;
		}

		case MP_SHOWED2KLINK:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CSimpleArray<CAbstractFile*> paFiles;
				paFiles.Add(BLCItem->m_knownFile);
				// NEO: MLD - [ModelesDialogs]
				CFileDetailDialog* dialog = new CFileDetailDialog(&paFiles,IDD_ED2KLINK); 
				dialog->OpenDialog();
				// NEO: MLD END
				//CFileDetailDialog dialog(&paFiles, IDD_ED2KLINK);
				//dialog.DoModal();
			}
			break;

		case MP_GETED2KLINK:
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file) {
				CString clpbrd = CreateED2kLink(BLCItem->m_file);
				theApp.CopyTextToClipboard(clpbrd);
			}
			else if (BLCItem->m_eItemType == BLCT_LOCALFILE && BLCItem->m_knownFile) {
				CString clpbrd = CreateED2kLink(BLCItem->m_knownFile);
				theApp.CopyTextToClipboard(clpbrd);
			}
			break;
		case MP_GETHTMLED2KLINK:
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file) {
				CString clpbrd = CreateHTMLED2kLink(BLCItem->m_file);
				theApp.CopyTextToClipboard(clpbrd);
			}
			break;
		case MP_PREVIEW:
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file && client) {
				CSearchFile *file = BLCItem->m_file;
				CSearchFile *clientFile = NULL;
				for (POSITION pos = client->GetListFiles()->GetHeadPosition(); pos != NULL; ) {
					CSearchFile *item = client->GetListFiles()->GetNext(pos);
					if (!md4cmp(file->GetFileHash(), item->GetFileHash())) {
						clientFile = item;
						break;
					}
				}
				if (clientFile) {
					if (clientFile->GetPreviews().GetSize() > 0){
						// already have previews
						(new PreviewDlg())->SetFile(clientFile);
					}
					else {
						client->SendPreviewRequest(clientFile);
						AddLogLine(true, GetResString(IDS_X_PREVIEWREQUESTED));
					}
				}
				else
					ASSERT(false); // there should be a corresponding cached CSearchFile* in CUpDownClient*
			}
			break;
		case MP_SEARCHRELATED:
			// just a shortcut for the user typing into the searchfield "related::[filehash]"
			if (BLCItem->m_eItemType == BLCT_FILE && BLCItem->m_file) {
				SSearchParams* pParams = new SSearchParams;
				pParams->strExpression = _T("related::") + md4str(BLCItem->m_file->GetFileHash());
				pParams->strSpecialTitle = GetResString(IDS_RELATED) + _T(": ") + BLCItem->m_file->GetFileName();
				if (pParams->strSpecialTitle.GetLength() > 50){
					pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
				}
				theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
			}
			return TRUE;
		/*case MP_CMT:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CSimpleArray<CAbstractFile*> paFiles;
				paFiles.Add(BLCItem->m_knownFile);
				CFileDetailDialog dialog(&paFiles, IDD_COMMENT);
				dialog.DoModal();
			}
			break;*/
		// NEO: XC - [ExtendedComments]
		case MP_VIEWFILECOMMENTS:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CSimpleArray<CAbstractFile*> paFiles;
				paFiles.Add(BLCItem->m_knownFile);
				// NEO: MLD - [ModelesDialogs]
				CFileDetailDialog* dialog = new CFileDetailDialog(&paFiles,IDD_COMMENTLST); 
				dialog->OpenDialog();
				// NEO: MLD END
				//CFileDetailDialog dialog(&paFiles, IDD_COMMENTLST);
				//dialog.DoModal();
			}
			break;
		// NEO: XC END

		case MP_PRIOVERYLOW:
		case MP_PRIOLOW:
		case MP_PRIONORMAL:
		case MP_PRIOHIGH:
		case MP_PRIOVERYHIGH:
		case MP_PRIOAUTO:
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				uint8 uPriority = PR_NORMAL;
				switch (wParam) {
					case MP_PRIOVERYLOW:	uPriority = PR_VERYLOW;		break;
					case MP_PRIOLOW:		uPriority = PR_LOW;			break;
					case MP_PRIONORMAL:		uPriority = PR_NORMAL;		break;
					case MP_PRIOHIGH:		uPriority = PR_HIGH;		break;
					case MP_PRIOVERYHIGH:	uPriority = PR_VERYHIGH;	break;
					case MP_PRIOAUTO:		uPriority = PR_AUTO;		break;
				}

				if(uPriority == PR_AUTO){
					BLCItem->m_knownFile->SetAutoUpPriority(true);
					BLCItem->m_knownFile->UpdateAutoUpPriority();
				}else{
					BLCItem->m_knownFile->SetAutoUpPriority(false);
					BLCItem->m_knownFile->SetUpPriority(uPriority);
				}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				if(BLCItem->m_knownFile->KnownPrefs->IsEnableVoodoo())
					theApp.voodoo->ManifestShareInstruction(BLCItem->m_knownFile,INST_UL_PRIO,uPriority);
#endif // VOODOO // NEO: VOODOO END
			}
			break;
		// NEO: RT - [ReleaseTweaks]
		case MP_PRIORELEASE:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetReleasePriority(!BLCItem->m_knownFile->IsReleasePriority());
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
					if(BLCItem->m_knownFile->KnownPrefs->IsEnableVoodoo())
						theApp.voodoo->ManifestShareInstruction(BLCItem->m_knownFile,INST_UL_PRIO,10,BLCItem->m_knownFile->IsReleasePriority());
#endif // VOODOO // NEO: VOODOO END
				}
				break;
			}
		// NEO: RT END
		// NEO: SSP - [ShowSharePermissions]
		case MP_PERMDEFAULT:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_DEFAULT);
				}
				break;
			}
		case MP_PERMNONE:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_NONE);
				}
				break;
			}
		case MP_PERMFRIENDS:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_FRIENDS); 
				}
				break;
			}
		case MP_PERMALL:
			{
				if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
					BLCItem->m_knownFile->SetPermissions(PERM_ALL);
				}
				break;
			}
		// NEO: SSP END
		// NEO: VSF - [VirtualSharedFiles]
		case MP_IOM_VIRTFILE:
		case MP_IOM_VIRTDIR: 
		case MP_IOM_VIRTSUBDIR: {
			InputBox input;
			CString title;
			CString path, virtpath;
			CString fileID;
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CKnownFile *file = BLCItem->m_knownFile;
				path = file->GetPath();
				virtpath = file->GetPath(true);
				fileID.Format(_T("%I64u:%s"), (uint64)file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
				if (wParam == MP_IOM_VIRTFILE) {
					title.Format(GetResString(IDS_X_VDS_CHANGEMAP), file->GetFileName());
					input.SetLabels(title,GetResString(IDS_X_VDS_VIRTUALFILE),file->GetPath(true));
				}
			}
			else if (BLCItem->m_eItemType == BLCT_DIR && m_bLocalFiles && !BLCItem->m_origPath.IsEmpty()) {
				if (wParam == MP_IOM_VIRTFILE)
					break;
				path = BLCItem->m_origPath;
				virtpath = BLCItem->m_fullPath;
			}
			else
				break;
			switch (wParam) {
				case MP_IOM_VIRTDIR:
					title.Format(GetResString(IDS_X_VDS_CHANGEMAP), path);
					input.SetLabels(title,GetResString(IDS_X_VDS_VIRTUALDIR),virtpath);
					break;
				case MP_IOM_VIRTSUBDIR:
					title.Format(GetResString(IDS_X_VDS_CHANGEMAP), path);
					input.SetLabels(title,GetResString(IDS_X_VDS_VIRTUALSUBDIR),virtpath);
					break;
			}
			input.DoModal();
			CString output = input.GetInput();
			if (!input.WasCancelled() && output.GetLength()>0) {
				output.MakeLower();
				output.TrimRight(_T('\\'));
				path.MakeLower();
				path.TrimRight(_T('\\'));
				if (wParam == MP_IOM_VIRTFILE)
					thePrefs.m_fileToVDir_map.SetAt(fileID, output);
				else if (wParam == MP_IOM_VIRTDIR)
					thePrefs.m_dirToVDir_map.SetAt(path, output);
				else if (wParam == MP_IOM_VIRTSUBDIR)
					thePrefs.m_dirToVDirWithSD_map.SetAt(path, output);
			}
			UpdateTree(_T(""));
			break;
		}
		case MP_IOM_VIRTREMOVE: {
			CString path, virt;
			if (BLCItem->m_eItemType == BLCT_LOCALFILE) {
				CKnownFile *file = BLCItem->m_knownFile;
				CString fileID;
				CString path = file->GetPath();
				fileID.Format(_T("%I64u:%s"), (uint64)file->GetFileSize(), EncodeBase16(file->GetFileHash(),16));
				if (thePrefs.m_fileToVDir_map.Lookup(fileID, virt))
					thePrefs.m_fileToVDir_map.RemoveKey(fileID);
			}
			else if (BLCItem->m_eItemType == BLCT_DIR && m_bLocalFiles && !BLCItem->m_origPath.IsEmpty())
				path = BLCItem->m_origPath;
			else
				break;
			path.MakeLower();
			path.TrimRight(_T('\\'));
			if (thePrefs.m_dirToVDir_map.Lookup(path, virt))
				thePrefs.m_dirToVDir_map.RemoveKey(path);
			if (thePrefs.m_dirToVDirWithSD_map.Lookup(path, virt))
				thePrefs.m_dirToVDirWithSD_map.RemoveKey(path);
			UpdateTree(_T(""));
			break;
		}
		//case MP_IOM_VIRTPREFS:
		//	theApp.emuledlg->ShowPreferences(IDD_PPG_VIRTUAL);
		//	break;
		// NEO: VSF END

		default:
			if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256) {
				CAbstractFile* file = NULL;
				if (BLCItem->m_eItemType == BLCT_FILE)
					file = BLCItem->m_file;
				else if (BLCItem->m_eItemType == BLCT_LOCALFILE)
					file = BLCItem->m_knownFile;
				if(file)
					theWebServices.RunURL(file, wParam);
			}
			break;
	}
	return true;
}

BOOL CSharedFilesPage::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	LPNMHDR pNmhdr = (LPNMHDR)lParam;

	switch (pNmhdr->code)
	{
		case NM_CUSTOMDRAW:
		{
			LPNMTVCUSTOMDRAW pCustomDraw = (LPNMTVCUSTOMDRAW)lParam;
			switch (pCustomDraw->nmcd.dwDrawStage)
			{
				case CDDS_PREPAINT:
					// Need to process this case and set pResult to CDRF_NOTIFYITEMDRAW, 
					// otherwise parent will never receive CDDS_ITEMPREPAINT notification.
					*pResult = CDRF_NOTIFYITEMDRAW;
					return true;

				case CDDS_ITEMPREPAINT:
					HTREEITEM hItem = (HTREEITEM) pCustomDraw->nmcd.dwItemSpec;
					if (!m_ctrlTree.GetItemData(hItem)) return true;
					BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hItem);
					if (BLCItem) {
						switch (BLCItem->m_eItemType) {
						case BLCT_FILE:
							{
							CKnownFile *file = theApp.downloadqueue->GetFileByID(BLCItem->m_file->GetFileHash());
							if (!file)
								file = theApp.sharedfiles->GetFileByID(BLCItem->m_file->GetFileHash());
							if (pCustomDraw->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED))
								pCustomDraw->clrTextBk = MLC_RGBBLEND(::GetSysColor(COLOR_HIGHLIGHT), ::GetSysColor(COLOR_WINDOW), 4);
							if (!file)
								pCustomDraw->clrText = ::GetSysColor(COLOR_WINDOWTEXT);
							else if (file->IsPartFile())
								pCustomDraw->clrText = RGB(224, 0, 0);
							else
								pCustomDraw->clrText = RGB(0, 128, 0);
							break;
							}
						case BLCT_LOCALFILE:
							if (pCustomDraw->nmcd.uItemState == (CDIS_FOCUS | CDIS_SELECTED))
								pCustomDraw->clrTextBk = MLC_RGBBLEND(::GetSysColor(COLOR_HIGHLIGHT), ::GetSysColor(COLOR_WINDOW), 4);
							if (BLCItem->m_knownFile->GetPermissions() == PERM_NONE)
								pCustomDraw->clrText = RGB(240, 0, 0);
							else if (BLCItem->m_knownFile->GetPermissions() == PERM_FRIENDS)
								pCustomDraw->clrText = RGB(208, 128, 0);
							else if (BLCItem->m_knownFile->IsPartFile())
								pCustomDraw->clrText = RGB(0, 0, 192);
							break;
						}
					}
					*pResult = CDRF_SKIPDEFAULT;
					return false;
			}
		}
		break;
	}
	return CResizablePage::OnNotify(wParam, lParam, pResult);
}

BOOL CSharedFilesPage::OnToolTipNotify(UINT /*id*/, NMHDR *pNMH, LRESULT* /*pResult*/)
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	NM_PPTOOLTIP_DISPLAY * pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMH;
	int control_id = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();
#else
	TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
	int control_id = ::GetDlgCtrlID((HWND)pNMH->idFrom);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	if (!control_id)
		return FALSE;

	CString info;

	if (control_id == IDC_EXT_OPTS)
	{
		HTREEITEM hItem = GetItemUnderMouse();
		if (hItem != NULL) {
			CTreeOptionsItemData* pItemData = (CTreeOptionsItemData*) m_ctrlTree.GetItemData(hItem);
			if (pItemData) {
				BLCItem_struct *BLCItem = (BLCItem_struct *) pItemData->m_dwItemData;
				if (BLCItem)
					if (BLCItem->m_eItemType == BLCT_FILE) {
						CPartFile *file = theApp.downloadqueue->GetFileByID(BLCItem->m_file->GetFileHash());
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
						if (file && file->IsPartFile())
							file->GetTooltipFileInfo(info);
						else
							BLCItem->m_file->GetTooltipFileInfo(info);

						pNotify->ti->sTooltip = info;
						SetFocus();
						return TRUE;
#else
						if (file && file->IsPartFile())
							info = file->GetInfoSummary(/*file*/);
						else
							info.Format(
								GetResString(IDS_DL_FILENAME) + _T(": %s\n") +
								GetResString(IDS_FD_HASH) + _T(" %s\n") +
								GetResString(IDS_FD_SIZE) + _T(" %s"), 
								BLCItem->m_file->GetFileName(),
								EncodeBase16(BLCItem->m_file->GetFileHash(),16), 
								CastItoXBytes(BLCItem->m_file->GetFileSize()));
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
					}
			}
		}
	}
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	else		
		if(pNotify->ti->hIcon)
			pNotify->ti->hIcon = DuplicateIcon(AfxGetInstanceHandle(), pNotify->ti->hIcon);
#else
	m_strToolTip.ReleaseBuffer();
	m_strToolTip = info;
	pText->lpszText = m_strToolTip.GetBuffer(1);
	pText->hinst = NULL; // we are not using a resource
	PostMessage(WM_ACTIVATE);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	return TRUE;
}

BOOL CSharedFilesPage::PreTranslateMessage(MSG* pMsg)
{
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	m_ttip.RelayEvent(pMsg);
	//m_othertips.RelayEvent(pMsg);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

   	if ( pMsg->message == 260 && pMsg->wParam == 13 && GetAsyncKeyState(VK_MENU)<0 ) {
		PostMessage(WM_COMMAND, MPG_ALTENTER, 0);
		return TRUE;
	}
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	if (pMsg->message== WM_LBUTTONDOWN || pMsg->message== WM_LBUTTONUP || pMsg->message== WM_MOUSEMOVE) {
		m_toolTip->RelayEvent(pMsg);
	}
	if (pMsg->message == WM_MOUSEMOVE) {
		HTREEITEM hItem = GetItemUnderMouse();
		if (hItem != NULL)
		{
			if (hItem != m_htiOldToolTipItemDown)
			{
				if (m_toolTip->IsWindowVisible())
					m_toolTip->Update();
				m_htiOldToolTipItemDown = hItem;
			}
		}
	}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	return CResizablePage::PreTranslateMessage(pMsg);
}

static int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CTreeOptionsItemData* pItemData1 = (CTreeOptionsItemData*) lParam1;
	BLCItem_struct *Item1 = (BLCItem_struct*)pItemData1->m_dwItemData;
	CTreeOptionsItemData* pItemData2 = (CTreeOptionsItemData*) lParam2;
	BLCItem_struct *Item2 = (BLCItem_struct*)pItemData2->m_dwItemData;

	if (Item1->m_eItemType == Item2->m_eItemType)
		return Item1->m_name.CompareNoCase(Item2->m_name);
	if (Item1->m_eItemType == BLCT_ROOT)
		return -1;
	if (Item2->m_eItemType == BLCT_ROOT)
		return 1;
	if (Item1->m_eItemType == BLCT_DIR)
		return -1;
	if (Item2->m_eItemType == BLCT_DIR)
		return 1;
	ASSERT(false);
	return 0;
}

void CSharedFilesPage::SortDirs(HTREEITEM hParent) {
	HTREEITEM hChild = m_ctrlTree.GetChildItem(hParent);
	while (hChild != NULL) {
		SortDirs(hChild);
		hChild = m_ctrlTree.GetNextSiblingItem(hChild);
	}

	TVSORTCB tvs;

	tvs.hParent = hParent;
	tvs.lpfnCompare = CompareProc;

	m_ctrlTree.SortChildrenCB(&tvs);
}

void CSharedFilesPage::FillTree()
{
	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	CRBMap <CString, bool> *listDirs = NULL;
	CList <CSearchFile *> *listFiles = NULL;
	if (!m_bLocalFiles) {
		if (!client)
			return;	// no client
		listDirs = client->GetListDirs();
		listFiles = client->GetListFiles();
	}
	else {
		GetDirs();
		listDirs = &m_dirsList;
	}

	BLCItem_struct *BLCItem;

	if (!m_htiRoot) {
		CString buffer;
		BLCItem = new BLCItem_struct;
		BLCItem->m_eItemType = BLCT_ROOT;
		m_BLCItem_list.AddTail(BLCItem);
		if (!m_bLocalFiles)
			buffer.Format(GetResString(IDS_X_USERSSHAREDFILES), client->GetUserName());
		else
			buffer.Format(GetResString(IDS_X_USERSSHAREDFILES), thePrefs.GetUserNick());
		m_htiRoot = m_ctrlTree.InsertGroup(buffer, m_iImgRoot, TVI_ROOT, TVI_LAST, (DWORD)BLCItem);
		m_HTIs_map.SetAt(_T("\\"), m_htiRoot);
		TVITEM it;
		it.hItem = m_htiRoot;
		it.mask = TVIF_CHILDREN;
		it.cChildren = 1;
		m_ctrlTree.SetItem(&it);
	}

	for (POSITION pos = listDirs->GetHeadPosition(); pos != NULL; listDirs->GetNext(pos)) {
		int curPos = 0;
		HTREEITEM dirHti;
		HTREEITEM lastNode = m_htiRoot;
		CString newDir = listDirs->GetKeyAt(pos);

		CString resToken = newDir.Tokenize(_T("\\"), curPos);
		while (resToken != _T(""))
		{
			CString fullPath = newDir.Left(curPos);
			fullPath.TrimRight(_T('\\'));
			if (!m_HTIs_map.Lookup(CString(fullPath).MakeLower(), dirHti)) {
				BLCItem = new BLCItem_struct;
				BLCItem->m_eItemType = BLCT_DIR;
				BLCItem->m_fullPath = fullPath;
				BLCItem->m_name = resToken;
				m_BLCItem_list.AddTail(BLCItem);
				dirHti = m_ctrlTree.InsertGroup(resToken, m_iImgDir, lastNode, TVI_LAST, (DWORD)BLCItem);
				m_HTIs_map.SetAt(CString(fullPath).MakeLower(), dirHti);
				TVITEM it;
				it.hItem = dirHti;
				it.mask = TVIF_CHILDREN;
				it.cChildren = 1;
				m_ctrlTree.SetItem(&it);
			}
			lastNode = dirHti;
			resToken = newDir.Tokenize(_T("\\"), curPos);
		};
		if (m_HTIs_map.Lookup(CString(newDir).TrimRight(_T('\\')).MakeLower(), dirHti)) {
			m_ctrlTree.SetItemImage(dirHti, m_iImgShDir, m_iImgShDir);
			BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(dirHti);
			ASSERT(BLCItem->m_eItemType == BLCT_DIR);
			BLCItem->m_fullPath = newDir;	// Must use unaltered string when requesting
		}
		else
			ASSERT(false);
	}

	if (m_bLocalFiles) {
		for (uint16 i = 0; i<theApp.sharedfiles->GetCount(); i++) {
			CKnownFile* cur_file = theApp.sharedfiles->GetFileByIndex(i);
			// NEO: PP - [PasswordProtection]
			if (cur_file->IsPWProtHidden())
				continue;
			// NEO: PP END
			CString path = cur_file->GetPath(true);	// NEO: VSF - [VirtualSharedFiles]
			path.TrimRight(_T('\\'));
			path.MakeLower();
			HTREEITEM dirHti;
			if (m_HTIs_map.Lookup(path, dirHti)) {
				BLCItem = new BLCItem_struct;
				BLCItem->m_eItemType = BLCT_LOCALFILE;
				BLCItem->m_knownFile = cur_file;
				BLCItem->m_name = cur_file->GetFileName();
				m_BLCItem_list.AddTail(BLCItem);
				/*HTREEITEM newHti =*/ m_ctrlTree.InsertGroup(BLCItem->m_name, GetFileIcon(BLCItem->m_name), dirHti, TVI_LAST, (DWORD)BLCItem);
				// TODO: Read virtual dirs data and set, instead of using the data from the files
				BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(dirHti);
				BLCItem->m_origPath = cur_file->GetPath();
			}
			else
				ASSERT (false);
		}
	}
	else {
		for (POSITION pos = listFiles->GetHeadPosition(); pos != NULL; listFiles->GetNext(pos)) {
			HTREEITEM dirHti;
			if (m_HTIs_map.Lookup(CString(listFiles->GetAt(pos)->GetDirectory()).TrimRight(_T('\\')).MakeLower(), dirHti)) {
				BLCItem = new BLCItem_struct;
				BLCItem->m_eItemType = BLCT_FILE;
				BLCItem->m_file = new CSearchFile(listFiles->GetAt(pos));
				BLCItem->m_name = BLCItem->m_file->GetFileName();
				m_BLCItem_list.AddTail(BLCItem);
				/*HTREEITEM newHti =*/ m_ctrlTree.InsertGroup(BLCItem->m_name, GetFileIcon(BLCItem->m_name), dirHti, TVI_LAST, (DWORD)BLCItem);
			}
			else
				ASSERT (false);
		}
	}
	SortDirs(m_htiRoot);
}

bool CSharedFilesPage::FilterDirs(CRBMap<CString, bool> *listDirs, HTREEITEM hParent) {
	BLCItem_struct *BLCItem = (BLCItem_struct *) m_ctrlTree.GetUserItemData(hParent);
	if (BLCItem->m_eItemType != BLCT_DIR) {
		POSITION pos = m_BLCItem_list.Find(BLCItem);
		ASSERT(pos != NULL);
		m_BLCItem_list.RemoveAt(pos);
		m_ctrlTree.DeleteItem(hParent);
		delete BLCItem;
		return false;
	}

	bool requested;
	bool hasChildren = false;
	HTREEITEM hChild = m_ctrlTree.GetChildItem(hParent);
	while (hChild != NULL) {
		HTREEITEM hNext = m_ctrlTree.GetNextSiblingItem(hChild);
		hasChildren |= FilterDirs(listDirs, hChild);
		hChild = hNext;
	}
	if (listDirs->Lookup(BLCItem->m_fullPath, requested))
		return true;
	if (hasChildren) {
		m_ctrlTree.SetItemImage(hParent, m_iImgDir, m_iImgDir);	// Not shared, but has shared subdir
		return true;
	}
	POSITION pos = m_BLCItem_list.Find(BLCItem);
	ASSERT(pos != NULL);
	m_BLCItem_list.RemoveAt(pos);
	m_ctrlTree.DeleteItem(hParent);
	m_HTIs_map.RemoveKey(CString(BLCItem->m_fullPath).MakeLower());
	delete BLCItem;
	return false;
}

void CSharedFilesPage::UpdateTree(CString newDir)
{
	if (!m_htiRoot) // X!
		return;

	CUpDownClient* client = NULL;
	if (m_paClients && m_paClients->GetSize() > 0) {
		if (theApp.clientlist->IsValidClient((CUpDownClient*)(*m_paClients)[0]))
			client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
	}

	CRBMap <CString, bool> *listDirs = NULL;
	if (!m_bLocalFiles) {
		if (!client)
			return;	// no client
		listDirs = client->GetListDirs();
	}
	else {
		GetDirs();
		listDirs = &m_dirsList;
	}

	SetRedraw(false);
	HTREEITEM hChild = m_ctrlTree.GetChildItem(m_htiRoot);
	while (hChild != NULL) {
		HTREEITEM hNext = m_ctrlTree.GetNextSiblingItem(hChild);
		FilterDirs(listDirs, hChild);
		hChild = hNext;
	}
	if (!newDir.IsEmpty()) {
		HTREEITEM dirHti;
		if (m_HTIs_map.Lookup(CString(newDir).TrimRight(_T('\\')).MakeLower(), dirHti)) {
			BLCItem_struct *BLCItem = (BLCItem_struct*)m_ctrlTree.GetUserItemData(dirHti);
			m_ctrlTree.SetItemImage(dirHti, m_iImgShDir, m_iImgShDir);
			m_ctrlTree.SetItemText(dirHti, BLCItem->m_name);
			m_ctrlTree.SetItemState(dirHti, TVIS_EXPANDED, TVIS_EXPANDED);
		}
		else
			ASSERT(false);
	}
	else
		m_ctrlTree.SetItemState(m_htiRoot, TVIS_EXPANDED, TVIS_EXPANDED);
	Localize();
	FillTree();
	SetRedraw(true);
	Invalidate();
	UpdateWindow();
}

void CSharedFilesPage::GetDirs()
{
	ASSERT(m_bLocalFiles);
	CKnownFile* cur_file;
	m_dirsList.RemoveAll();
	for (uint16 i = 0; i<theApp.sharedfiles->GetCount(); i++) {
		cur_file = theApp.sharedfiles->GetFileByIndex(i);
		// NEO: PP - [PasswordProtection]
		if (cur_file->IsPWProtHidden())
			continue;
		// NEO: PP END
		CString path = cur_file->GetPath(true);	// NEO: VSF - [VirtualSharedFiles]
		path.TrimRight(_T('\\'));
		m_dirsList.SetAt(path, true);
	}
}

HTREEITEM CSharedFilesPage::GetItemUnderMouse()
{
	CPoint pt;
	::GetCursorPos(&pt);
	m_ctrlTree.ScreenToClient(&pt);
	TVHITTESTINFO hit;
	hit.pt = pt;
	HTREEITEM hItem = m_ctrlTree.HitTest(&hit);
	return hItem;
}

int CSharedFilesPage::GetFileIcon(CString filename)
{
	int srcicon = theApp.GetFileTypeSystemImageIdx(filename.MakeLower());
	int dsticon;

	if (m_iconMap.Lookup(srcicon, dsticon))
		return dsticon;

	CImageList* imageList = m_ctrlTree.GetImageList(TVSIL_NORMAL);
	dsticon = imageList->Add(CImageList::FromHandle(theApp.GetSystemImageList())->ExtractIcon(srcicon));
	m_iconMap.SetAt(srcicon, dsticon);
	return dsticon;
}

// NEO: XSF END <-- Xanatos --

// NEO: NMX - [NeoMenuXP] -- Xanatos -->
void CSharedFilesPage::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	if(CMenu *pMenu = CMenu::FromHandle(hMenu))
		pMenu->MeasureItem(lpMeasureItemStruct);
	
	CResizablePage::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// NEO: NMX END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CSharedFilesPage::SetTTDelay()
{
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 20000);
	m_ttip.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
	//m_othertips.SetDelayTime(TTDT_AUTOPOP, 20000);
	//m_othertips.SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay() * 500);
}		
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
