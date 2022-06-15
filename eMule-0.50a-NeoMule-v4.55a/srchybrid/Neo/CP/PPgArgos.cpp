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
#include "PPgArgos.h"
#include "neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "Neo/Functions.h"
#include "Neo/GUI/CP/TreeFunctions.h"
#include "Neo/Defaults.h"
#include "KnownFileList.h"
#include "Opcodes.h"
#include "Neo/Argos.h"
#include "emuledlg.h"
#include "transferwnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CPPgArgos dialog

IMPLEMENT_DYNAMIC(CPPgArgos, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgArgos, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgArgos::CPPgArgos()
	: CPropertyPage(CPPgArgos::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgArgos::~CPPgArgos()
{
}

void CPPgArgos::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
	m_htiZeroScoreGPLBreaker = NULL;
	m_htiBanTime = NULL;
	m_htiCloseMaellaBackdoor = NULL;

	// DLP Groupe
	m_htiDetectionLevel = NULL;
		m_htiDetectionLevel0 = NULL;
		m_htiDetectionLevel1 = NULL;
		m_htiDetectionLevel2 = NULL;
		m_htiLeecherModDetection = NULL;
		m_htiLeecherNickDetection = NULL;
		m_htiLeecherHashDetection = NULL;
	

	// Behavioural groupe
	m_htiAgressionDetection = NULL;
	m_htiHashChangeDetection = NULL;

	m_htiUploadFakerDetection = NULL;
	m_htiFileFakerDetection = NULL;
	m_htiRankFloodDetection = NULL;
	m_htiXsExploitDetection = NULL;
	m_htiFileScannerDetection = NULL;
	m_htiSpamerDetection = NULL;

	m_htiHashThiefDetection = NULL;
	m_htiNickThiefDetection = NULL;
	m_htiModThiefDetection = NULL;
}

void ReloadDLP(){
	theApp.argos->LoadDLPlibrary();
}

void CPPgArgos::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		int iImgEngine = 8;
		if (piml){
			iImgEngine = piml->Add(CTempIconLoader(_T("ARGOSENGINE")));
		}

		SetTreeCheck(m_ctrlTreeOptions,m_htiZeroScoreGPLBreaker,GetResString(IDS_X_ZERO_SCORE_NON_GPL),TVI_ROOT,GetResString(IDS_X_ZERO_SCORE_NON_GPL_INFO),FALSE,m_bZeroScoreGPLBreaker);
		SetTreeNumEdit(m_ctrlTreeOptions,m_htiBanTime,GetResString(IDS_X_BAN_TIME), TVI_ROOT,GetResString(IDS_X_BAN_TIME_INFO));
		SetTreeCheck(m_ctrlTreeOptions,m_htiCloseMaellaBackdoor,GetResString(IDS_X_MAELA_BACKDOOR),TVI_ROOT,GetResString(IDS_X_MAELA_BACKDOOR_INFO),FALSE,m_bCloseMaellaBackdoor);

		// DLP Groupe
		m_htiDetectionLevel = m_ctrlTreeOptions.InsertButton(GetResString(IDS_X_DLP), TVI_ROOT, &ReloadDLP);
		m_ctrlTreeOptions.SetItemIcons(m_htiDetectionLevel,iImgEngine,iImgEngine);
		m_ctrlTreeOptions.SetItemInfo(m_htiDetectionLevel,GetResString(IDS_X_DLP_INFO));

			SetTreeRadio(m_ctrlTreeOptions,m_htiDetectionLevel0,GetResString(IDS_X_DLP_HIGH),m_htiDetectionLevel,GetResString(IDS_X_DLP_HIGH_INFO),FALSE,m_iDetectionLevel == 0);
			SetTreeRadio(m_ctrlTreeOptions,m_htiDetectionLevel1,GetResString(IDS_X_DLP_NORMAL),m_htiDetectionLevel,GetResString(IDS_X_DLP_NORMAL_INFO),FALSE,m_iDetectionLevel == 1);
			SetTreeRadio(m_ctrlTreeOptions,m_htiDetectionLevel2,GetResString(IDS_X_DLP_LOW),m_htiDetectionLevel,GetResString(IDS_X_DLP_LOW_INFO),FALSE,m_iDetectionLevel == 2);
	
			SetTreeCheck(m_ctrlTreeOptions,m_htiLeecherModDetection,GetResString(IDS_X_DLP_MOD),m_htiDetectionLevel,GetResString(IDS_X_DLP_MOD_INFO),FALSE,m_bLeecherModDetection);
			SetTreeCheck(m_ctrlTreeOptions,m_htiLeecherNickDetection,GetResString(IDS_X_DLP_NICK),m_htiDetectionLevel,GetResString(IDS_X_DLP_NICK_INFO),FALSE,m_bLeecherNickDetection);
			SetTreeCheck(m_ctrlTreeOptions,m_htiLeecherHashDetection,GetResString(IDS_X_DLP_HASH),m_htiDetectionLevel,GetResString(IDS_X_DLP_HASH_INFO),FALSE,m_bLeecherHashDetection);

		// Behavioural groupe
		SetTreeCheck(m_ctrlTreeOptions,m_htiAgressionDetection,GetResString(IDS_X_ANTI_AGRESSION),TVI_ROOT,GetResString(IDS_X_ANTI_AGRESSION_INFO),TRUE,m_uAgressionDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiHashChangeDetection,GetResString(IDS_X_ANTI_HASH_CHANGE),TVI_ROOT,GetResString(IDS_X_ANTI_HASH_CHANGE_INFO),TRUE,m_uHashChangeDetection);

		SetTreeCheck(m_ctrlTreeOptions,m_htiUploadFakerDetection,GetResString(IDS_X_ANTI_UPLOAD_FAKER),TVI_ROOT,GetResString(IDS_X_ANTI_UPLOAD_FAKER_INFO),FALSE,m_bUploadFakerDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiFileFakerDetection,GetResString(IDS_X_ANTI_FILE_FAKER),TVI_ROOT,GetResString(IDS_X_ANTI_FILE_FAKER_INFO),FALSE,m_bFileFakerDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiRankFloodDetection,GetResString(IDS_X_ANTI_RANK_FLOODER),TVI_ROOT,GetResString(IDS_X_ANTI_RANK_FLOODER_INFO),FALSE,m_bRankFloodDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiXsExploitDetection,GetResString(IDS_X_ANTI_XS_EXPLOIT),TVI_ROOT,GetResString(IDS_X_ANTI_XS_EXPLOIT_INFO),FALSE,m_bXsExploitDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiFileScannerDetection,GetResString(IDS_X_ANTI_FILE_SCANNER),TVI_ROOT,GetResString(IDS_X_ANTI_FILE_SCANNER_INFO),FALSE,m_bFileScannerDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiSpamerDetection,GetResString(IDS_X_ANTI_SPAMER),TVI_ROOT,GetResString(IDS_X_ANTI_SPAMER_INFO),TRUE,m_uSpamerDetection);

		SetTreeCheck(m_ctrlTreeOptions,m_htiHashThiefDetection,GetResString(IDS_X_ANTI_HASH_THIEF),TVI_ROOT,GetResString(IDS_X_ANTI_HASH_THIEF_INFO),FALSE,m_bHashThiefDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiNickThiefDetection,GetResString(IDS_X_ANTI_NICK_THIEF),TVI_ROOT,GetResString(IDS_X_ANTI_NICK_THIEF_INFO),TRUE,m_uNickThiefDetection);
		SetTreeCheck(m_ctrlTreeOptions,m_htiModThiefDetection,GetResString(IDS_X_ANTI_MOD_THIEF),TVI_ROOT,GetResString(IDS_X_ANTI_MOD_THIEF_INFO),FALSE,m_bModThiefDetection);

		//m_ctrlTreeOptions.Expand(m_hti, TVE_EXPAND);
		m_bInitializedTreeOpts = true;
	}

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiZeroScoreGPLBreaker, m_bZeroScoreGPLBreaker);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiBanTime, m_iBanTime);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCloseMaellaBackdoor, m_bCloseMaellaBackdoor);

	// DLP Groupe
		DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiDetectionLevel, m_iDetectionLevel);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLeecherModDetection, m_bLeecherModDetection);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLeecherNickDetection, m_bLeecherNickDetection);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLeecherHashDetection, m_bLeecherHashDetection);

	// Behavioural groupe
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLeecherHashDetection, m_bLeecherHashDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLeecherHashDetection, m_bLeecherHashDetection);

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUploadFakerDetection, m_bUploadFakerDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiFileFakerDetection, m_bFileFakerDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRankFloodDetection, m_bRankFloodDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiXsExploitDetection, m_bXsExploitDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiFileScannerDetection, m_bFileScannerDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSpamerDetection, m_uSpamerDetection);

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiHashThiefDetection, m_bHashThiefDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiNickThiefDetection, m_uNickThiefDetection);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiModThiefDetection, m_bModThiefDetection);
}

BOOL CPPgArgos::OnInitDialog()
{
	LoadSettings();

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgArgos::LoadSettings()
{
	/*
	* Globale Einstellungen Laden
	*/

	m_bZeroScoreGPLBreaker = NeoPrefs.m_bZeroScoreGPLBreaker;
	m_iBanTime = NeoPrefs.m_iBanTime;
	m_bCloseMaellaBackdoor = NeoPrefs.m_bCloseMaellaBackdoor;

	// DLP Groupe
	m_bLeecherModDetection = NeoPrefs.m_bLeecherModDetection;
	m_bLeecherNickDetection = NeoPrefs.m_bLeecherNickDetection;
	m_bLeecherHashDetection = NeoPrefs.m_bLeecherHashDetection;
	m_iDetectionLevel = NeoPrefs.m_iDetectionLevel;

	// Behavioural groupe
	m_uAgressionDetection = NeoPrefs.m_uAgressionDetection;
	m_uHashChangeDetection = NeoPrefs.m_uHashChangeDetection;

	m_bUploadFakerDetection = NeoPrefs.m_bUploadFakerDetection;
	m_bFileFakerDetection = NeoPrefs.m_bFileFakerDetection;
	m_bRankFloodDetection = NeoPrefs.m_bRankFloodDetection;
	m_bXsExploitDetection = NeoPrefs.m_bXsExploitDetection;
	m_bFileScannerDetection = NeoPrefs.m_bFileScannerDetection;
	m_uSpamerDetection = NeoPrefs.m_uSpamerDetection;

	m_bHashThiefDetection = NeoPrefs.m_bHashThiefDetection;
	m_uNickThiefDetection = NeoPrefs.m_uNickThiefDetection;
	m_bModThiefDetection = NeoPrefs.m_bModThiefDetection;

}

BOOL CPPgArgos::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	/*
	* Globale Einstellungen Speichern
	*/

	bool ArgosEngine = NeoPrefs.UseDLPScanner();

	NeoPrefs.m_bZeroScoreGPLBreaker = m_bZeroScoreGPLBreaker;
	NeoPrefs.m_iBanTime = m_iBanTime;
	NeoPrefs.m_bCloseMaellaBackdoor = m_bCloseMaellaBackdoor;

	// DLP Groupe
	NeoPrefs.m_bLeecherModDetection = m_bLeecherModDetection;
	NeoPrefs.m_bLeecherNickDetection = m_bLeecherNickDetection;
	NeoPrefs.m_bLeecherHashDetection = m_bLeecherHashDetection;
	NeoPrefs.m_iDetectionLevel = m_iDetectionLevel;

	// Behavioural groupe
	NeoPrefs.m_uAgressionDetection = m_uAgressionDetection;
	NeoPrefs.m_uHashChangeDetection = m_uHashChangeDetection;

	NeoPrefs.m_bUploadFakerDetection = m_bUploadFakerDetection;
	NeoPrefs.m_bFileFakerDetection = m_bFileFakerDetection;
	NeoPrefs.m_bRankFloodDetection = m_bRankFloodDetection;
	NeoPrefs.m_bXsExploitDetection = m_bXsExploitDetection;
	NeoPrefs.m_bFileScannerDetection = m_bFileScannerDetection;
	NeoPrefs.m_uSpamerDetection = m_uSpamerDetection;

	NeoPrefs.m_bHashThiefDetection = m_bHashThiefDetection;
	NeoPrefs.m_uNickThiefDetection = m_uNickThiefDetection;
	NeoPrefs.m_bModThiefDetection = m_bModThiefDetection;

	if(ArgosEngine != NeoPrefs.UseDLPScanner()){
		if(NeoPrefs.UseDLPScanner())
			theApp.argos->LoadDLPlibrary();
		else
			theApp.argos->UnLoadDLPlibrary();
	}

	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgArgos::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgArgos::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

LRESULT CPPgArgos::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){
			if(m_htiBanTime && pton->hItem == m_htiBanTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBanTime, MS2MIN(CLIENTBANTIME/2), MS2MIN(CLIENTBANTIME), MS2MIN(CLIENTBANTIME*4))) SetModified();
			}
		}else{
			if (pton->hItem && (pton->hItem == m_htiLeecherModDetection || pton->hItem == m_htiLeecherNickDetection || pton->hItem == m_htiLeecherHashDetection)){
				UINT bCheck;
				m_ctrlTreeOptions.GetCheckBox(m_htiLeecherModDetection, bCheck);
				if(!bCheck)
					m_ctrlTreeOptions.GetCheckBox(m_htiLeecherNickDetection, bCheck);
				if(!bCheck)
					m_ctrlTreeOptions.GetCheckBox(m_htiLeecherHashDetection, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiDetectionLevel, bCheck,FALSE);
				m_ctrlTreeOptions.SetItemEnable(m_htiDetectionLevel0, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiDetectionLevel1, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiDetectionLevel2, bCheck);
			}
			SetModified();
		}
	}
	return 0;
}

LRESULT CPPgArgos::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgArgos::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgArgos::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgArgos::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

#endif // ARGOS // NEO: NA END <-- Xanatos --