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
#include "UserMsgs.h"
#include "PPgSourceStorage.h"
#include "Preferences.h"
#include "Partfile.h"
#include "downloadqueue.h"
#include "knownfilelist.h"
#include "Neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "Neo/Functions.h"
#include "Neo/Defaults.h"
#include "KnownFileList.h"
#include "Opcodes.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CPPgSourceStorage dialog

IMPLEMENT_DYNAMIC(CPPgSourceStorage, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgSourceStorage, CPropertyPage)
	ON_WM_TIMER() // NEO: FCFG - [FileConfiguration]
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged) // NEO: FCFG - [FileConfiguration]
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgSourceStorage::CPPgSourceStorage()
	: CPropertyPage(CPPgSourceStorage::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	// NEO: FCFG - [FileConfiguration]
	m_paFiles = NULL;
	m_Category = NULL;
	m_bDataChanged = false;
	//m_strCaption = GetResString(IDS_FILEINFORMATION);
	//m_psp.pszTitle = m_strCaption;
	//m_psp.dwFlags |= PSP_USETITLE;
	m_timer = 0;
	// NEO: FCFG END
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgSourceStorage::~CPPgSourceStorage()
{
}

void CPPgSourceStorage::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_htiSourceStorage = NULL;
		m_htiAutoSaveSources = NULL;
			m_htiAutoSaveSourcesDisable = NULL;
			m_htiAutoSaveSourcesEnable1 = NULL;
			m_htiAutoSaveSourcesEnable2 = NULL;
			m_htiAutoSaveSourcesDefault = NULL;
			m_htiAutoSaveSourcesGlobal = NULL;

			m_htiAutoSaveSourcesIntervals = NULL;
			m_htiSourceStorageLimit = NULL;

			m_htiStoreAlsoA4AFSources = NULL;
				m_htiStoreAlsoA4AFSourcesDisable = NULL;
				m_htiStoreAlsoA4AFSourcesEnable = NULL;
				m_htiStoreAlsoA4AFSourcesDefault = NULL;
				m_htiStoreAlsoA4AFSourcesGlobal = NULL;
				
		m_htiAutoLoadSources = NULL;
			m_htiAutoLoadSourcesDisable = NULL;
			m_htiAutoLoadSourcesEnable1 = NULL;
			m_htiAutoLoadSourcesEnable2 = NULL;
			m_htiAutoLoadSourcesDefault = NULL;
			m_htiAutoLoadSourcesGlobal = NULL;

			m_htiLoadedSourceCleanUpTime = NULL;
			m_htiSourceStorageReaskLimit = NULL;

			m_htiTotalSourceRestore = NULL;
				m_htiTotalSourceRestoreDisable = NULL;
				m_htiTotalSourceRestoreEnable1 = NULL;
				m_htiTotalSourceRestoreEnable2 = NULL;
				m_htiTotalSourceRestoreDefault = NULL;
				m_htiTotalSourceRestoreGlobal = NULL;

		m_htiReaskManagement = NULL;
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			m_htiReaskPropability = NULL;

			m_htiUnpredictedPropability = NULL;
				m_htiUnpredictedPropabilityDisable = NULL;
				m_htiUnpredictedPropabilityEnable = NULL;
				m_htiUnpredictedPropabilityDefault = NULL;
				m_htiUnpredictedPropabilityGlobal = NULL;
				m_htiUnpredictedReaskPropability = NULL;
 #endif // NEO_SA // NEO: NSA END

			m_htiAutoReaskStoredSourcesDelay = NULL;

			m_htiGroupStoredSourceReask = NULL;
				m_htiGroupStoredSourceReaskDisable = NULL;
				m_htiGroupStoredSourceReaskEnable = NULL;
				m_htiGroupStoredSourceReaskDefault = NULL;
				m_htiGroupStoredSourceReaskGlobal = NULL;

				m_htiStoredSourceGroupIntervals = NULL;
				m_htiStoredSourceGroupSize = NULL;
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	m_htiClientDatabase = NULL;
		m_htiEnableSourceList = NULL;
		// cleanup
			m_htiSourceListExpirationTime = NULL;
				m_htiSourceListRunTimeCleanUp = NULL;
	// NEO: SFL - [SourceFileList]
	// seen files
		m_htiSaveSourceFileList = NULL;
			m_htiFileListExpirationTime = NULL;
	// NEO: SFL END
	// security
		m_htiIpSecurity = NULL;
			m_htiUseIPZoneCheck = NULL;
			m_htiSourceHashMonitor = NULL; // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
		m_htiIpTables = NULL;
			m_htiTableAmountToStore = NULL;
			m_htiIgnoreUndefinedIntervall = NULL;
			m_htiIgnoreUnreachableInterval = NULL;
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_htiSourceAnalyzer = NULL;
		m_htiEnableSourceAnalizer = NULL;
			m_htiAnalyzeIntervals = NULL;
			m_htiTableAmountToAnalyze = NULL;
	// gap handling
			m_htiHandleTableGaps = NULL;
			m_htiPriorityGapRatio = NULL;
			m_htiMaxGapSize = NULL;
			m_htiMaxGapTime = NULL;
	// analyze obtions
		m_htiAdvanced = NULL;
			m_htiMaxMidleDiscrepanceHigh = NULL;
			m_htiMaxMidleDiscrepanceLow = NULL;
			m_htiDualLinkedTableGravity = NULL;
	// calculation obtions
		m_htiCalculation = NULL;
			m_htiEnhancedFactor = NULL;
			m_htiFreshSourceTreshold = NULL;
			m_htiTempralIPBorderLine = NULL;
	// additional obtions
			m_htiAdvancedCalculation = NULL;
				m_htiLastSeenDurationThreshold = NULL;
				m_htiLinkTimeThreshold = NULL;
				m_htiMaxReliableTime = NULL;
#endif // NEO_SA // NEO: NSA END
}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
void CPPgSourceStorage::SetTreeRadioForNSS(HTREEITEM &htiDisable, HTREEITEM &htiEnable, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),Value,0),htiParent,GetResString(IDS_X_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable,MkRdBtnLbl(GetResString(IDS_X_ENABLE),Value,1),htiParent,GetResString(IDS_X_ENABLE_INFO),TRUE,Value.Val == 1);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

void CPPgSourceStorage::SetTreeRadioForNSS(HTREEITEM &htiDisable, HTREEITEM &htiEnable1, HTREEITEM &htiEnable2, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value, CString lbl1, CString Info1, CString lbl2, CString Info2)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),Value,0),htiParent,GetResString(IDS_X_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable1,MkRdBtnLbl(lbl1,Value,1),htiParent,Info1,TRUE,Value.Val == 1);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable2,MkRdBtnLbl(lbl2,Value,2),htiParent,Info2,TRUE,Value.Val == 2);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}
#endif // NEO_SS // NEO: NSS END

void CPPgSourceStorage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		
		int iImgStorage = 8;
		int iImgAutoSave = 8;
		int iImgSaveA4AF = 8;
		int iImgAutoLoad = 8;
		int iImgRestore = 8;
		int iImgManagement = 8;
		int iImgUnpredicted = 8;
		int iImgGroupe = 8;
		int iImgDB = 8;
		int iImgIPSec = 8;
		int iImgHandling = 8;
		int iImgAnalyzer = 8;
		int iImgAdvanced = 8;
		int iImgCalc = 8;
		int iImgAdvCalc = 8;
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgStorage = piml->Add(CTempIconLoader(_T("STORAGE")));
			iImgAutoSave = piml->Add(CTempIconLoader(_T("AUTOSAVESOURCES")));
			iImgSaveA4AF = piml->Add(CTempIconLoader(_T("STORA4AF")));
			iImgAutoLoad = piml->Add(CTempIconLoader(_T("AUTOLOADSOURCES")));
			iImgRestore = piml->Add(CTempIconLoader(_T("BOOTSTRAP")));
			iImgManagement = piml->Add(CTempIconLoader(_T("AUTOREASKLOADEDSOURCES")));
			iImgUnpredicted = piml->Add(CTempIconLoader(_T("UNPREDICTED")));
			iImgGroupe = piml->Add(CTempIconLoader(_T("GROUPESRC")));

			iImgDB = piml->Add(CTempIconLoader(_T("SOURCEDATABASE")));
			iImgIPSec = piml->Add(CTempIconLoader(_T("IPSECURITY")));
			iImgHandling = piml->Add(CTempIconLoader(_T("IPTABLES")));
			iImgAnalyzer = piml->Add(CTempIconLoader(_T("SOURCEANALIZER")));
			iImgAdvanced = piml->Add(CTempIconLoader(_T("ADVANCEDANALISE")));
			iImgCalc = piml->Add(CTempIconLoader(_T("CALCULATION")));
			iImgAdvCalc = piml->Add(CTempIconLoader(_T("ADVANCEDCALCULATION")));
		}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		SetTreeGroup(m_ctrlTreeOptions,m_htiSourceStorage,GetResString(IDS_X_SOURCE_STORAGE),iImgStorage, TVI_ROOT, GetResString(IDS_X_SOURCE_STORAGE_INFO));
	
			SetTreeGroup(m_ctrlTreeOptions,m_htiAutoSaveSources,GetResString(IDS_X_AUTO_SAVE_SOURCES),iImgAutoSave, m_htiSourceStorage, GetResString(IDS_X_AUTO_SAVE_SOURCES_INFO));
				SetTreeRadioForNSS(m_htiAutoSaveSourcesDisable,m_htiAutoSaveSourcesEnable1,m_htiAutoSaveSourcesEnable2,m_htiAutoSaveSourcesDefault, m_htiAutoSaveSourcesGlobal, m_htiAutoSaveSources, m_AutoSaveSources, GetResString(IDS_X_AUTO_SAVE_SOURCES_1), GetResString(IDS_X_AUTO_SAVE_SOURCES_1_INFO), GetResString(IDS_X_AUTO_SAVE_SOURCES_2), GetResString(IDS_X_AUTO_SAVE_SOURCES_2_INFO));

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiAutoSaveSourcesIntervals,GetResString(IDS_X_AUTO_SAVE_SOURCES_INTERVALS), m_htiAutoSaveSources, GetResString(IDS_X_AUTO_SAVE_SOURCES_INTERVALS_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceStorageLimit,GetResString(IDS_X_SAVE_SOURCES_LIMIT), m_htiAutoSaveSources, GetResString(IDS_X_SAVE_SOURCES_LIMIT_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiStoreAlsoA4AFSources,GetResString(IDS_X_SAVE_A4AF_SOURCES),iImgSaveA4AF, m_htiAutoSaveSources, GetResString(IDS_X_SAVE_A4AF_SOURCES_INFO));
					SetTreeRadioForNSS(m_htiStoreAlsoA4AFSourcesDisable,m_htiStoreAlsoA4AFSourcesEnable,m_htiStoreAlsoA4AFSourcesDefault, m_htiStoreAlsoA4AFSourcesGlobal, m_htiStoreAlsoA4AFSources, m_StoreAlsoA4AFSources);
				
			SetTreeGroup(m_ctrlTreeOptions,m_htiAutoLoadSources,GetResString(IDS_X_AUTO_LOAD_SOURCES),iImgAutoLoad, m_htiSourceStorage, GetResString(IDS_X_AUTO_LOAD_SOURCES_INFO));
				SetTreeRadioForNSS(m_htiAutoLoadSourcesDisable,m_htiAutoLoadSourcesEnable1,m_htiAutoLoadSourcesEnable2,m_htiAutoLoadSourcesDefault, m_htiAutoLoadSourcesGlobal, m_htiAutoLoadSources, m_AutoLoadSources, GetResString(IDS_X_AUTO_LOAD_SOURCES_1), GetResString(IDS_X_AUTO_LOAD_SOURCES_1_INFO), GetResString(IDS_X_AUTO_LOAD_SOURCES_2), GetResString(IDS_X_AUTO_LOAD_SOURCES_2_INFO));

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiLoadedSourceCleanUpTime,GetResString(IDS_X_LOAD_SOURCES_CLEANUP_TIME), m_htiAutoLoadSources, GetResString(IDS_X_LOAD_SOURCES_CLEANUP_TIME_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceStorageReaskLimit,GetResString(IDS_X_LOAD_SOURCES_REASK_LIMIT), m_htiAutoLoadSources, GetResString(IDS_X_LOAD_SOURCES_REASK_LIMIT_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiTotalSourceRestore,GetResString(IDS_X_SOURCE_RESTORE),iImgRestore, m_htiAutoLoadSources, GetResString(IDS_X_SOURCE_RESTORE_INFO));
					SetTreeRadioForNSS(m_htiTotalSourceRestoreDisable,m_htiTotalSourceRestoreEnable1,m_htiTotalSourceRestoreEnable2,m_htiTotalSourceRestoreDefault, m_htiTotalSourceRestoreGlobal, m_htiTotalSourceRestore, m_TotalSourceRestore, GetResString(IDS_X_ENABLE), GetResString(IDS_X_ENABLE_INFO), GetResString(IDS_X_SOURCE_RESTORE_NO_LOW), GetResString(IDS_X_SOURCE_RESTORE_NO_LOW_INFO));

			SetTreeGroup(m_ctrlTreeOptions,m_htiReaskManagement,GetResString(IDS_X_STORAGE_REASK_MANAGEMENT),iImgManagement, m_htiSourceStorage, GetResString(IDS_X_STORAGE_REASK_MANAGEMENT_INFO));
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiReaskPropability,GetResString(IDS_X_REASK_PROPABILITY), m_htiReaskManagement, GetResString(IDS_X_REASK_PROPABILITY_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiUnpredictedPropability,GetResString(IDS_X_UNPREDICTED_PROPABILITY),iImgUnpredicted, m_htiReaskManagement, GetResString(IDS_X_UNPREDICTED_PROPABILITY_INFO));
					SetTreeRadioForNSS(m_htiUnpredictedPropabilityDisable,m_htiUnpredictedPropabilityEnable,m_htiUnpredictedPropabilityDefault, m_htiUnpredictedPropabilityGlobal, m_htiUnpredictedPropability, m_UnpredictedPropability);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiUnpredictedReaskPropability,GetResString(IDS_X_REASK_PROPABILITY), m_htiUnpredictedPropability, GetResString(IDS_X_REASK_PROPABILITY_INFO));
 #endif // NEO_SA // NEO: NSA END

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiAutoReaskStoredSourcesDelay,GetResString(IDS_X_STORAGE_REASK_DELAY), m_htiReaskManagement, GetResString(IDS_X_STORAGE_REASK_DELAY_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiGroupStoredSourceReask,GetResString(IDS_X_GROUPE_STORAGE_REASKS),iImgGroupe, m_htiReaskManagement, GetResString(IDS_X_GROUPE_STORAGE_REASKS_INFO));
					SetTreeRadioForNSS(m_htiGroupStoredSourceReaskDisable,m_htiGroupStoredSourceReaskEnable,m_htiGroupStoredSourceReaskDefault, m_htiGroupStoredSourceReaskGlobal, m_htiGroupStoredSourceReask, m_GroupStoredSourceReask);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiStoredSourceGroupIntervals,GetResString(IDS_X_GROUPE_STORAGE_REASKS_INTERVALS), m_htiGroupStoredSourceReask, GetResString(IDS_X_GROUPE_STORAGE_REASKS_INTERVALS_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiStoredSourceGroupSize,GetResString(IDS_X_GROUPE_STORAGE_REASKS_SIZE), m_htiGroupStoredSourceReask, GetResString(IDS_X_GROUPE_STORAGE_REASKS_SIZE_INFO));
#endif // NEO_SS // NEO: NSS END

		if(m_Category == NULL && m_paFiles == NULL)
		{
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
			SetTreeGroup(m_ctrlTreeOptions,m_htiClientDatabase,GetResString(IDS_X_CLIENT_DATABASE),iImgDB, TVI_ROOT, GetResString(IDS_X_CLIENT_DATABASE_INFO));
				SetTreeCheck(m_ctrlTreeOptions,m_htiEnableSourceList,GetResString(IDS_X_ENABLE_CLIENT_DATABASE),m_htiClientDatabase,GetResString(IDS_X_ENABLE_CLIENT_DATABASE_INFO),FALSE,m_bEnableSourceList);
	// cleanup
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceListExpirationTime,GetResString(IDS_X_CLIENT_DATABASE_EXPIRATION),m_htiEnableSourceList,GetResString(IDS_X_CLIENT_DATABASE_EXPIRATION_INFO));
						SetTreeCheck(m_ctrlTreeOptions,m_htiSourceListRunTimeCleanUp,GetResString(IDS_X_CLIENT_DATABASE_CLEANUP),m_htiSourceListExpirationTime,GetResString(IDS_X_CLIENT_DATABASE_CLEANUP_INFO),TRUE,m_uSourceListRunTimeCleanUp);
	// NEO: SFL - [SourceFileList]
	// seen files
				SetTreeCheck(m_ctrlTreeOptions,m_htiSaveSourceFileList,GetResString(IDS_X_CLIENT_FILE_LIST),m_htiClientDatabase,GetResString(IDS_X_CLIENT_FILE_LIST_INFO),FALSE,m_bSaveSourceFileList);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiFileListExpirationTime,GetResString(IDS_X_CLIENT_FILE_LIST_EXPIRATION), m_htiSaveSourceFileList,GetResString(IDS_X_CLIENT_FILE_LIST_EXPIRATION_INFO));
	// NEO: SFL END
	// security
				SetTreeGroup(m_ctrlTreeOptions,m_htiIpSecurity,GetResString(IDS_X_CLIENT_DATABASE_SECURITY),iImgIPSec, m_htiClientDatabase, GetResString(IDS_X_CLIENT_DATABASE_SECURITY_INFO));
					SetTreeCheck(m_ctrlTreeOptions,m_htiUseIPZoneCheck,GetResString(IDS_X_CHECK_IP_ZONE),m_htiIpSecurity,GetResString(IDS_X_CHECK_IP_ZONE_INFO),FALSE,m_bUseIPZoneCheck);
					SetTreeCheck(m_ctrlTreeOptions,m_htiSourceHashMonitor,GetResString(IDS_X_MONITOR_HASH_CHANGES),m_htiIpSecurity,GetResString(IDS_X_MONITOR_HASH_CHANGES_INFO),FALSE,m_bSourceHashMonitor); // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
				SetTreeGroup(m_ctrlTreeOptions,m_htiIpTables,GetResString(IDS_X_IP_TABLE_HANDLING),iImgHandling, m_htiClientDatabase, GetResString(IDS_X_IP_TABLE_HANDLING_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiTableAmountToStore,GetResString(IDS_X_IP_TABLE_AMOUNT), m_htiIpTables,GetResString(IDS_X_IP_TABLE_AMOUNT_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiIgnoreUndefinedIntervall,GetResString(IDS_X_IP_TABLE_IGNORE_UNKNOWN), m_htiIpTables,GetResString(IDS_X_IP_TABLE_IGNORE_UNKNOWN_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiIgnoreUnreachableInterval,GetResString(IDS_X_IP_TABLE_IGNORE_FAILD), m_htiIpTables,GetResString(IDS_X_IP_TABLE_IGNORE_FAILD_INFO));
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			SetTreeGroup(m_ctrlTreeOptions,m_htiSourceAnalyzer,GetResString(IDS_X_SOURCE_ANALYZER),iImgAnalyzer, TVI_ROOT, GetResString(IDS_X_SOURCE_ANALYZER_INFO));
				SetTreeCheck(m_ctrlTreeOptions,m_htiEnableSourceAnalizer,GetResString(IDS_X_ENABLE_SOURCE_ANALYZER),m_htiSourceAnalyzer,GetResString(IDS_X_ENABLE_SOURCE_ANALYZER_INFO),FALSE,m_bEnableSourceAnalizer);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiAnalyzeIntervals,GetResString(IDS_X_CLIENT_ANALYZER_INTERVALS),m_htiEnableSourceAnalizer,GetResString(IDS_X_CLIENT_ANALYZER_INTERVALS_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiTableAmountToAnalyze,GetResString(IDS_X_IP_TABLES_TO_ANALYZER),m_htiEnableSourceAnalizer,GetResString(IDS_X_IP_TABLES_TO_ANALYZER_INFO));
	// gap handling
				SetTreeCheck(m_ctrlTreeOptions,m_htiHandleTableGaps,GetResString(IDS_X_HANDLE_IP_TABLE_GAPS),m_htiSourceAnalyzer,GetResString(IDS_X_HANDLE_IP_TABLE_GAPS_INFO),FALSE,m_bHandleTableGaps);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiPriorityGapRatio,GetResString(IDS_X_PRIOR_GAP_RATIO),m_htiHandleTableGaps,GetResString(IDS_X_PRIOR_GAP_RATIO_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxGapSize,GetResString(IDS_X_MAX_GAP_SIZE),m_htiHandleTableGaps,GetResString(IDS_X_MAX_GAP_SIZE_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxGapTime,GetResString(IDS_X_MAX_GAP_TIME),m_htiHandleTableGaps,GetResString(IDS_X_MAX_GAP_TIME_INFO));
	// analyze obtions
				SetTreeGroup(m_ctrlTreeOptions,m_htiAdvanced,GetResString(IDS_X_ADVANCED_ANALYZER),iImgAdvanced, m_htiSourceAnalyzer, GetResString(IDS_X_ADVANCED_ANALYZER_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxMidleDiscrepanceHigh,GetResString(IDS_X_MAX_MINDLE_DESCR_HIGH),m_htiAdvanced,GetResString(IDS_X_MAX_MINDLE_DESCR_HIGH_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxMidleDiscrepanceLow,GetResString(IDS_X_MAX_MINDLE_DESCR_LOW),m_htiAdvanced,GetResString(IDS_X_MAX_MINDLE_DESCR_LOW_INFO));
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiDualLinkedTableGravity,GetResString(IDS_X_DUAL_LINKED_TABLE_GRAVITY),m_htiAdvanced,GetResString(IDS_X_DUAL_LINKED_TABLE_GRAVITY_INFO),FALSE,m_bDualLinkedTableGravity);
	// calculation obtions
				SetTreeGroup(m_ctrlTreeOptions,m_htiCalculation,GetResString(IDS_X_CALCULATION),iImgCalc, m_htiSourceAnalyzer, GetResString(IDS_X_CALCULATION_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiEnhancedFactor,GetResString(IDS_X_MAX_MIDLE_ENHANCED_FACTOR),m_htiCalculation,GetResString(IDS_X_MAX_MIDLE_ENHANCED_FACTOR_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiFreshSourceTreshold,GetResString(IDS_X_FRESH_SOURCE_TRESHOLD),m_htiCalculation,GetResString(IDS_X_FRESH_SOURCE_TRESHOLD_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiTempralIPBorderLine,GetResString(IDS_X_TEMPRAL_IP_BORDERLINE),m_htiCalculation,GetResString(IDS_X_TEMPRAL_IP_BORDERLINE_INFO));
	// additional obtions
					SetTreeGroup(m_ctrlTreeOptions,m_htiAdvancedCalculation,GetResString(IDS_X_ADVANCED_CALCULATION),iImgAdvCalc, m_htiAdvanced, GetResString(IDS_X_ADVANCED_CALCULATION_INFO));
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiLastSeenDurationThreshold,GetResString(IDS_X_LAST_SEEN_THRESHOLD),m_htiAdvancedCalculation,GetResString(IDS_X_LAST_SEEN_THRESHOLD_INFO));
						SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiLinkTimeThreshold,GetResString(IDS_X_LINK_TIME_THRESHOLD),m_htiAdvancedCalculation,GetResString(IDS_X_LINK_TIME_THRESHOLD_INFO),FALSE,m_bLinkTimePropability);
						SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiMaxReliableTime,GetResString(IDS_X_SCALE_RELIABLE_TIME),m_htiAdvancedCalculation,GetResString(IDS_X_SCALE_RELIABLE_TIME_INFO),FALSE,m_bScaleReliableTime);
#endif // NEO_SA // NEO: NSA END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			m_ctrlTreeOptions.Expand(m_htiSourceStorage, TVE_EXPAND);
#endif // NEO_SS // NEO: NSS END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
			m_ctrlTreeOptions.Expand(m_htiClientDatabase, TVE_EXPAND);

			m_ctrlTreeOptions.SetItemEnable(m_htiSourceListExpirationTime, m_bEnableSourceList);
			m_ctrlTreeOptions.SetItemEnable(m_htiSaveSourceFileList, m_bEnableSourceList);
			//m_ctrlTreeOptions.SetItemEnable(m_htiIpSecurity, m_bEnableSourceList);
			m_ctrlTreeOptions.SetItemEnable(m_htiIpTables, m_bEnableSourceList);

			m_ctrlTreeOptions.SetItemEnable(m_htiFileListExpirationTime, m_bSaveSourceFileList); // NEO: SFL - [SourceFileList]

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			m_ctrlTreeOptions.SetGroupEnable(m_htiSourceAnalyzer,m_bEnableSourceList);
 #endif // NEO_SA // NEO: NSA END
#endif // NEO_CD // NEO: NCD END

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			m_ctrlTreeOptions.Expand(m_htiSourceAnalyzer, TVE_EXPAND);

			if(m_bEnableSourceList)
			{
				m_ctrlTreeOptions.SetItemEnable(m_htiAnalyzeIntervals, m_bEnableSourceAnalizer);
				m_ctrlTreeOptions.SetItemEnable(m_htiTableAmountToAnalyze, m_bEnableSourceAnalizer);

				m_ctrlTreeOptions.SetItemEnable(m_htiHandleTableGaps, m_bEnableSourceAnalizer);
				m_ctrlTreeOptions.SetItemEnable(m_htiAdvanced, m_bEnableSourceAnalizer);
				m_ctrlTreeOptions.SetItemEnable(m_htiCalculation, m_bEnableSourceAnalizer);
				if(m_bEnableSourceAnalizer)
				{
					m_ctrlTreeOptions.SetItemEnable(m_htiPriorityGapRatio, m_bHandleTableGaps);
					m_ctrlTreeOptions.SetItemEnable(m_htiMaxGapSize, m_bHandleTableGaps);
					m_ctrlTreeOptions.SetItemEnable(m_htiMaxGapTime, m_bHandleTableGaps);

					m_ctrlTreeOptions.SetItemEnable(m_htiDualLinkedTableGravity, m_bDualLinkedTableGravity,FALSE,TRUE);
					m_ctrlTreeOptions.SetItemEnable(m_htiLinkTimeThreshold, m_bLinkTimePropability,FALSE,TRUE);
					m_ctrlTreeOptions.SetItemEnable(m_htiMaxReliableTime, m_bScaleReliableTime,FALSE,TRUE);
				}
			}
#endif // NEO_SA // NEO: NSA END
		}
		else
		{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			m_ctrlTreeOptions.Expand(m_htiSourceStorage, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiAutoSaveSources, TVE_EXPAND);
			m_ctrlTreeOptions.Expand(m_htiAutoLoadSources, TVE_EXPAND);
#endif // NEO_SS // NEO: NSS END
		}

		m_bInitializedTreeOpts = true;
	}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiAutoSaveSources, m_AutoSaveSources);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAutoSaveSourcesIntervals, m_AutoSaveSourcesIntervals);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceStorageLimit, m_SourceStorageLimit);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiStoreAlsoA4AFSources, m_StoreAlsoA4AFSources);
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiAutoLoadSources, m_AutoLoadSources);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiLoadedSourceCleanUpTime, m_LoadedSourceCleanUpTime);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceStorageReaskLimit, m_SourceStorageReaskLimit);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiTotalSourceRestore, m_TotalSourceRestore);
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReaskPropability, m_ReaskPropability);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiUnpredictedPropability, m_UnpredictedPropability);
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiUnpredictedReaskPropability, m_UnpredictedReaskPropability);
 #endif // NEO_SA // NEO: NSA END
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAutoReaskStoredSourcesDelay, m_AutoReaskStoredSourcesDelay);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiGroupStoredSourceReask, m_GroupStoredSourceReask);
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiStoredSourceGroupIntervals, m_StoredSourceGroupIntervals);
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiStoredSourceGroupSize, m_StoredSourceGroupSize);
#endif // NEO_SS // NEO: NSS END

	if(m_Category == NULL && m_paFiles == NULL)
	{
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiEnableSourceList, m_bEnableSourceList);
	// cleanup
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceListExpirationTime, m_iSourceListExpirationTime);
				DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSourceListRunTimeCleanUp, m_uSourceListRunTimeCleanUp);
	// NEO: SFL - [SourceFileList]
	// seen files
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSaveSourceFileList, m_bSaveSourceFileList);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiFileListExpirationTime, m_iFileListExpirationTime);
	// NEO: SFL END
	// security
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseIPZoneCheck, m_bUseIPZoneCheck);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSourceHashMonitor, m_bSourceHashMonitor); // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiTableAmountToStore, m_iTableAmountToStore);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiIgnoreUndefinedIntervall, m_iIgnoreUndefinedIntervall);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiIgnoreUnreachableInterval, m_iIgnoreUnreachableInterval);
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiEnableSourceAnalizer, m_bEnableSourceAnalizer);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAnalyzeIntervals, m_iAnalyzeIntervals);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiTableAmountToAnalyze, m_iTableAmountToAnalyze);
	// gap handling
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiHandleTableGaps, m_bHandleTableGaps);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiPriorityGapRatio, m_fPriorityGapRatio);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMaxGapSize, m_iMaxGapSize);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMaxGapTime, m_iMaxGapTime);
	// analyze obtions
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMaxMidleDiscrepanceHigh, m_fMaxMidleDiscrepanceHigh);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMaxMidleDiscrepanceLow, m_fMaxMidleDiscrepanceLow);
				DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDualLinkedTableGravity, m_bDualLinkedTableGravity);
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiDualLinkedTableGravity, m_iDualLinkedTableGravity);
	// calculation obtions
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiEnhancedFactor, m_fEnhancedFactor);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiFreshSourceTreshold, m_iFreshSourceTreshold);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiTempralIPBorderLine, m_iTempralIPBorderLine);
	// additional obtions
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiLastSeenDurationThreshold, m_fLastSeenDurationThreshold);
					DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLinkTimeThreshold, m_bLinkTimePropability);
						DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiLinkTimeThreshold, m_iLinkTimeThreshold);
					DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMaxReliableTime, m_bScaleReliableTime);
						DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMaxReliableTime, m_iDualLinkedTableGravity);
#endif // NEO_SA // NEO: NSA END
	}
}

BOOL CPPgSourceStorage::OnInitDialog()
{
	// NEO: FCFG - [FileConfiguration]
	if(m_paFiles)
		RefreshData();
	else
	// NEO: FCFG END
		LoadSettings();
		
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	// NEO: FCFG - [FileConfiguration]
	// no need to explicitly call 'RefreshData' here, 'OnSetActive' will be called right after 'OnInitDialog'

	// start time for calling 'RefreshData'
	if(m_paFiles)
		VERIFY( (m_timer = SetTimer(301, 5000, 0)) != NULL );
	// NEO: FCFG END

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgSourceStorage::SetLimits()
{
	if(m_Category == NULL)
	{
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
		m_iTableAmountToStore.MSMG(MIN_TABLE_AMOUT_TO_STORE, DEF_TABLE_AMOUT_TO_STORE, MAX_TABLE_AMOUT_TO_STORE, NeoPrefs.m_iTableAmountToStore);
		// cleanup
		m_iSourceListExpirationTime.MSMG(MIN_SOURCE_EXPIRATION_TIME, DEF_SOURCE_EXPIRATION_TIME, MAX_SOURCE_EXPIRATION_TIME, NeoPrefs.m_iSourceListExpirationTime, 't');
		// NEO: SFL - [SourceFileList]
		// seen files
		m_iFileListExpirationTime.MSMG(MIN_FILE_EXPIRATION_TIME, DEF_FILE_EXPIRATION_TIME, MAX_FILE_EXPIRATION_TIME, NeoPrefs.m_iFileListExpirationTime, 't');
		// NEO: SFL END
		// aditional obtions
		m_iIgnoreUndefinedIntervall.MSMG(MIN_IGNORE_UNREACHABLE_INTERVAL, DEF_IGNORE_UNREACHABLE_INTERVAL, MAX_IGNORE_UNREACHABLE_INTERVAL, NeoPrefs.m_iIgnoreUndefinedIntervall, 't');
		m_iIgnoreUnreachableInterval.MSMG(MIN_IGNORE_UNDEFINED_INTERVAL, DEF_IGNORE_UNDEFINED_INTERVAL, MAX_IGNORE_UNDEFINED_INTERVAL, NeoPrefs.m_iIgnoreUnreachableInterval, 't');
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		m_iAnalyzeIntervals.MSMG(MIN_ANALISIS_INTERVALS, DEF_ANALISIS_INTERVALS, MAX_ANALISIS_INTERVALS, NeoPrefs.m_iAnalyzeIntervals, 't');
		m_iTableAmountToAnalyze.MSMG(MIN_TABLE_AMOUT_TO_ANALISE, DEF_TABLE_AMOUT_TO_ANALISE, MAX_TABLE_AMOUT_TO_ANALISE, NeoPrefs.m_iTableAmountToAnalyze);
		// gap handling
		m_fPriorityGapRatio.MSMG(MIN_PRIORITY_GAP_RATIO, DEF_PRIORITY_GAP_RATIO, MAX_PRIORITY_GAP_RATIO, NeoPrefs.m_fPriorityGapRatio, 'f');
		m_iMaxGapSize.MSMG(MIN_MAX_GAP_SIZE, DEF_MAX_GAP_SIZE, MAX_MAX_GAP_SIZE, NeoPrefs.m_iMaxGapSize);
		m_iMaxGapTime.MSMG(MIN_MAX_GAP_TIME, DEF_MAX_GAP_TIME, MAX_MAX_GAP_TIME, NeoPrefs.m_iMaxGapTime, 't');
		// analyze obtions
		m_fMaxMidleDiscrepanceHigh.MSMG(MIN_MAX_MIDLE_DISCREPANCE_HIGH, DEF_MAX_MIDLE_DISCREPANCE_HIGH, MAX_MAX_MIDLE_DISCREPANCE_HIGH, NeoPrefs.m_fMaxMidleDiscrepanceHigh, 'f');
		m_fMaxMidleDiscrepanceLow.MSMG(MIN_MAX_MIDLE_DISCREPANCE_LOW, DEF_MAX_MIDLE_DISCREPANCE_LOW, MAX_MAX_MIDLE_DISCREPANCE_LOW, NeoPrefs.m_fMaxMidleDiscrepanceLow, 'f');
		m_iDualLinkedTableGravity.MSMG(MIN_DUAL_LINKED_TABLE_GRAVITY, DEF_DUAL_LINKED_TABLE_GRAVITY, MAX_DUAL_LINKED_TABLE_GRAVITY, NeoPrefs.m_iDualLinkedTableGravity);
		// calculation obtions
		m_fEnhancedFactor.MSMG(MIN_VAL_ENHANCED_FACTOR, DEF_VAL_ENHANCED_FACTOR, MAX_VAL_ENHANCED_FACTOR, NeoPrefs.m_fEnhancedFactor, 'f');
		m_iFreshSourceTreshold.MSMG(MIN_FRESH_SOURCE_TRESHOLD, DEF_FRESH_SOURCE_TRESHOLD, MAX_FRESH_SOURCE_TRESHOLD, NeoPrefs.m_iFreshSourceTreshold, 't');
		m_iTempralIPBorderLine.MSMG(MIN_TEMPORAL_IP_BORDERLINE, DEF_TEMPORAL_IP_BORDERLINE, MAX_TEMPORAL_IP_BORDERLINE, NeoPrefs.m_iTempralIPBorderLine, 't');
		// additional obtions
		m_fLastSeenDurationThreshold.MSMG(MIN_LAST_SEEN_DURATION_THRESHOLD, DEF_LAST_SEEN_DURATION_THRESHOLD, MAX_LAST_SEEN_DURATION_THRESHOLD, NeoPrefs.m_fLastSeenDurationThreshold, 'f');
		m_iLinkTimeThreshold.MSMG(MIN_LINK_TIME_THRESHOLD, DEF_LINK_TIME_THRESHOLD, MAX_LINK_TIME_THRESHOLD, NeoPrefs.m_iLinkTimeThreshold, 't');
		m_iMaxReliableTime.MSMG(MIN_RELIABLE_TIME, DEF_RELIABLE_TIME, MAX_RELIABLE_TIME, NeoPrefs.m_iMaxReliableTime, 't');
#endif // NEO_SA // NEO: NSA END
	}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_AutoSaveSourcesIntervals.MSMG(MIN_AUTO_SAVE_SOURCES_INTERVALS, DEF_AUTO_SAVE_SOURCES_INTERVALS, MAX_AUTO_SAVE_SOURCES_INTERVALS, NeoPrefs.PartPrefs.m_AutoSaveSourcesIntervals, 't');
	m_SourceStorageLimit.MSMG(MIN_SOURCE_STORAGE_LIMIT, DEF_SOURCE_STORAGE_LIMIT, MAX_SOURCE_STORAGE_LIMIT, NeoPrefs.PartPrefs.m_SourceStorageLimit, 'n');

	m_LoadedSourceCleanUpTime.MSMG(MIN_LOADED_SOURCE_CLEAN_UP_TIME, DEF_LOADED_SOURCE_CLEAN_UP_TIME, MAX_LOADED_SOURCE_CLEAN_UP_TIME, NeoPrefs.PartPrefs.m_LoadedSourceCleanUpTime, 't');
	m_SourceStorageReaskLimit.MSMG(MIN_SOURCE_STORAGE_REASK_LIMIT, DEF_SOURCE_STORAGE_REASK_LIMIT, MAX_SOURCE_STORAGE_REASK_LIMIT, NeoPrefs.PartPrefs.m_SourceStorageReaskLimit, 'n');
	m_SourceStorageReaskLimit.MSMa(MIN_SOURCE_STORAGE_REASK_LIMIT/10, DEF_SOURCE_STORAGE_REASK_LIMIT/10, MAX_SOURCE_STORAGE_REASK_LIMIT/10, 1, 'n');

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_ReaskPropability.MSMG(MIN_REASK_PROPABILITY, DEF_REASK_PROPABILITY, MAX_REASK_PROPABILITY, NeoPrefs.PartPrefs.m_ReaskPropability);
	m_UnpredictedReaskPropability.MSMG(MIN_UNPREDICTED_REASK_PROPABILITY, DEF_UNPREDICTED_REASK_PROPABILITY, MAX_UNPREDICTED_REASK_PROPABILITY, NeoPrefs.PartPrefs.m_UnpredictedReaskPropability);
 #endif // NEO_SA // NEO: NSA END

	m_AutoReaskStoredSourcesDelay.MSMG(MIN_AUTO_REASK_STORED_SOURCES_DELAY, DEF_AUTO_REASK_STORED_SOURCES_DELAY, MAX_AUTO_REASK_STORED_SOURCES_DELAY, NeoPrefs.PartPrefs.m_AutoReaskStoredSourcesDelay, 't');

	m_StoredSourceGroupIntervals.MSMG(MIN_STORED_SOURCE_GROUP_INTERVALS, DEF_STORED_SOURCE_GROUP_INTERVALS, MAX_STORED_SOURCE_GROUP_INTERVALS, NeoPrefs.PartPrefs.m_StoredSourceGroupIntervals, 't');
	m_StoredSourceGroupSize.MSMG(MIN_STORED_SOURCE_GROUP_SIZE, DEF_STORED_SOURCE_GROUP_SIZE, MAX_STORED_SOURCE_GROUP_SIZE, NeoPrefs.PartPrefs.m_StoredSourceGroupSize);
#endif // NEO_SS // NEO: NSS END

}

void CPPgSourceStorage::LoadSettings()
{
	/*
	* Einstellungen Laden
	*/

	SetLimits();

	if(m_Category)
	{
		CPartPreferences* PartPrefs = m_Category->PartPrefs;

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		m_AutoSaveSources.CVDC(2, PartPrefs ? PartPrefs->m_AutoSaveSources : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoSaveSources);
		m_AutoSaveSourcesIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_AutoSaveSourcesIntervals : FCFG_DEF);
		m_SourceStorageLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SourceStorageLimit : FCFG_DEF);

		m_StoreAlsoA4AFSources.CVDC(1, PartPrefs ? PartPrefs->m_StoreAlsoA4AFSources : FCFG_DEF, NeoPrefs.PartPrefs.m_StoreAlsoA4AFSources);

		m_AutoLoadSources.CVDC(2, PartPrefs ? PartPrefs->m_AutoLoadSources : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoLoadSources);
		m_LoadedSourceCleanUpTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_LoadedSourceCleanUpTime : FCFG_DEF);
		m_SourceStorageReaskLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SourceStorageReaskLimit : FCFG_DEF);

		m_TotalSourceRestore.CVDC(2, PartPrefs ? PartPrefs->m_TotalSourceRestore : FCFG_DEF, NeoPrefs.PartPrefs.m_TotalSourceRestore);

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		m_ReaskPropability.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_ReaskPropability : FCFG_DEF);
		m_UnpredictedPropability.CVDC(1, PartPrefs ? PartPrefs->m_UnpredictedPropability : FCFG_DEF, NeoPrefs.PartPrefs.m_UnpredictedPropability);
		m_UnpredictedReaskPropability.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_UnpredictedReaskPropability : FCFG_DEF);
 #endif // NEO_SA // NEO: NSA END

		m_AutoReaskStoredSourcesDelay.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_AutoReaskStoredSourcesDelay : FCFG_DEF);

		m_GroupStoredSourceReask.CVDC(1, PartPrefs ? PartPrefs->m_GroupStoredSourceReask : FCFG_DEF, NeoPrefs.PartPrefs.m_GroupStoredSourceReask);
		m_StoredSourceGroupIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_StoredSourceGroupIntervals : FCFG_DEF);
		m_StoredSourceGroupSize.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_StoredSourceGroupSize : FCFG_DEF);
#endif // NEO_SS // NEO: NSS END

	}
	else
	{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		m_AutoSaveSources.CVDC(2, NeoPrefs.PartPrefs.m_AutoSaveSources);
		m_AutoSaveSourcesIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_AutoSaveSourcesIntervals, true);
		m_SourceStorageLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SourceStorageLimit, true);

		m_StoreAlsoA4AFSources.CVDC(1, NeoPrefs.PartPrefs.m_StoreAlsoA4AFSources);

		m_AutoLoadSources.CVDC(2, NeoPrefs.PartPrefs.m_AutoLoadSources);
		m_LoadedSourceCleanUpTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_LoadedSourceCleanUpTime, true);
		m_SourceStorageReaskLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SourceStorageReaskLimit, true);

		m_TotalSourceRestore.CVDC(2, NeoPrefs.PartPrefs.m_TotalSourceRestore);

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		m_ReaskPropability.DV(FCFG_STD, NeoPrefs.PartPrefs.m_ReaskPropability, true);
		m_UnpredictedPropability.CVDC(1, NeoPrefs.PartPrefs.m_UnpredictedPropability);
		m_UnpredictedReaskPropability.DV(FCFG_STD, NeoPrefs.PartPrefs.m_UnpredictedReaskPropability, true);
 #endif // NEO_SA // NEO: NSA END

		m_AutoReaskStoredSourcesDelay.DV(FCFG_STD, NeoPrefs.PartPrefs.m_AutoReaskStoredSourcesDelay, true);

		m_GroupStoredSourceReask.CVDC(1, NeoPrefs.PartPrefs.m_GroupStoredSourceReask);
		m_StoredSourceGroupIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_StoredSourceGroupIntervals, true);
		m_StoredSourceGroupSize.DV(FCFG_STD, NeoPrefs.PartPrefs.m_StoredSourceGroupSize, true);
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
		m_bEnableSourceList = NeoPrefs.m_bEnableSourceList;
		// cleanup
		m_uSourceListRunTimeCleanUp = NeoPrefs.m_uSourceListRunTimeCleanUp;
		// NEO: SFL - [SourceFileList]
		// seen files
		m_bSaveSourceFileList = NeoPrefs.m_bSaveSourceFileList;
		// NEO: SFL END
		// security
		m_bUseIPZoneCheck = NeoPrefs.m_bUseIPZoneCheck;
		m_bSourceHashMonitor = NeoPrefs.m_bSourceHashMonitor; // NEO: SHM - [SourceHashMonitor]
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		m_bEnableSourceAnalizer = NeoPrefs.m_bEnableSourceAnalizer;
		// gap handling
		m_bHandleTableGaps = NeoPrefs.m_bHandleTableGaps;
		// analyze obtions
		m_bDualLinkedTableGravity = NeoPrefs.m_bDualLinkedTableGravity;
		// additional obtions
		m_bLinkTimePropability = NeoPrefs.m_bLinkTimePropability;
		m_bScaleReliableTime = NeoPrefs.m_bScaleReliableTime;
#endif // NEO_SA // NEO: NSA END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
		m_iTableAmountToStore.DV(FCFG_STD, NeoPrefs.m_iTableAmountToStore, true);
		// cleanup
		m_iSourceListExpirationTime.DV(FCFG_STD, NeoPrefs.m_iSourceListExpirationTime, true);
		// NEO: SFL - [SourceFileList]
		// seen files
		m_iFileListExpirationTime.DV(FCFG_STD, NeoPrefs.m_iFileListExpirationTime, true);
		// NEO: SFL END
		// aditional obtions
		m_iIgnoreUndefinedIntervall.DV(FCFG_STD, NeoPrefs.m_iIgnoreUndefinedIntervall, true);
		m_iIgnoreUnreachableInterval.DV(FCFG_STD, NeoPrefs.m_iIgnoreUnreachableInterval, true);
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		m_iAnalyzeIntervals.DV(FCFG_STD, NeoPrefs.m_iAnalyzeIntervals, true);
		m_iTableAmountToAnalyze.DV(FCFG_STD, NeoPrefs.m_iTableAmountToAnalyze, true);
		// gap handling
		m_fPriorityGapRatio.DV(FCFG_STD, NeoPrefs.m_fPriorityGapRatio, true);
		m_iMaxGapSize.DV(FCFG_STD, NeoPrefs.m_iMaxGapSize, true);
		m_iMaxGapTime.DV(FCFG_STD, NeoPrefs.m_iMaxGapTime, true);
		// analyze obtions
		m_fMaxMidleDiscrepanceHigh.DV(FCFG_STD, NeoPrefs.m_fMaxMidleDiscrepanceHigh, true);
		m_fMaxMidleDiscrepanceLow.DV(FCFG_STD, NeoPrefs.m_fMaxMidleDiscrepanceLow, true);
		m_iDualLinkedTableGravity.DV(FCFG_STD, NeoPrefs.m_iDualLinkedTableGravity, true);
		// calculation obtions
		m_fEnhancedFactor.DV(FCFG_STD, NeoPrefs.m_fEnhancedFactor, true);
		m_iFreshSourceTreshold.DV(FCFG_STD, NeoPrefs.m_iFreshSourceTreshold, true);
		m_iTempralIPBorderLine.DV(FCFG_STD, NeoPrefs.m_iTempralIPBorderLine, true);
		// additional obtions
		m_fLastSeenDurationThreshold.DV(FCFG_STD, NeoPrefs.m_fLastSeenDurationThreshold, true);
		m_iLinkTimeThreshold.DV(FCFG_STD, NeoPrefs.m_iLinkTimeThreshold, true);
		m_iMaxReliableTime.DV(FCFG_STD, NeoPrefs.m_iMaxReliableTime, true);
#endif // NEO_SA // NEO: NSA END

	}

	SetAuxSet(m_Category == NULL);
	SetAut();
}

void CPPgSourceStorage::SetAuxSet(bool bGlobal)
{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_SourceStorageReaskLimit.aS(bGlobal ? !(m_bEnableSourceAnalizer && m_bEnableSourceList) : !NeoPrefs.EnableSourceAnalizer());
 #else
	m_SourceStorageReaskLimit.aS(true);
 #endif // NEO_SA // NEO: NSA END
#endif // NEO_SS // NEO: NSS END
}

void CPPgSourceStorage::SetAutVal(SintD &Val, int Ref, int Fac, int Set)
{
	if(Ref == FCFG_UNK){
		Val.A(FCFG_UNK,Set);
	}else{
		int Lim = Ref / Fac;
		MinMax(&Lim,Val.Min[Set],Val.Max[Set]);
		Val.A(Lim,Set);
	}
}

void CPPgSourceStorage::SetAut()
{
	int SourceLimit = NeoPrefs.PartPrefs.m_SourceLimit;

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	SetAutVal(m_SourceStorageLimit,SourceLimit,1);
	SetAutVal(m_SourceStorageReaskLimit,SourceLimit,4);
	SetAutVal(m_SourceStorageReaskLimit,SourceLimit,4,1);
#endif // NEO_SS // NEO: NSS END
}

// NEO: FCFG - [FileConfiguration]
void CPPgSourceStorage::RefreshData()
{
	/*
	* Datei Einstellungen Laden
	*/

	SetLimits();

	if(!theApp.downloadqueue->IsPartFile((CPartFile*)(*m_paFiles)[0]))
		return;

	const CPartFile* PartFile = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[0]);

	CPartPreferences* PartPrefs = PartFile->PartPrefs;
	Category_Struct* Category = thePrefs.GetCategory(PartFile->GetCategory());
	ASSERT(Category);

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_AutoSaveSources.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoSaveSources : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoSaveSources, Category && Category->PartPrefs ? Category->PartPrefs->m_AutoSaveSources : -1);;
	m_AutoSaveSourcesIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_AutoSaveSourcesIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoSaveSourcesIntervals : FCFG_DEF);
	m_SourceStorageLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SourceStorageLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SourceStorageLimit : FCFG_DEF);

	m_StoreAlsoA4AFSources.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_StoreAlsoA4AFSources : FCFG_DEF, NeoPrefs.PartPrefs.m_StoreAlsoA4AFSources, Category && Category->PartPrefs ? Category->PartPrefs->m_StoreAlsoA4AFSources : -1);;

	m_AutoLoadSources.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoLoadSources : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoLoadSources, Category && Category->PartPrefs ? Category->PartPrefs->m_AutoLoadSources : -1);;
	m_LoadedSourceCleanUpTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_LoadedSourceCleanUpTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_LoadedSourceCleanUpTime : FCFG_DEF);
	m_SourceStorageReaskLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SourceStorageReaskLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SourceStorageReaskLimit : FCFG_DEF);

	m_TotalSourceRestore.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_TotalSourceRestore : FCFG_DEF, NeoPrefs.PartPrefs.m_TotalSourceRestore, Category && Category->PartPrefs ? Category->PartPrefs->m_TotalSourceRestore : -1);;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_ReaskPropability.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_ReaskPropability : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_ReaskPropability : FCFG_DEF);
	m_UnpredictedPropability.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_UnpredictedPropability : FCFG_DEF, NeoPrefs.PartPrefs.m_UnpredictedPropability, Category && Category->PartPrefs ? Category->PartPrefs->m_UnpredictedPropability : -1);;
	m_UnpredictedReaskPropability.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_UnpredictedReaskPropability : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_UnpredictedReaskPropability : FCFG_DEF);
 #endif // NEO_SA // NEO: NSA END

	m_AutoReaskStoredSourcesDelay.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_AutoReaskStoredSourcesDelay : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoReaskStoredSourcesDelay : FCFG_DEF);

	m_GroupStoredSourceReask.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_GroupStoredSourceReask : FCFG_DEF, NeoPrefs.PartPrefs.m_GroupStoredSourceReask, Category && Category->PartPrefs ? Category->PartPrefs->m_GroupStoredSourceReask : -1);;
	m_StoredSourceGroupIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_StoredSourceGroupIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_StoredSourceGroupIntervals : FCFG_DEF);
	m_StoredSourceGroupSize.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_StoredSourceGroupSize : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_StoredSourceGroupSize : FCFG_DEF);
#endif // NEO_SS // NEO: NSS END

	for (int i = 1; i < m_paFiles->GetSize(); i++)
	{
		if(!theApp.downloadqueue->IsPartFile((CPartFile*)(*m_paFiles)[i]))
			continue;

		PartFile = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[i]);

		if(Category && Category != thePrefs.GetCategory(PartFile->GetCategory()))
		{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			m_AutoSaveSources.Cat = -2;
			m_AutoSaveSourcesIntervals.Def = FCFG_UNK;
			m_SourceStorageLimit.Def = FCFG_UNK;

			m_StoreAlsoA4AFSources.Cat = -2;

			m_AutoLoadSources.Cat = -2;
			m_LoadedSourceCleanUpTime.Def = FCFG_UNK;
			m_SourceStorageReaskLimit.Def = FCFG_UNK;

			m_TotalSourceRestore.Cat = -2;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			m_ReaskPropability.Def = FCFG_UNK;
			m_UnpredictedPropability.Cat = -2;
			m_UnpredictedReaskPropability.Def = FCFG_UNK;
 #endif // NEO_SA // NEO: NSA END

			m_AutoReaskStoredSourcesDelay.Def = FCFG_UNK;

			m_GroupStoredSourceReask.Cat = -2;
			m_StoredSourceGroupIntervals.Def = FCFG_UNK;
			m_StoredSourceGroupSize.Def = FCFG_UNK;
#endif // NEO_SS // NEO: NSS END

			Category = NULL;
		}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		if(m_AutoSaveSources.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoSaveSources : FCFG_DEF)) m_AutoSaveSources.Val = FCFG_UNK;
		if(m_AutoSaveSourcesIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoSaveSourcesIntervals : FCFG_DEF)) m_AutoSaveSourcesIntervals.Val = FCFG_UNK;
		if(m_SourceStorageLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SourceStorageLimit : FCFG_DEF)) m_SourceStorageLimit.Val = FCFG_UNK;

		if(m_StoreAlsoA4AFSources.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_StoreAlsoA4AFSources : FCFG_DEF)) m_StoreAlsoA4AFSources.Val = FCFG_UNK;

		if(m_AutoLoadSources.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoLoadSources : FCFG_DEF)) m_AutoLoadSources.Val = FCFG_UNK;
		if(m_LoadedSourceCleanUpTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_LoadedSourceCleanUpTime : FCFG_DEF)) m_LoadedSourceCleanUpTime.Val = FCFG_UNK;
		if(m_SourceStorageReaskLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SourceStorageReaskLimit : FCFG_DEF)) m_SourceStorageReaskLimit.Val = FCFG_UNK;

		if(m_TotalSourceRestore.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_TotalSourceRestore : FCFG_DEF)) m_TotalSourceRestore.Val = FCFG_UNK;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		if(m_ReaskPropability.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_ReaskPropability : FCFG_DEF)) m_ReaskPropability.Val = FCFG_UNK;
		if(m_UnpredictedPropability.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_UnpredictedPropability : FCFG_DEF)) m_UnpredictedPropability.Val = FCFG_UNK;
		if(m_UnpredictedReaskPropability.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_UnpredictedReaskPropability : FCFG_DEF)) m_UnpredictedReaskPropability.Val = FCFG_UNK;
 #endif // NEO_SA // NEO: NSA END

		if(m_AutoReaskStoredSourcesDelay.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoReaskStoredSourcesDelay : FCFG_DEF)) m_AutoReaskStoredSourcesDelay.Val = FCFG_UNK;

		if(m_GroupStoredSourceReask.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_GroupStoredSourceReask : FCFG_DEF)) m_GroupStoredSourceReask.Val = FCFG_UNK;
		if(m_StoredSourceGroupIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_StoredSourceGroupIntervals : FCFG_DEF)) m_StoredSourceGroupIntervals.Val = FCFG_UNK;
		if(m_StoredSourceGroupSize.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_StoredSourceGroupSize : FCFG_DEF)) m_StoredSourceGroupSize.Val = FCFG_UNK;
#endif // NEO_SS // NEO: NSS END
	}

	SetAuxSet(false);
	SetAut();

	UpdateData(FALSE);
}

void CPPgSourceStorage::GetFilePreferences(CKnownPreferences* /*KnownPrefs*/, CPartPreferences* PartPrefs, bool OperateData)
{
	if(OperateData)
		UpdateData();
	if(PartPrefs)
	{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		if(m_AutoSaveSources.Val != FCFG_UNK) PartPrefs->m_AutoSaveSources = m_AutoSaveSources.Val;
		if(m_AutoSaveSourcesIntervals.Val != FCFG_UNK) PartPrefs->m_AutoSaveSourcesIntervals = m_AutoSaveSourcesIntervals.Val;
		if(m_SourceStorageLimit.Val != FCFG_UNK) PartPrefs->m_SourceStorageLimit = m_SourceStorageLimit.Val;

		if(m_StoreAlsoA4AFSources.Val != FCFG_UNK) PartPrefs->m_StoreAlsoA4AFSources = m_StoreAlsoA4AFSources.Val;

		if(m_AutoLoadSources.Val != FCFG_UNK) PartPrefs->m_AutoLoadSources = m_AutoLoadSources.Val;
		if(m_LoadedSourceCleanUpTime.Val != FCFG_UNK) PartPrefs->m_LoadedSourceCleanUpTime = m_LoadedSourceCleanUpTime.Val;
		if(m_SourceStorageReaskLimit.Val != FCFG_UNK) PartPrefs->m_SourceStorageReaskLimit = m_SourceStorageReaskLimit.Val;

		if(m_TotalSourceRestore.Val != FCFG_UNK) PartPrefs->m_TotalSourceRestore = m_TotalSourceRestore.Val;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		if(m_ReaskPropability.Val != FCFG_UNK) PartPrefs->m_ReaskPropability = m_ReaskPropability.Val;
		if(m_UnpredictedPropability.Val != FCFG_UNK) PartPrefs->m_UnpredictedPropability = m_UnpredictedPropability.Val;
		if(m_UnpredictedReaskPropability.Val != FCFG_UNK) PartPrefs->m_UnpredictedReaskPropability = m_UnpredictedReaskPropability.Val;
 #endif // NEO_SA // NEO: NSA END

		if(m_AutoReaskStoredSourcesDelay.Val != FCFG_UNK) PartPrefs->m_AutoReaskStoredSourcesDelay = m_AutoReaskStoredSourcesDelay.Val;

		if(m_GroupStoredSourceReask.Val != FCFG_UNK) PartPrefs->m_GroupStoredSourceReask = m_GroupStoredSourceReask.Val;
		if(m_StoredSourceGroupIntervals.Val != FCFG_UNK) PartPrefs->m_StoredSourceGroupIntervals = m_StoredSourceGroupIntervals.Val;
		if(m_StoredSourceGroupSize.Val != FCFG_UNK) PartPrefs->m_StoredSourceGroupSize = m_StoredSourceGroupSize.Val;
#endif // NEO_SS // NEO: NSS END
	}
}

void CPPgSourceStorage::SetFilePreferences(CKnownPreferences* /*KnownPrefs*/, CPartPreferences* PartPrefs, bool OperateData)
{
	if(PartPrefs)
	{
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		m_AutoSaveSources.Val = PartPrefs->m_AutoSaveSources;
		m_AutoSaveSourcesIntervals.Val = PartPrefs->m_AutoSaveSourcesIntervals;
		m_SourceStorageLimit.Val = PartPrefs->m_SourceStorageLimit;

		m_StoreAlsoA4AFSources.Val = PartPrefs->m_StoreAlsoA4AFSources;

		m_AutoLoadSources.Val = PartPrefs->m_AutoLoadSources;
		m_LoadedSourceCleanUpTime.Val = PartPrefs->m_LoadedSourceCleanUpTime;
		m_SourceStorageReaskLimit.Val = PartPrefs->m_SourceStorageReaskLimit;

		m_TotalSourceRestore.Val = PartPrefs->m_TotalSourceRestore;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		m_ReaskPropability.Val = PartPrefs->m_ReaskPropability;
		m_UnpredictedPropability.Val = PartPrefs->m_UnpredictedPropability;
		m_UnpredictedReaskPropability.Val = PartPrefs->m_UnpredictedReaskPropability;
 #endif // NEO_SA // NEO: NSA END

		m_AutoReaskStoredSourcesDelay.Val = PartPrefs->m_AutoReaskStoredSourcesDelay;

		m_GroupStoredSourceReask.Val = PartPrefs->m_GroupStoredSourceReask;
		m_StoredSourceGroupIntervals.Val = PartPrefs->m_StoredSourceGroupIntervals;
		m_StoredSourceGroupSize.Val = PartPrefs->m_StoredSourceGroupSize;
#endif // NEO_SS // NEO: NSS END
	}
	if(OperateData)
	{
		UpdateData(FALSE);
		SetModified();
	}
}
// NEO: FCFG END

BOOL CPPgSourceStorage::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	/*
	* Einstellungen Speichern
	*/

	// NEO: FCFG - [FileConfiguration]
	if(m_paFiles)
	{
		CPartFile* PartFile;
		CPartPreferences* PartPrefs;
		for (int i = 0; i < m_paFiles->GetSize(); i++)
		{
			// check if the file is still valid
			if(!theApp.downloadqueue->IsPartFile((CPartFile*)(*m_paFiles)[i]))
				continue;

			PartFile = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[i]);

			// get ot create Preferences 
			PartPrefs = PartFile->PartPrefs;
			if(!PartPrefs->IsFilePrefs())
				PartPrefs = new CPartPreferencesEx(CFP_FILE);

			// save preferences
			GetFilePreferences(NULL, PartPrefs);

			// check validiti of the tweaks
			PartPrefs->CheckTweaks();

			// valiate pointers and update (!)
			PartFile->UpdatePartPrefs(PartPrefs);
		}

		RefreshData();
	}
	else if(m_Category)
	{
		int cat = thePrefs.FindCategory(m_Category);
		if(cat != -1)
		{
			// get ot create Preferences 
			CPartPreferences* PartPrefs = m_Category->PartPrefs;
			if(PartPrefs == NULL)
				PartPrefs = new CPartPreferencesEx(CFP_CATEGORY);

			// save preferences
			GetFilePreferences(NULL, PartPrefs);

			// check validiti of the tweaks
			PartPrefs->CheckTweaks();

			// valiate pointers and update (!)
			theApp.downloadqueue->UpdatePartPrefs(PartPrefs, NULL, (UINT)cat); // may delete PartPrefs

			thePrefs.SaveCats();

			LoadSettings();
		}
		else
			AfxMessageBox(GetResString(IDS_X_INVALID_CAT),MB_OK | MB_ICONSTOP, NULL);
	}
	else
	// NEO: FCFG END
	{
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
		NeoPrefs.m_bEnableSourceList = m_bEnableSourceList;
		// cleanup
		NeoPrefs.m_uSourceListRunTimeCleanUp = m_uSourceListRunTimeCleanUp;
		// NEO: SFL - [SourceFileList]
		// seen files
		NeoPrefs.m_bSaveSourceFileList = m_bSaveSourceFileList;
		// NEO: SFL END
		// security
		NeoPrefs.m_bUseIPZoneCheck = m_bUseIPZoneCheck;
		NeoPrefs.m_bSourceHashMonitor = m_bSourceHashMonitor; // NEO: SHM - [SourceHashMonitor]
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		NeoPrefs.m_bEnableSourceAnalizer = m_bEnableSourceAnalizer;
		// gap handling
		NeoPrefs.m_bHandleTableGaps = m_bHandleTableGaps;
		// analyze obtions
		NeoPrefs.m_bDualLinkedTableGravity = m_bDualLinkedTableGravity;
		// additional obtions
		NeoPrefs.m_bLinkTimePropability = m_bLinkTimePropability;
		NeoPrefs.m_bScaleReliableTime = m_bScaleReliableTime;
#endif // NEO_SA // NEO: NSA END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
		if(m_iTableAmountToStore.Val != FCFG_UNK) NeoPrefs.m_iTableAmountToStore = m_iTableAmountToStore.Val;
		// cleanup
		if(m_iSourceListExpirationTime.Val != FCFG_UNK) NeoPrefs.m_iSourceListExpirationTime = m_iSourceListExpirationTime.Val;
		// NEO: SFL - [SourceFileList]
		// seen files
		if(m_iFileListExpirationTime.Val != FCFG_UNK) NeoPrefs.m_iFileListExpirationTime = m_iFileListExpirationTime.Val;
		// NEO: SFL END
		// aditional obtions
		if(m_iIgnoreUndefinedIntervall.Val != FCFG_UNK) NeoPrefs.m_iIgnoreUndefinedIntervall = m_iIgnoreUndefinedIntervall.Val;
		if(m_iIgnoreUnreachableInterval.Val != FCFG_UNK) NeoPrefs.m_iIgnoreUnreachableInterval = m_iIgnoreUnreachableInterval.Val;
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
		if(m_iAnalyzeIntervals.Val != FCFG_UNK) NeoPrefs.m_iAnalyzeIntervals = m_iAnalyzeIntervals.Val;
		if(m_iTableAmountToAnalyze.Val != FCFG_UNK) NeoPrefs.m_iTableAmountToAnalyze = m_iTableAmountToAnalyze.Val;
		// gap handling
		if(m_fPriorityGapRatio.Val != FCFG_UNK) NeoPrefs.m_fPriorityGapRatio = m_fPriorityGapRatio.Val;
		if(m_iMaxGapSize.Val != FCFG_UNK) NeoPrefs.m_iMaxGapSize = m_iMaxGapSize.Val;
		if(m_iMaxGapTime.Val != FCFG_UNK) NeoPrefs.m_iMaxGapTime = m_iMaxGapTime.Val;
		// analyze obtions
		if(m_fMaxMidleDiscrepanceHigh.Val != FCFG_UNK) NeoPrefs.m_fMaxMidleDiscrepanceHigh = m_fMaxMidleDiscrepanceHigh.Val;
		if(m_fMaxMidleDiscrepanceLow.Val != FCFG_UNK) NeoPrefs.m_fMaxMidleDiscrepanceLow = m_fMaxMidleDiscrepanceLow.Val;
		if(m_iDualLinkedTableGravity.Val != FCFG_UNK) NeoPrefs.m_iDualLinkedTableGravity = m_iDualLinkedTableGravity.Val;
		// calculation obtions
		if(m_fEnhancedFactor.Val != FCFG_UNK) NeoPrefs.m_fEnhancedFactor = m_fEnhancedFactor.Val;
		if(m_iFreshSourceTreshold.Val != FCFG_UNK) NeoPrefs.m_iFreshSourceTreshold = m_iFreshSourceTreshold.Val;
		if(m_iTempralIPBorderLine.Val != FCFG_UNK) NeoPrefs.m_iTempralIPBorderLine = m_iTempralIPBorderLine.Val;
		// additional obtions
		if(m_fLastSeenDurationThreshold.Val != FCFG_UNK) NeoPrefs.m_fLastSeenDurationThreshold = m_fLastSeenDurationThreshold.Val;
		if(m_iLinkTimeThreshold.Val != FCFG_UNK) NeoPrefs.m_iLinkTimeThreshold = m_iLinkTimeThreshold.Val;
		if(m_iMaxReliableTime.Val != FCFG_UNK) NeoPrefs.m_iMaxReliableTime = m_iMaxReliableTime.Val;
#endif // NEO_SA // NEO: NSA END

		GetFilePreferences(NULL, &NeoPrefs.PartPrefs);

		NeoPrefs.PartPrefs.CheckTweaks();

		NeoPrefs.CheckNeoPreferences();
		LoadSettings();
	}

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

// NEO: FCFG - [FileConfiguration]
void CPPgSourceStorage::OnTimer(UINT /*nIDEvent*/)
{
	if (m_bDataChanged)
	{
		if(m_paFiles)
			RefreshData();
		m_bDataChanged = false;
	}
}

BOOL CPPgSourceStorage::OnSetActive()
{
	if (!CPropertyPage::OnSetActive())
		return FALSE;
	if (m_bDataChanged)
	{
		if(m_paFiles)
			RefreshData();
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CPPgSourceStorage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}
// NEO: FCFG END

BOOL CPPgSourceStorage::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgSourceStorage::OnDestroy()
{
	// NEO: FCFG - [FileConfiguration]
	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}
	// NEO: FCFG END

	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

CString FormatNumber(SintD& nValue, int value, CString Addon = _T(""));

LRESULT CPPgSourceStorage::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			if(m_htiAutoSaveSourcesIntervals && pton->hItem == m_htiAutoSaveSourcesIntervals){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoSaveSourcesIntervals, m_AutoSaveSourcesIntervals)) SetModified();
			}else
			if(m_htiSourceStorageLimit && pton->hItem == m_htiSourceStorageLimit){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceStorageLimit, m_SourceStorageLimit)) SetModified();
			}else

			if(m_htiLoadedSourceCleanUpTime && pton->hItem == m_htiLoadedSourceCleanUpTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLoadedSourceCleanUpTime, m_LoadedSourceCleanUpTime)) SetModified();
			}else
			if(m_htiSourceStorageReaskLimit && pton->hItem == m_htiSourceStorageReaskLimit){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceStorageReaskLimit, m_SourceStorageReaskLimit)) SetModified();
			}else

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			if(m_htiReaskPropability && pton->hItem == m_htiReaskPropability){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReaskPropability, m_ReaskPropability)) SetModified();
			}else
			if(m_htiUnpredictedReaskPropability && pton->hItem == m_htiUnpredictedReaskPropability){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUnpredictedReaskPropability, m_UnpredictedReaskPropability)) SetModified();
			}else
#endif // NEO_SA // NEO: NSA END

			if(m_htiAutoReaskStoredSourcesDelay && pton->hItem == m_htiAutoReaskStoredSourcesDelay){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoReaskStoredSourcesDelay, m_AutoReaskStoredSourcesDelay)) SetModified();
			}else

			if(m_htiStoredSourceGroupIntervals && pton->hItem == m_htiStoredSourceGroupIntervals){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiStoredSourceGroupIntervals, m_StoredSourceGroupIntervals)) SetModified();
			}else
			if(m_htiStoredSourceGroupSize && pton->hItem == m_htiStoredSourceGroupSize){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiStoredSourceGroupSize, m_StoredSourceGroupSize)) SetModified();
			}else
#endif // NEO_SS // NEO: NSS END

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
			if(m_htiSourceListExpirationTime && pton->hItem == m_htiSourceListExpirationTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceListExpirationTime, m_iSourceListExpirationTime)) SetModified();
			}else
			if(m_htiFileListExpirationTime && pton->hItem == m_htiFileListExpirationTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFileListExpirationTime, m_iFileListExpirationTime)) SetModified();
			}else
			if(m_htiTableAmountToStore && pton->hItem == m_htiTableAmountToStore){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiTableAmountToStore, m_iTableAmountToStore)) SetModified();
			}else
			if(m_htiIgnoreUndefinedIntervall && pton->hItem == m_htiIgnoreUndefinedIntervall){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiIgnoreUndefinedIntervall, m_iIgnoreUndefinedIntervall)) SetModified();
			}else
			if(m_htiIgnoreUnreachableInterval && pton->hItem == m_htiIgnoreUnreachableInterval){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiIgnoreUnreachableInterval, m_iIgnoreUnreachableInterval)) SetModified();
			}else
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			if(m_htiAnalyzeIntervals && pton->hItem == m_htiAnalyzeIntervals){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAnalyzeIntervals, m_iAnalyzeIntervals)) SetModified();
			}else
			if(m_htiTableAmountToAnalyze && pton->hItem == m_htiTableAmountToAnalyze){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiTableAmountToAnalyze, m_iTableAmountToAnalyze)) SetModified();
			}else
			if(m_htiPriorityGapRatio && pton->hItem == m_htiPriorityGapRatio){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiPriorityGapRatio, m_fPriorityGapRatio)) SetModified();
			}else
			if(m_htiMaxGapSize && pton->hItem == m_htiMaxGapSize){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxGapSize, m_iMaxGapSize)) SetModified();
			}else
			if(m_htiMaxGapTime && pton->hItem == m_htiMaxGapTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxGapTime, m_iMaxGapTime)) SetModified();
			}else
			if(m_htiMaxMidleDiscrepanceHigh && pton->hItem == m_htiMaxMidleDiscrepanceHigh){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxMidleDiscrepanceHigh, m_fMaxMidleDiscrepanceHigh)) SetModified();
			}else
			if(m_htiMaxMidleDiscrepanceLow && pton->hItem == m_htiMaxMidleDiscrepanceLow){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxMidleDiscrepanceLow, m_fMaxMidleDiscrepanceLow)) SetModified();
			}else
			if(m_htiDualLinkedTableGravity && pton->hItem == m_htiDualLinkedTableGravity){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDualLinkedTableGravity, m_iDualLinkedTableGravity)) SetModified();
			}else
			if(m_htiEnhancedFactor && pton->hItem == m_htiEnhancedFactor){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiEnhancedFactor, m_fEnhancedFactor)) SetModified();
			}else
			if(m_htiFreshSourceTreshold && pton->hItem == m_htiFreshSourceTreshold){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFreshSourceTreshold, m_iFreshSourceTreshold)) SetModified();
			}else
			if(m_htiTempralIPBorderLine && pton->hItem == m_htiTempralIPBorderLine){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiTempralIPBorderLine, m_iTempralIPBorderLine)) SetModified();
			}else
			if(m_htiLastSeenDurationThreshold && pton->hItem == m_htiLastSeenDurationThreshold){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLastSeenDurationThreshold, m_fLastSeenDurationThreshold)) SetModified();
			}else
			if(m_htiLinkTimeThreshold && pton->hItem == m_htiLinkTimeThreshold){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLinkTimeThreshold, m_iLinkTimeThreshold)) SetModified();
			}else
			if(m_htiMaxReliableTime && pton->hItem == m_htiMaxReliableTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxReliableTime, m_iMaxReliableTime)) SetModified();
			}
#endif // NEO_SA // NEO: NSA END
		}else{
			UINT bCheck;
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
			if(m_htiEnableSourceList && pton->hItem == m_htiEnableSourceList){
				m_ctrlTreeOptions.GetCheckBox(m_htiEnableSourceList, bCheck);
					m_ctrlTreeOptions.SetItemEnable(m_htiSourceListExpirationTime, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiSaveSourceFileList, bCheck);
				//m_ctrlTreeOptions.SetItemEnable(m_htiIpSecurity, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiIpTables, bCheck);
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
				m_ctrlTreeOptions.SetGroupEnable(m_htiSourceAnalyzer,bCheck);
 #endif // NEO_SA // NEO: NSA END
 #ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				m_SourceStorageReaskLimit.aS(bCheck);
				m_ctrlTreeOptions.SetEditText(m_htiSourceStorageReaskLimit,FormatNumber(m_SourceStorageReaskLimit,m_SourceStorageReaskLimit.Std[m_SourceStorageReaskLimit.SubSet()],m_SourceStorageReaskLimit.Base ? _T("") : _T(" (std)")));
 #endif // NEO_SS // NEO: NSS END
			}else
			// NEO: SFL - [SourceFileList]
			if(m_htiSaveSourceFileList && pton->hItem == m_htiSaveSourceFileList){
				m_ctrlTreeOptions.GetCheckBox(m_htiSaveSourceFileList, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiFileListExpirationTime, bCheck);
			}
			// NEO: SFL END
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			if(m_htiEnableSourceAnalizer && pton->hItem == m_htiEnableSourceAnalizer){
				m_ctrlTreeOptions.GetCheckBox(m_htiEnableSourceAnalizer, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiAnalyzeIntervals, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiTableAmountToAnalyze, bCheck);

				m_ctrlTreeOptions.SetItemEnable(m_htiHandleTableGaps, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiAdvanced, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiCalculation, bCheck);

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
				m_SourceStorageReaskLimit.aS(bCheck);
				m_ctrlTreeOptions.SetEditText(m_htiSourceStorageReaskLimit,FormatNumber(m_SourceStorageReaskLimit,m_SourceStorageReaskLimit.Std[m_SourceStorageReaskLimit.SubSet()],m_SourceStorageReaskLimit.Base ? _T("") : _T(" (std)")));
 #endif // NEO_SS // NEO: NSS END
			}	
			if(m_htiHandleTableGaps && pton->hItem == m_htiHandleTableGaps){
				m_ctrlTreeOptions.GetCheckBox(m_htiHandleTableGaps, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiPriorityGapRatio, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiMaxGapSize, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiMaxGapTime, bCheck);
			}
			if(m_htiDualLinkedTableGravity && pton->hItem == m_htiDualLinkedTableGravity){
				m_ctrlTreeOptions.GetCheckBox(m_htiDualLinkedTableGravity, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiDualLinkedTableGravity, bCheck,FALSE,TRUE);
			}
			if(m_htiLinkTimeThreshold && pton->hItem == m_htiLinkTimeThreshold){
				m_ctrlTreeOptions.GetCheckBox(m_htiLinkTimeThreshold, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiLinkTimeThreshold, bCheck,FALSE,TRUE);
			}
			if(m_htiMaxReliableTime && pton->hItem == m_htiMaxReliableTime){
				m_ctrlTreeOptions.GetCheckBox(m_htiMaxReliableTime, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiMaxReliableTime, bCheck,FALSE,TRUE);
			}
#endif // NEO_SA // NEO: NSA END
			SetModified();
		}
	}
	return 0;
}


LRESULT CPPgSourceStorage::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgSourceStorage::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgSourceStorage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgSourceStorage::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
