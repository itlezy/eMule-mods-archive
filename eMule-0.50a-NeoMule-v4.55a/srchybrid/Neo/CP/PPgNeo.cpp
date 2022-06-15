//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#include "PPgNeo.h"
#include "Neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "Neo/Functions.h"
#include "Neo/GUI/CP/TreeFunctions.h"
#include "Neo/Defaults.h"
#include "KnownFileList.h"
#include "Opcodes.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CPPgNeo dialog

IMPLEMENT_DYNAMIC(CPPgNeo, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgNeo, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgNeo::CPPgNeo()
	: CPropertyPage(CPPgNeo::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgNeo::~CPPgNeo()
{
}

void CPPgNeo::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
	m_htiAppPriority = NULL; // NEO: MOD - [AppPriority]
	m_htiPauseOnFileComplete = NULL; // NEO: POFC - [PauseOnFileComplete]

	m_htiProt = NULL;
		m_htiIncompletePartStatus = NULL;	// NEO: ICS - [InteligentChunkSelection] 
		m_htiSubChunkTransfer = NULL; // NEO: SCT - [SubChunkTransfer]
		m_htiPartStatusHistory = NULL; // NEO: PSH - [PartStatusHistory]
		m_htiLowID2HighIDAutoCallback = NULL; // NEO: L2HAC - [LowID2HighIDAutoCallback]
		m_htiSaveComments = NULL; // NEO: XCs - [SaveComments]
		m_htiKnownComments = NULL; // NEO: XCk - [KnownComments]	
	
	m_htiPreferShareAll = NULL; // NEO: PSA - [PreferShareAll]
	m_htiLugdunumCredits = NULL; // NEO: KLC - [KhaosLugdunumCredits]
	m_htiDontRemoveStaticServers = NULL; // NEO: MOD - [DontRemoveStaticServers]

	m_htiOnlyCompleteFiles = NULL; // NEO: OCF - [OnlyCompleetFiles]

	m_htiRefreshShared = NULL; // NEO: MOD - [RefreshShared]
}

void CPPgNeo::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgProt = 8;
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgProt = piml->Add(CTempIconLoader(_T("MAINTWEAKS")));
		}
		
		SetTreePriority(m_ctrlTreeOptions,m_htiAppPriority,GetResString(IDS_X_APP_PRIORITY), TVI_ROOT,GetResString(IDS_X_APP_PRIORITY_INFO)); // NEO: MOD - [AppPriority]
		SetTreeCheck(m_ctrlTreeOptions,m_htiPauseOnFileComplete,GetResString(IDS_X_PAUSE_ON_COMPLETE),TVI_ROOT,GetResString(IDS_X_PAUSE_ON_COMPLETE_INFO),FALSE,m_bPauseOnFileComplete); // NEO: POFC - [PauseOnFileComplete]

		SetTreeGroup(m_ctrlTreeOptions,m_htiProt,GetResString(IDS_X_PROTOCOL_TWEAKS),iImgProt, TVI_ROOT, GetResString(IDS_X_PROTOCOL_TWEAKS_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiIncompletePartStatus,GetResString(IDS_X_INTELIGENT_CHUNK_SELECTION),m_htiProt,GetResString(IDS_X_INTELIGENT_CHUNK_SELECTION_INFO),TRUE,m_uIncompletePartStatus); // NEO: ICS - [InteligentChunkSelection] 
			SetTreeCheck(m_ctrlTreeOptions,m_htiSubChunkTransfer,GetResString(IDS_X_SUB_CHUNK_TRANSFER),m_htiProt,GetResString(IDS_X_SUB_CHUNK_TRANSFER_INFO),TRUE,m_uSubChunkTransfer); // NEO: SCT - [SubChunkTransfer]
			SetTreeCheck(m_ctrlTreeOptions,m_htiLowID2HighIDAutoCallback,GetResString(IDS_X_L2HAC),m_htiProt,GetResString(IDS_X_L2HAC_INFO),FALSE,m_bLowID2HighIDAutoCallback); // NEO: L2HAC - [LowID2HighIDAutoCallback]
			SetTreeCheck(m_ctrlTreeOptions,m_htiKnownComments,GetResString(IDS_X_KNOWN_COMMENTS),m_htiProt,GetResString(IDS_X_KNOWN_COMMENTS_INFO),FALSE,m_bKnownComments); // NEO: XCk - [KnownComments
		SetTreeCheck(m_ctrlTreeOptions,m_htiSaveComments,GetResString(IDS_X_SAVE_COMMENTS),TVI_ROOT,GetResString(IDS_X_SAVE_COMMENTS_INFO),FALSE,m_bSaveComments); // NEO: XCs - [SaveComments]
		SetTreeCheck(m_ctrlTreeOptions,m_htiPartStatusHistory,GetResString(IDS_X_PART_STATUS_HISTORY),TVI_ROOT,GetResString(IDS_X_PART_STATUS_HISTORY_INFO),FALSE,m_bPartStatusHistory); // NEO: PSH - [PartStatusHistory]

		SetTreeCheck(m_ctrlTreeOptions,m_htiPreferShareAll,GetResString(IDS_X_PREFER_SHARE_ALL),TVI_ROOT,GetResString(IDS_X_PREFER_SHARE_ALL_INFO),FALSE,m_bPreferShareAll); // NEO: PSA - [PreferShareAll]
		SetTreeCheck(m_ctrlTreeOptions,m_htiLugdunumCredits,GetResString(IDS_X_LUGDUNUM_CREDITS),TVI_ROOT,GetResString(IDS_X_LUGDUNUM_CREDITS_INFO),TRUE,m_uLugdunumCredits);  // NEO: KLC - [KhaosLugdunumCredits]
		SetTreeCheck(m_ctrlTreeOptions,m_htiDontRemoveStaticServers,GetResString(IDS_X_DONT_REMOVE_STATIC),TVI_ROOT,GetResString(IDS_X_DONT_REMOVE_STATIC_INFO),FALSE,m_bDontRemoveStaticServers); // NEO: MOD - [DontRemoveStaticServers]

		SetTreeCheck(m_ctrlTreeOptions,m_htiOnlyCompleteFiles,GetResString(IDS_X_DOWNLOAD_ONLY_COMPLETE_FILES),TVI_ROOT,GetResString(IDS_X_DOWNLOAD_ONLY_COMPLETE_FILES_INFO),FALSE,m_bOnlyCompleteFiles); // NEO: OCF - [OnlyCompleetFiles]

		SetTreeCheck(m_ctrlTreeOptions,m_htiRefreshShared,GetResString(IDS_X_REFRESH_SHARE),TVI_ROOT,GetResString(IDS_X_REFRESH_SHARE_INFO),FALSE,m_bRefreshShared); // NEO: MOD - [RefreshShared]

		m_ctrlTreeOptions.Expand(m_htiProt, TVE_EXPAND);

		m_bInitializedTreeOpts = true;
	}

	DDX_PriorityC(&m_ctrlTreeOptions, pDX, IDC_MOD_OPTS, m_htiAppPriority, m_dAppPriority); // NEO: MOD - [AppPriority]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPauseOnFileComplete, m_bPauseOnFileComplete); // NEO: POFC - [PauseOnFileComplete]

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIncompletePartStatus, m_uIncompletePartStatus); // NEO: ICS - [InteligentChunkSelection] 
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSubChunkTransfer, m_uSubChunkTransfer); // NEO: SCT - [SubChunkTransfer]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPartStatusHistory, m_bPartStatusHistory);	 // NEO: PSH - [PartStatusHistory]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLowID2HighIDAutoCallback, m_bLowID2HighIDAutoCallback); // NEO: L2HAC - [LowID2HighIDAutoCallback]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSaveComments, m_bSaveComments); // NEO: XCs - [SaveComments]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiKnownComments, m_bKnownComments); // NEO: XCk - [KnownComments

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPreferShareAll, m_bPreferShareAll); // NEO: PSA - [PreferShareAll]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLugdunumCredits, m_uLugdunumCredits); // NEO: KLC - [KhaosLugdunumCredits]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDontRemoveStaticServers, m_bDontRemoveStaticServers); // NEO: MOD - [DontRemoveStaticServers]

	// NEO: OCF - [OnlyCompleetFiles]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiOnlyCompleteFiles, m_bOnlyCompleteFiles);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiOnlyCompleteFiles, m_iToOldComplete);
	// NEO: OCF END

	// NEO: MOD - [RefreshShared]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRefreshShared, m_bRefreshShared);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiRefreshShared, m_iRefreshSharedIntervals);
	// NEO: MOD END
}

BOOL CPPgNeo::OnInitDialog()
{
	LoadSettings();

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgNeo::LoadSettings()
{
	/*
	* Globale Einstellungen Laden
	*/

	m_dAppPriority = NeoPrefs.m_dAppPriority; // NEO: MOD - [AppPriority]
	m_bPauseOnFileComplete = NeoPrefs.m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]

	m_uIncompletePartStatus = NeoPrefs.m_uIncompletePartStatus; // NEO: ICS - [InteligentChunkSelection] 
	m_uSubChunkTransfer = NeoPrefs.m_uSubChunkTransfer; // NEO: SCT - [SubChunkTransfer]
	m_bPartStatusHistory = NeoPrefs.m_bPartStatusHistory; // NEO: PSH - [PartStatusHistory]
	m_bLowID2HighIDAutoCallback = NeoPrefs.m_bLowID2HighIDAutoCallback; // NEO: L2HAC - [LowID2HighIDAutoCallback]
	m_bSaveComments = NeoPrefs.m_bSaveComments; // NEO: XCs - [SaveComments]
	m_bKnownComments = NeoPrefs.m_bKnownComments; // NEO: XCk - [KnownComments]	
	
	m_bPreferShareAll = NeoPrefs.m_bPreferShareAll; // NEO: PSA - [PreferShareAll]
	m_uLugdunumCredits = NeoPrefs.m_uLugdunumCredits; // NEO: KLC - [KhaosLugdunumCredits]
	m_bDontRemoveStaticServers = NeoPrefs.m_bDontRemoveStaticServers; // NEO: MOD - [DontRemoveStaticServers]

	// NEO: OCF - [OnlyCompleetFiles]
	m_bOnlyCompleteFiles = NeoPrefs.m_bOnlyCompleteFiles;
	m_iToOldComplete = NeoPrefs.m_iToOldComplete;
	// NEO: OCF END

	// NEO: MOD - [RefreshShared]
	m_bRefreshShared = NeoPrefs.m_bRefreshShared;
	m_iRefreshSharedIntervals = NeoPrefs.m_iRefreshSharedIntervals;
	// NEO: MOD END
}

BOOL CPPgNeo::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	/*
	* Globale Einstellungen Speichern
	*/

	// NEO: MOD - [AppPriority]
	if(NeoPrefs.m_dAppPriority != m_dAppPriority)
		SetPriorityClass(GetCurrentProcess(), m_dAppPriority);
	NeoPrefs.m_dAppPriority = m_dAppPriority;
	// NEO: MOD END
	NeoPrefs.m_bPauseOnFileComplete = m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]

	NeoPrefs.m_uIncompletePartStatus = m_uIncompletePartStatus; // NEO: ICS - [InteligentChunkSelection] 
	NeoPrefs.m_uSubChunkTransfer = m_uSubChunkTransfer; // NEO: SCT - [SubChunkTransfer]
	NeoPrefs.m_bPartStatusHistory = m_bPartStatusHistory; // NEO: PSH - [PartStatusHistory]
	NeoPrefs.m_bLowID2HighIDAutoCallback = m_bLowID2HighIDAutoCallback; // NEO: L2HAC - [LowID2HighIDAutoCallback]
	NeoPrefs.m_bSaveComments = m_bSaveComments; // NEO: XCs - [SaveComments]
	NeoPrefs.m_bKnownComments = m_bKnownComments; // NEO: XCk - [KnownComments]	
	
	NeoPrefs.m_bPreferShareAll = m_bPreferShareAll; // NEO: PSA - [PreferShareAll]
	NeoPrefs.m_uLugdunumCredits = m_uLugdunumCredits; // NEO: KLC - [KhaosLugdunumCredits]
	NeoPrefs.m_bDontRemoveStaticServers = m_bDontRemoveStaticServers; // NEO: MOD - [DontRemoveStaticServers]

	// NEO: OCF - [OnlyCompleetFiles]
	NeoPrefs.m_bOnlyCompleteFiles = m_bOnlyCompleteFiles;
	NeoPrefs.m_iToOldComplete = m_iToOldComplete;
	// NEO: OCF END

	// NEO: MOD - [RefreshShared]
	NeoPrefs.m_bRefreshShared = m_bRefreshShared;
	NeoPrefs.m_iRefreshSharedIntervals = m_iRefreshSharedIntervals;
	// NEO: MOD END

	NeoPrefs.CheckNeoPreferences();
	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgNeo::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgNeo::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

LRESULT CPPgNeo::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){
			// NEO: OCF - [OnlyCompleetFiles]
			if (m_htiOnlyCompleteFiles && pton->hItem == m_htiOnlyCompleteFiles){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiOnlyCompleteFiles, 3, 30, 300)) SetModified();
			}
			// NEO: OCF END
			// NEO: MOD - [RefreshShared]
			if (m_htiRefreshShared && pton->hItem == m_htiRefreshShared){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiRefreshShared, 60, 120, 240)) SetModified();
			}
			// NEO: MOD END
		}else{
			SetModified();
		}
	}
	return 0;
}


LRESULT CPPgNeo::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgNeo::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgNeo::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgNeo::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
