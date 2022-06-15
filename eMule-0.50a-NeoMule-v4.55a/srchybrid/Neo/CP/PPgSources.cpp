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
#include "PPgSources.h"
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
#include "Scheduler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CPPgSources dialog

IMPLEMENT_DYNAMIC(CPPgSources, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgSources, CPropertyPage)
	ON_WM_TIMER() // NEO: FCFG - [FileConfiguration]
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged) // NEO: FCFG - [FileConfiguration]
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgSources::CPPgSources()
	: CPropertyPage(CPPgSources::IDD)
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

CPPgSources::~CPPgSources()
{
}

void CPPgSources::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
	// NEO: SRT - [SourceRequestTweaks]
	m_htiSourceRequest = NULL;
		// General
		m_htiSourceLimit = NULL;

		// Management
		m_htiSourceManagement = NULL;
			m_htiSwapLimit = NULL;
			// NEO: NXC - [NewExtendedCategories]
			m_htiAdvancedA4AFMode = NULL;
				m_htiAdvancedA4AFModeDisabled = NULL;
				m_htiAdvancedA4AFModeBalance = NULL;
				m_htiAdvancedA4AFModeStack = NULL;
				m_htiSmartA4AFSwapping = NULL;
			m_htiA4AFFlags = NULL;
				m_htiA4AFFlagsNone = NULL;
				m_htiA4AFFlagsOn = NULL;
				m_htiA4AFFlagsOff = NULL;
			// NEO: NXC END
			// NEO: XSC - [ExtremeSourceCache]
			m_htiSourceCache = NULL;
				m_htiSourceCacheDisable = NULL;
				m_htiSourceCacheEnable = NULL;
				m_htiSourceCacheDefault = NULL;
				m_htiSourceCacheGlobal = NULL;
				m_htiSourceCacheLimit = NULL;
				m_htiSourceCacheTime = NULL;
			// NEO: XSC END
			// NEO: ASL - [AutoSoftLock]
			m_htiAutoSoftLock = NULL;
				m_htiAutoSoftLockDisable = NULL;
				m_htiAutoSoftLockEnable = NULL;
				m_htiAutoSoftLockDefault = NULL;
				m_htiAutoSoftLockGlobal = NULL;
				m_htiAutoSoftLockLimit = NULL;
			// NEO: ASL END
			// NEO: AHL - [AutoHardLimit]
			m_htiAutoHardLimit = NULL;
				m_htiAutoHardLimitDisable = NULL;
				m_htiAutoHardLimitEnable = NULL;
				m_htiAutoHardLimitDefault = NULL;
				m_htiAutoHardLimitGlobal = NULL;
				m_htiAutoHardLimitTime = NULL;
			// NEO: AHL END
			// NEO: CSL - [CategorySourceLimit]
			m_htiCategorySourceLimit = NULL;
				m_htiCategorySourceLimitDisable = NULL;
				m_htiCategorySourceLimitEnable = NULL;
				m_htiCategorySourceLimitDefault = NULL;
				m_htiCategorySourceLimitGlobal = NULL;
				m_htiCategorySourceLimitLimit = NULL;
				m_htiCategorySourceLimitTime = NULL;
			// NEO: CSL END
			// NEO: GSL - [GlobalSourceLimit]
			m_htiGlobalSourceLimit = NULL;
				m_htiGlobalSourceLimitDisable = NULL;
				m_htiGlobalSourceLimitEnable = NULL;
				m_htiGlobalSourceLimitDefault = NULL;
				m_htiGlobalSourceLimitGlobal = NULL;
				m_htiGlobalSourceLimitLimit = NULL;
				m_htiGlobalSourceLimitTime = NULL;
			// NEO: GSL END
			m_htiMinSourcePerFile = NULL;

		//XS
		m_htiXs = NULL;
			m_htiXsDisable = NULL;
			m_htiXsEnable = NULL;
			m_htiXsDefault = NULL;
			m_htiXsGlobal = NULL;
				
			m_htiXsLimit = NULL;
			m_htiXsIntervals = NULL;
			m_htiXsClientIntervals = NULL;
			m_htiXsCleintDelay = NULL;
			m_htiXsRareLimit = NULL;

	// SVR
		m_htiSvr = NULL;
			m_htiSvrDisable = NULL;
			m_htiSvrEnable = NULL;
			m_htiSvrDefault = NULL;
			m_htiSvrGlobal = NULL;

			m_htiSvrLimit = NULL;
			m_htiSvrIntervals = NULL;

	//KAD
		m_htiKad = NULL;
			m_htiKadDisable = NULL;
			m_htiKadEnable = NULL;
			m_htiKadDefault = NULL;
			m_htiKadGlobal = NULL;

			m_htiKadLimit = NULL;
			m_htiKadIntervals = NULL;
			m_htiKadMaxFiles = NULL;
			m_htiKadRepeatDelay = NULL;

	//UDP
		m_htiUdp = NULL;
			m_htiUdpDisable = NULL;
			m_htiUdpEnable = NULL;
			m_htiUdpDefault = NULL;
			m_htiUdpGlobal = NULL;

			m_htiUdpLimit = NULL;
			m_htiUdpIntervals = NULL;
			m_htiUdpGlobalIntervals = NULL;
			m_htiUdpFilesPerServer = NULL;
	// NEO: SRT END

		m_htiTCPConnectionRetry = NULL; // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	m_htiDownloadReask = NULL;
		m_htiSpreadReask = NULL;
		m_htiSourceReaskTime = NULL;
		m_htiFullQSourceReaskTime = NULL;
		m_htiNNPSourceReaskTime = NULL;
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	m_htiSourcesDrop = NULL;
		m_htiDropTime = NULL;

	//Bad
		m_htiBad = NULL;
			m_htiBadSourceDontDrop = NULL;
			m_htiBadSourceDrop = NULL;
			m_htiBadSourceTrySwap = NULL;
			m_htiBadSourceDefault = NULL;
			m_htiBadSourceGlobal = NULL;

			m_htiBadSourceLimit = NULL;
				m_htiBadSourceLimitTotal = NULL;
				m_htiBadSourceLimitRelativ = NULL;
				m_htiBadSourceLimitSpecyfic = NULL;
				m_htiBadSourceLimitDefault = NULL;
				m_htiBadSourceLimitGlobal = NULL;
			m_htiBadSourceDropTime = NULL;
				m_htiBadSourceDropProgressiv = NULL;
				m_htiBadSourceDropDistributiv = NULL;
				m_htiBadSourceDropCummulativ = NULL;
				m_htiBadSourceDropDefault = NULL;
				m_htiBadSourceDropGlobal = NULL;

	//NNP
		m_htiNNP = NULL;
			m_htiNNPSourceDontDrop = NULL;
			m_htiNNPSourceDrop = NULL;
			m_htiNNPSourceTrySwap = NULL;
			m_htiNNPSourceDefault = NULL;
			m_htiNNPSourceGlobal = NULL;

			m_htiNNPSourceLimit = NULL;
				m_htiNNPSourceLimitTotal = NULL;
				m_htiNNPSourceLimitRelativ = NULL;
				m_htiNNPSourceLimitSpecyfic = NULL;
				m_htiNNPSourceLimitDefault = NULL;
				m_htiNNPSourceLimitGlobal = NULL;
			m_htiNNPSourceDropTime = NULL;
				m_htiNNPSourceDropProgressiv = NULL;
				m_htiNNPSourceDropDistributiv = NULL;
				m_htiNNPSourceDropCummulativ = NULL;
				m_htiNNPSourceDropDefault = NULL;
				m_htiNNPSourceDropGlobal = NULL;
	//FullQ
		m_htiFullQ = NULL;
			m_htiFullQSourceDontDrop = NULL;
			m_htiFullQSourceDrop = NULL;
			m_htiFullQSourceTrySwap = NULL;
			m_htiFullQSourceDefault = NULL;
			m_htiFullQSourceGlobal = NULL;

			m_htiFullQSourceLimit = NULL;
				m_htiFullQSourceLimitTotal = NULL;
				m_htiFullQSourceLimitRelativ = NULL;
				m_htiFullQSourceLimitSpecyfic = NULL;
				m_htiFullQSourceLimitDefault = NULL;
				m_htiFullQSourceLimitGlobal = NULL;
			m_htiFullQSourceDropTime = NULL;
				m_htiFullQSourceDropProgressiv = NULL;
				m_htiFullQSourceDropDistributiv = NULL;
				m_htiFullQSourceDropCummulativ = NULL;
				m_htiFullQSourceDropDefault = NULL;
				m_htiFullQSourceDropGlobal = NULL;

	//HighQ
		m_htiHighQ = NULL;
			m_htiHighQSourceDontDrop = NULL;
			m_htiHighQSourceDrop = NULL;
			m_htiHighQSourceTrySwap = NULL;
			m_htiHighQSourceDefault = NULL;
			m_htiHighQSourceGlobal = NULL;

			m_htiHighQSourceLimit = NULL;
				m_htiHighQSourceLimitTotal = NULL;
				m_htiHighQSourceLimitRelativ = NULL;
				m_htiHighQSourceLimitSpecyfic = NULL;
				m_htiHighQSourceLimitDefault = NULL;
				m_htiHighQSourceLimitGlobal = NULL;
			m_htiHighQSourceDropTime = NULL;
				m_htiHighQSourceDropProgressiv = NULL;
				m_htiHighQSourceDropDistributiv = NULL;
				m_htiHighQSourceDropCummulativ = NULL;
				m_htiHighQSourceDropDefault = NULL;
				m_htiHighQSourceDropGlobal = NULL;
			m_htiHighQSourceMaxRank = NULL;
				m_htiHighQSourceRankNormal = NULL;
				m_htiHighQSourceRankAverage = NULL;
				m_htiHighQSourceRankDefault = NULL;
				m_htiHighQSourceRankGlobal = NULL;

		m_htiDeadTime = NULL;
			m_htiDeadTimeFWMulti = NULL;
		m_htiGlobalDeadTime = NULL;
			m_htiGlobalDeadTimeFWMulti = NULL;
	// NEO: SDT END
}

// NEO: SRT - [SourceRequestTweaks]
void CPPgSources::SetTreeRadioForSRT(HTREEITEM &htiDisable, HTREEITEM &htiEnable, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),Value,0),htiParent,GetResString(IDS_X_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable,MkRdBtnLbl(GetResString(IDS_X_ENABLE),Value,1),htiParent,GetResString(IDS_X_ENABLE_INFO),TRUE,Value.Val == 1);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

void CPPgSources::SetTreeNumEditForSRT(HTREEITEM &htiLimit, HTREEITEM &htiIntervals, HTREEITEM &htiParent)
{
	SetTreeNumEdit(m_ctrlTreeOptions,htiLimit,GetResString(IDS_X_REQUEST_LIMIT), htiParent,GetResString(IDS_X_REQUEST_LIMIT_INFO));
	SetTreeNumEdit(m_ctrlTreeOptions,htiIntervals,GetResString(IDS_X_REQUEST_INTERVALS), htiParent,GetResString(IDS_X_REQUEST_INTERVALS_INFO));
}
// NEO: SRT END

// NEO: SDT - [SourcesDropTweaks]
void CPPgSources::SetTreeRadioForSDT(HTREEITEM &htiDontDrop, HTREEITEM &htiDrop, HTREEITEM &htiTrySwap, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value, bool bNoSwap)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDontDrop,MkRdBtnLbl(GetResString(IDS_X_DONT_DROP),Value,0),htiParent,GetResString(IDS_X_DONT_DROP_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiDrop,MkRdBtnLbl(GetResString(IDS_X_DO_DROP),Value,1),htiParent,GetResString(IDS_X_DO_DROP_INFO),TRUE,Value.Val == 1);
	if(!bNoSwap)
		SetTreeRadio(m_ctrlTreeOptions,htiTrySwap,MkRdBtnLbl(GetResString(IDS_X_TRY_SWAP),Value,2),htiParent,GetResString(IDS_X_TRY_SWAP_INFO),TRUE,Value.Val == 2);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

void CPPgSources::SetTreeRadioNumEditForSDT1(HTREEITEM &htiLimit,HTREEITEM &htiLimitTotal,HTREEITEM &htiLimitRelativ,HTREEITEM &htiLimitSpecyfic,HTREEITEM &htiLimitDefault,HTREEITEM &htiLimitGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeNumEdit(m_ctrlTreeOptions,htiLimit,GetResString(IDS_X_DROP_LIMIT), htiParent,GetResString(IDS_X_DROP_LIMIT_INFO));
		SetTreeRadio(m_ctrlTreeOptions,htiLimitTotal,MkRdBtnLbl(GetResString(IDS_X_DROP_LIMIT_TOTAL),Value,0),htiLimit,GetResString(IDS_X_DROP_LIMIT_TOTAL_INFO),TRUE,Value.Val == 0);
		SetTreeRadio(m_ctrlTreeOptions,htiLimitRelativ,MkRdBtnLbl(GetResString(IDS_X_DROP_LIMIT_RELATIV),Value,1),htiLimit,GetResString(IDS_X_DROP_LIMIT_RELATIV_INFO),TRUE,Value.Val == 1);
		SetTreeRadio(m_ctrlTreeOptions,htiLimitSpecyfic,MkRdBtnLbl(GetResString(IDS_X_DROP_LIMIT_SPECIFIC),Value,2),htiLimit,GetResString(IDS_X_DROP_LIMIT_SPECIFIC_INFO),TRUE,Value.Val == 2);
		if(m_paFiles || m_Category)
			SetTreeRadio(m_ctrlTreeOptions,htiLimitDefault,GetResString(IDS_X_DEFAULT),htiLimit,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
		if(m_paFiles)
			SetTreeRadio(m_ctrlTreeOptions,htiLimitGlobal,GetResString(IDS_X_GLOBAL),htiLimit,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

void CPPgSources::SetTreeRadioNumEditForSDT2(HTREEITEM &htiTime,HTREEITEM &htiDropProgressiv,HTREEITEM &htiDropDistributiv,HTREEITEM &htiDropCummulativ,HTREEITEM &htiDropDefault,HTREEITEM &htiDropGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeNumEdit(m_ctrlTreeOptions,htiTime,GetResString(IDS_X_DROP_INTERVALS), htiParent,GetResString(IDS_X_DROP_INTERVALS_INFO));
		SetTreeRadio(m_ctrlTreeOptions,htiDropProgressiv,MkRdBtnLbl(GetResString(IDS_X_DROP_INTERVALS_PROGRESSIV),Value,0),htiTime,GetResString(IDS_X_DROP_INTERVALS_PROGRESSIV_INFO),TRUE,Value.Val == 0);
		SetTreeRadio(m_ctrlTreeOptions,htiDropDistributiv,MkRdBtnLbl(GetResString(IDS_X_DROP_INTERVALS_DISTRIBUTIV),Value,1),htiTime,GetResString(IDS_X_DROP_INTERVALS_DISTRIBUTIV_INFO),TRUE,Value.Val == 1);
		SetTreeRadio(m_ctrlTreeOptions,htiDropCummulativ,MkRdBtnLbl(GetResString(IDS_X_DROP_INTERVALS_CUMULATIV),Value,2),htiTime,GetResString(IDS_X_DROP_INTERVALS_CUMULATIV_INFO),TRUE,Value.Val == 2);
		if(m_paFiles || m_Category)
			SetTreeRadio(m_ctrlTreeOptions,htiDropDefault,GetResString(IDS_X_DEFAULT),htiTime,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
		if(m_paFiles)
			SetTreeRadio(m_ctrlTreeOptions,htiDropGlobal,GetResString(IDS_X_GLOBAL),htiTime,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}
// NEO: SDT END

void CPPgSources::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgSR = 8;
		int iImgSM = 8;
		int iImgSC = 8;
		int iImgASL = 8;
		int iImgAHL = 8;
		int iImgCSL = 8;
		int iImgGSL = 8;
		int iImgXs = 8;
		int iImgSvr = 8;
		int iImgKad = 8;
		int iImgUdp = 8;

		int	iImgReask = 8;

		int iImgDrop = 8;
		int iImgBad = 8;
		int iImgNNP = 8;
		int iImgFullQ = 8;
		int iImgHighQ = 8;

		int iImgA4AF = 8;

        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgSR = piml->Add(CTempIconLoader(_T("COLLECTING")));
			iImgSM = piml->Add(CTempIconLoader(_T("COLLECTINGCONTROL")));
			iImgSC = piml->Add(CTempIconLoader(_T("CACHE")));
			iImgASL = piml->Add(CTempIconLoader(_T("ASL")));
			iImgAHL = piml->Add(CTempIconLoader(_T("AHL")));
			iImgCSL = piml->Add(CTempIconLoader(_T("CSL")));
			iImgGSL = piml->Add(CTempIconLoader(_T("GSL")));
			iImgXs = piml->Add(CTempIconLoader(_T("XSSRC")));
			iImgSvr = piml->Add(CTempIconLoader(_T("SVRSRC")));
			iImgKad = piml->Add(CTempIconLoader(_T("KADSRC")));
			iImgUdp = piml->Add(CTempIconLoader(_T("UDPSRC")));

			iImgReask = piml->Add(CTempIconLoader(_T("REASKDL")));

			iImgDrop = piml->Add(CTempIconLoader(_T("DROP")));
			iImgBad = piml->Add(CTempIconLoader(_T("DROPBAD")));
			iImgNNP = piml->Add(CTempIconLoader(_T("DROPNNP")));
			iImgFullQ = piml->Add(CTempIconLoader(_T("DROPFULLQ")));
			iImgHighQ = piml->Add(CTempIconLoader(_T("DROPHIGHQ")));

			iImgA4AF = piml->Add(CTempIconLoader(_T("CATADVA4AF")));
		}

		// NEO: SRT - [SourceRequestTweaks]
		SetTreeGroup(m_ctrlTreeOptions,m_htiSourceRequest,GetResString(IDS_X_SOURCE_REQUEST_TWEAKS),iImgSR, TVI_ROOT, GetResString(IDS_X_SOURCE_REQUEST_TWEAKS_INFO));
			// General
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceLimit,GetResString(IDS_X_MAX_SOURCE), m_htiSourceRequest, GetResString(IDS_X_MAX_SOURCE_INFO));

			// Management
			SetTreeGroup(m_ctrlTreeOptions,m_htiSourceManagement,GetResString(IDS_X_SOURCE_MANAGEMENT_TWEAKS),iImgSM, m_htiSourceRequest, GetResString(IDS_X_SOURCE_MANAGEMENT_TWEAKS_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiSwapLimit,GetResString(IDS_X_SWAP_LIMIT), m_htiSourceManagement, GetResString(IDS_X_SWAP_LIMIT_INFO));

				// NEO: NXC - [NewExtendedCategories]
				if(m_paFiles){
					SetTreeGroup(m_ctrlTreeOptions,m_htiA4AFFlags,GetResString(IDS_X_A4AF_FLAGS),iImgA4AF, m_htiSourceManagement, GetResString(IDS_X_A4AF_FLAGS_INFO));
						SetTreeRadio(m_ctrlTreeOptions,m_htiA4AFFlagsNone,GetResString(IDS_X_A4AF_NONEFLAG),m_htiA4AFFlags,GetResString(IDS_X_A4AF_NONEFLAG_INFO),TRUE,m_ForceA4AF == 0);
						SetTreeRadio(m_ctrlTreeOptions,m_htiA4AFFlagsOn,GetResString(IDS_X_A4AF_ONFLAG),m_htiA4AFFlags,GetResString(IDS_X_A4AF_ONFLAG_INFO),TRUE,m_ForceA4AF == 1);
						SetTreeRadio(m_ctrlTreeOptions,m_htiA4AFFlagsOff,GetResString(IDS_X_A4AF_OFFFLAG),m_htiA4AFFlags,GetResString(IDS_X_A4AF_OFFFLAG_INFO),TRUE,m_ForceA4AF == 2);
				}else if(!m_Category){
					SetTreeGroup(m_ctrlTreeOptions,m_htiAdvancedA4AFMode,GetResString(IDS_X_ADVANCED_A4AF_MODE),iImgA4AF, m_htiSourceManagement, GetResString(IDS_X_ADVANCED_A4AF_MODE_INFO));
						SetTreeRadio(m_ctrlTreeOptions,m_htiAdvancedA4AFModeDisabled,GetResString(IDS_X_ADVANCED_A4AF_MODE_DISABLED),m_htiAdvancedA4AFMode,GetResString(IDS_X_ADVANCED_A4AF_MODE_DISABLED_INFO),FALSE,m_iAdvancedA4AFMode == 0);
						SetTreeRadio(m_ctrlTreeOptions,m_htiAdvancedA4AFModeBalance,GetResString(IDS_X_ADVANCED_A4AF_MODE_BALANCE),m_htiAdvancedA4AFMode,GetResString(IDS_X_ADVANCED_A4AF_MODE_BALANCE_INFO),FALSE,m_iAdvancedA4AFMode == 1);
						SetTreeRadio(m_ctrlTreeOptions,m_htiAdvancedA4AFModeStack,GetResString(IDS_X_ADVANCED_A4AF_MODE_STACK),m_htiAdvancedA4AFMode,GetResString(IDS_X_ADVANCED_A4AF_MODE_STACK_INFO),FALSE,m_iAdvancedA4AFMode == 2);
						SetTreeCheck(m_ctrlTreeOptions,m_htiSmartA4AFSwapping,GetResString(IDS_X_SMART_A4AF_SWAPPING),m_htiAdvancedA4AFMode,GetResString(IDS_X_SMART_A4AF_SWAPPING_INFO),FALSE,m_bSmartA4AFSwapping);
				}
				// NEO: NXC END

				// NEO: XSC - [ExtremeSourceCache]
				SetTreeGroup(m_ctrlTreeOptions,m_htiSourceCache,GetResString(IDS_X_SOURCE_CACHE),iImgSC, m_htiSourceManagement, GetResString(IDS_X_SOURCE_CACHE_INFO));
					SetTreeRadioForSRT(m_htiSourceCacheDisable,m_htiSourceCacheEnable,m_htiSourceCacheDefault,m_htiSourceCacheGlobal,m_htiSourceCache,m_UseSourceCache);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceCacheLimit,GetResString(IDS_X_SOURCE_CACHE_LIMIT), m_htiSourceCache,GetResString(IDS_X_SOURCE_CACHE_LIMIT_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceCacheTime,GetResString(IDS_X_SOURCE_CACHE_TIME), m_htiSourceCache,GetResString(IDS_X_SOURCE_CACHE_TIME_INFO));
				// NEO: XSC END

				// NEO: ASL - [AutoSoftLock]
				SetTreeGroup(m_ctrlTreeOptions,m_htiAutoSoftLock,GetResString(IDS_X_AUTO_SOFT_LOCK),iImgASL, m_htiSourceManagement, GetResString(IDS_X_AUTO_SOFT_LOCK_INFO));
					SetTreeRadioForSRT(m_htiAutoSoftLockDisable,m_htiAutoSoftLockEnable,m_htiAutoSoftLockDefault,m_htiAutoSoftLockGlobal,m_htiAutoSoftLock,m_AutoSoftLock);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiAutoSoftLockLimit,GetResString(IDS_X_AUTO_SOFT_LOCK_LIMIT), m_htiAutoSoftLock,GetResString(IDS_X_AUTO_SOFT_LOCK_LIMIT_INFO));
				// NEO: ASL END

				// NEO: AHL - [AutoHardLimit]
				SetTreeGroup(m_ctrlTreeOptions,m_htiAutoHardLimit,GetResString(IDS_X_AUTO_HARD_LIMIT),iImgAHL, m_htiSourceManagement, GetResString(IDS_X_AUTO_HARD_LIMIT_INFO));
					SetTreeRadioForSRT(m_htiAutoHardLimitDisable,m_htiAutoHardLimitEnable,m_htiAutoHardLimitDefault,m_htiAutoHardLimitGlobal,m_htiAutoHardLimit,m_AutoHardLimit);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiAutoHardLimitTime,GetResString(IDS_X_LIMIT_RECALCULATION_TIME), m_htiAutoHardLimit,GetResString(IDS_X_LIMIT_RECALCULATION_TIME_INFO));
				// NEO: AHL END

				// NEO: CSL - [CategorySourceLimit]
				SetTreeGroup(m_ctrlTreeOptions,m_htiCategorySourceLimit,GetResString(IDS_X_CATEGORY_SOURCE_LIMIT),iImgCSL, m_htiSourceManagement, GetResString(IDS_X_CATEGORY_SOURCE_LIMIT_INFO));
					SetTreeRadioForSRT(m_htiCategorySourceLimitDisable,m_htiCategorySourceLimitEnable,m_htiCategorySourceLimitDefault,m_htiCategorySourceLimitGlobal,m_htiCategorySourceLimit,m_CategorySourceLimit);
					if(m_paFiles)
						m_ctrlTreeOptions.SetItemEnable(m_htiCategorySourceLimitEnable,FALSE);
					else{
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiCategorySourceLimitLimit,GetResString(IDS_X_LIMIT_LIMIT), m_htiCategorySourceLimit,GetResString(IDS_X_LIMIT_LIMIT_INFO));
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiCategorySourceLimitTime,GetResString(IDS_X_LIMIT_RECALCULATION_TIME), m_htiCategorySourceLimit,GetResString(IDS_X_LIMIT_RECALCULATION_TIME_INFO));
					}
				// NEO: CSL END

				// NEO: GSL - [GlobalSourceLimit]
				SetTreeGroup(m_ctrlTreeOptions,m_htiGlobalSourceLimit,GetResString(IDS_X_GLOBAL_SOURCE_LIMIT),iImgGSL, m_htiSourceManagement, GetResString(IDS_X_GLOBAL_SOURCE_LIMIT_INFO));
					SetTreeRadioForSRT(m_htiGlobalSourceLimitDisable,m_htiGlobalSourceLimitEnable,m_htiGlobalSourceLimitDefault,m_htiGlobalSourceLimitGlobal,m_htiGlobalSourceLimit,m_GlobalSourceLimit);
					if(m_paFiles || m_Category)
						m_ctrlTreeOptions.SetItemEnable(m_htiGlobalSourceLimitEnable,FALSE);
					else{
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiGlobalSourceLimitLimit,GetResString(IDS_X_LIMIT_LIMIT), m_htiGlobalSourceLimit,GetResString(IDS_X_LIMIT_LIMIT_INFO));
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiGlobalSourceLimitTime,GetResString(IDS_X_LIMIT_RECALCULATION_TIME), m_htiGlobalSourceLimit,GetResString(IDS_X_LIMIT_RECALCULATION_TIME_INFO));
					}
				// NEO: GSL END

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiMinSourcePerFile,GetResString(IDS_X_MIN_SOURCE_PER_FILE), m_htiSourceManagement,GetResString(IDS_X_MIN_SOURCE_PER_FILE_INFO));

			//XS
			SetTreeGroup(m_ctrlTreeOptions,m_htiXs,GetResString(IDS_X_XS_SOURCE),iImgXs, m_htiSourceRequest, GetResString(IDS_X_XS_SOURCE_INFO));
				SetTreeRadioForSRT(m_htiXsDisable, m_htiXsEnable, m_htiXsDefault, m_htiXsGlobal, m_htiXs, m_XsEnable);
				SetTreeNumEditForSRT(m_htiXsLimit, m_htiXsIntervals, m_htiXs);
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiXsClientIntervals,GetResString(IDS_X_CLIENT_REQUEST_INTERVALS), m_htiXs,GetResString(IDS_X_CLIENT_REQUEST_INTERVALS_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiXsCleintDelay,GetResString(IDS_X_CLIENT_DELAY), m_htiXs,GetResString(IDS_X_CLIENT_DELAY_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiXsRareLimit,GetResString(IDS_X_RARE_LIMIT), m_htiXs,GetResString(IDS_X_RARE_LIMIT_INFO));

			// SVR
			SetTreeGroup(m_ctrlTreeOptions,m_htiSvr,GetResString(IDS_X_SVR_SOURCE),iImgSvr, m_htiSourceRequest, GetResString(IDS_X_SVR_SOURCE_INFO));
				SetTreeRadioForSRT(m_htiSvrDisable, m_htiSvrEnable, m_htiSvrDefault, m_htiSvrGlobal, m_htiSvr, m_SvrEnable);
				SetTreeNumEditForSRT(m_htiSvrLimit, m_htiSvrIntervals, m_htiSvr);

			//KAD
			SetTreeGroup(m_ctrlTreeOptions,m_htiKad,GetResString(IDS_X_KAD_SOURCE),iImgKad, m_htiSourceRequest, GetResString(IDS_X_KAD_SOURCE_INFO));
				SetTreeRadioForSRT(m_htiKadDisable, m_htiKadEnable, m_htiKadDefault, m_htiKadGlobal, m_htiKad, m_KadEnable);
				SetTreeNumEditForSRT(m_htiKadLimit, m_htiKadIntervals, m_htiKad);
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiKadMaxFiles,GetResString(IDS_X_KAD_MAX_FILES), m_htiKad,GetResString(IDS_X_KAD_MAX_FILES_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiKadRepeatDelay,GetResString(IDS_X_KAD_DELAY), m_htiKad,GetResString(IDS_X_KAD_DELAY_INFO));

			//UDP
			SetTreeGroup(m_ctrlTreeOptions,m_htiUdp,GetResString(IDS_X_UDP_SOURCE),iImgUdp, m_htiSourceRequest, GetResString(IDS_X_UDP_SOURCE_INFO));
				SetTreeRadioForSRT(m_htiUdpDisable, m_htiUdpEnable, m_htiUdpDefault, m_htiUdpGlobal, m_htiUdp, m_UdpEnable);
				SetTreeNumEditForSRT(m_htiUdpLimit, m_htiUdpIntervals, m_htiUdp);
				if(m_Category == NULL && m_paFiles == NULL){
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiUdpGlobalIntervals,GetResString(IDS_X_GLOBAL_INTERVAL), m_htiUdp,GetResString(IDS_X_GLOBAL_INTERVAL_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiUdpFilesPerServer,GetResString(IDS_X_UDP_MAX_FILES), m_htiUdp,GetResString(IDS_X_UDP_MAX_FILES_INFO));
				}
		// NEO: SRT END

			SetTreeNumEdit(m_ctrlTreeOptions,m_htiTCPConnectionRetry,GetResString(IDS_X_TCP_CONNECTION_RETRY), m_htiSourceRequest,GetResString(IDS_X_TCP_CONNECTION_RETRY_INFO)); // NEO: TCR - [TCPConnectionRetry]

		// NEO: DRT - [DownloadReaskTweaks]
		SetTreeGroup(m_ctrlTreeOptions,m_htiDownloadReask,GetResString(IDS_X_DOWNLOAD_REASK_TWEAKS),iImgReask, TVI_ROOT, GetResString(IDS_X_DOWNLOAD_REASK_TWEAKS_INFO));
			if(m_Category == NULL && m_paFiles == NULL)
				SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiSpreadReask,GetResString(IDS_X_SPREAD_REASK),m_htiDownloadReask,GetResString(IDS_X_SPREAD_REASK_INFO),FALSE,m_SpreadReaskEnable);

			SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceReaskTime,GetResString(IDS_X_NORMAL_REASK_INTERVALS), m_htiDownloadReask,GetResString(IDS_X_NORMAL_REASK_INTERVALS_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiFullQSourceReaskTime,GetResString(IDS_X_FULLQ_REASK_INTERVALS), m_htiDownloadReask,GetResString(IDS_X_FULLQ_REASK_INTERVALS_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiNNPSourceReaskTime,GetResString(IDS_X_NNP_REASK_INTERVALS), m_htiDownloadReask,GetResString(IDS_X_NNP_REASK_INTERVALS_INFO));
		// NEO: DRT END

		// NEO: SDT - [SourcesDropTweaks]
		SetTreeGroup(m_ctrlTreeOptions,m_htiSourcesDrop,GetResString(IDS_X_SOURCES_DROP_TWEAKS),iImgDrop, TVI_ROOT, GetResString(IDS_X_SOURCES_DROP_TWEAKS_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiDropTime,GetResString(IDS_X_DROP_TIME), m_htiSourcesDrop,GetResString(IDS_X_DROP_TIME_INFO));

			//Bad
			SetTreeGroup(m_ctrlTreeOptions,m_htiBad,GetResString(IDS_X_BAD_DROP),iImgBad, m_htiSourcesDrop, GetResString(IDS_X_BAD_DROP_INFO));
				SetTreeRadioForSDT(m_htiBadSourceDontDrop, m_htiBadSourceDrop, m_htiBadSourceTrySwap, m_htiBadSourceDefault, m_htiBadSourceGlobal, m_htiBad, m_BadSourceDrop,true);
				SetTreeRadioNumEditForSDT1(m_htiBadSourceLimit,m_htiBadSourceLimitTotal,m_htiBadSourceLimitRelativ,m_htiBadSourceLimitSpecyfic,m_htiBadSourceLimitDefault,m_htiBadSourceLimitGlobal, m_htiBad, m_BadSourceLimitMode);
				SetTreeRadioNumEditForSDT2(m_htiBadSourceDropTime,m_htiBadSourceDropProgressiv,m_htiBadSourceDropDistributiv,m_htiBadSourceDropCummulativ,m_htiBadSourceDropDefault,m_htiBadSourceDropGlobal, m_htiBad, m_BadSourceDropMode);

			//NNP
			SetTreeGroup(m_ctrlTreeOptions,m_htiNNP,GetResString(IDS_X_NNP_DROP),iImgNNP, m_htiSourcesDrop, GetResString(IDS_X_NNP_DROP_INFO));
				SetTreeRadioForSDT(m_htiNNPSourceDontDrop, m_htiNNPSourceDrop, m_htiNNPSourceTrySwap, m_htiNNPSourceDefault, m_htiNNPSourceGlobal, m_htiNNP, m_NNPSourceDrop);
				SetTreeRadioNumEditForSDT1(m_htiNNPSourceLimit,m_htiNNPSourceLimitTotal,m_htiNNPSourceLimitRelativ,m_htiNNPSourceLimitSpecyfic,m_htiNNPSourceLimitDefault,m_htiNNPSourceLimitGlobal, m_htiNNP, m_NNPSourceLimitMode);
				SetTreeRadioNumEditForSDT2(m_htiNNPSourceDropTime,m_htiNNPSourceDropProgressiv,m_htiNNPSourceDropDistributiv,m_htiNNPSourceDropCummulativ,m_htiNNPSourceDropDefault,m_htiNNPSourceDropGlobal, m_htiNNP, m_NNPSourceDropMode);

			//FullQ
			SetTreeGroup(m_ctrlTreeOptions,m_htiFullQ,GetResString(IDS_X_FULLQ_DROP),iImgFullQ, m_htiSourcesDrop, GetResString(IDS_X_FULLQ_DROP_INFO));
				SetTreeRadioForSDT(m_htiFullQSourceDontDrop, m_htiFullQSourceDrop, m_htiFullQSourceTrySwap, m_htiFullQSourceDefault, m_htiFullQSourceGlobal, m_htiFullQ, m_FullQSourceDrop);
				SetTreeRadioNumEditForSDT1(m_htiFullQSourceLimit,m_htiFullQSourceLimitTotal,m_htiFullQSourceLimitRelativ,m_htiFullQSourceLimitSpecyfic,m_htiFullQSourceLimitDefault,m_htiFullQSourceLimitGlobal, m_htiFullQ, m_FullQSourceLimitMode);
				SetTreeRadioNumEditForSDT2(m_htiFullQSourceDropTime,m_htiFullQSourceDropProgressiv,m_htiFullQSourceDropDistributiv,m_htiFullQSourceDropCummulativ,m_htiFullQSourceDropDefault,m_htiFullQSourceDropGlobal, m_htiFullQ, m_FullQSourceDropMode);

			//HighQ
			SetTreeGroup(m_ctrlTreeOptions,m_htiHighQ,GetResString(IDS_X_HIGHQ_DROP),iImgHighQ, m_htiSourcesDrop, GetResString(IDS_X_HIGHQ_DROP_INFO));
				SetTreeRadioForSDT(m_htiHighQSourceDontDrop, m_htiHighQSourceDrop, m_htiHighQSourceTrySwap, m_htiHighQSourceDefault, m_htiHighQSourceGlobal, m_htiHighQ, m_HighQSourceDrop);
				SetTreeRadioNumEditForSDT1(m_htiHighQSourceLimit,m_htiHighQSourceLimitTotal,m_htiHighQSourceLimitRelativ,m_htiHighQSourceLimitSpecyfic,m_htiHighQSourceLimitDefault,m_htiHighQSourceLimitGlobal, m_htiHighQ, m_HighQSourceLimitMode);
				SetTreeRadioNumEditForSDT2(m_htiHighQSourceDropTime,m_htiHighQSourceDropProgressiv,m_htiHighQSourceDropDistributiv,m_htiHighQSourceDropCummulativ,m_htiHighQSourceDropDefault,m_htiHighQSourceDropGlobal, m_htiHighQ, m_HighQSourceDropMode);

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiHighQSourceMaxRank,GetResString(IDS_X_HIGHQ_MAX_RANK),m_htiHighQ,GetResString(IDS_X_HIGHQ_MAX_RANK_INFO));
					SetTreeRadio(m_ctrlTreeOptions,m_htiHighQSourceRankNormal,MkRdBtnLbl(GetResString(IDS_X_HIGHQ_MAX_RANK_NORMAL),m_HighQSourceRankMode,0),m_htiHighQSourceMaxRank,GetResString(IDS_X_HIGHQ_MAX_RANK_NORMAL_INFO),TRUE,m_HighQSourceRankMode.Val == 0);
					SetTreeRadio(m_ctrlTreeOptions,m_htiHighQSourceRankAverage,MkRdBtnLbl(GetResString(IDS_X_HIGHQ_MAX_RANK_AVERAGE),m_HighQSourceRankMode,1),m_htiHighQSourceMaxRank,GetResString(IDS_X_HIGHQ_MAX_RANK_AVERAGE_INFO),TRUE,m_HighQSourceRankMode.Val == 1);
					if(m_paFiles || m_Category)
						SetTreeRadio(m_ctrlTreeOptions,m_htiHighQSourceRankDefault,GetResString(IDS_X_DEFAULT),m_htiHighQSourceMaxRank,GetResString(IDS_X_DEFAULT_INFO),TRUE,m_HighQSourceRankMode.Val == FCFG_DEF);
					if(m_paFiles)
						SetTreeRadio(m_ctrlTreeOptions,m_htiHighQSourceRankGlobal,GetResString(IDS_X_GLOBAL),m_htiHighQSourceMaxRank,GetResString(IDS_X_GLOBAL_INFO),TRUE,m_HighQSourceRankMode.Val == FCFG_GLB);

			SetTreeNumEdit(m_ctrlTreeOptions,m_htiDeadTime,GetResString(IDS_X_DEAD_TIME), m_htiSourcesDrop,GetResString(IDS_X_DEAD_TIME_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiDeadTimeFWMulti,GetResString(IDS_X_DEAD_TIME_FW), m_htiDeadTime,GetResString(IDS_X_DEAD_TIME_FW_INFO));
			if(m_Category == NULL && m_paFiles == NULL){
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiGlobalDeadTime,GetResString(IDS_X_GLOBAL_DEAD_TIME), m_htiSourcesDrop,GetResString(IDS_X_GLOBAL_DEAD_TIME_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiGlobalDeadTimeFWMulti,GetResString(IDS_X_DEAD_TIME_FW), m_htiGlobalDeadTime,GetResString(IDS_X_DEAD_TIME_FW_INFO));
			}
		// NEO: SDT END

		m_ctrlTreeOptions.Expand(m_htiSourceRequest, TVE_EXPAND);
		//m_ctrlTreeOptions.Expand(m_htiDownloadReask, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiSourcesDrop, TVE_EXPAND);
		m_bInitializedTreeOpts = true;
	}

	// NEO: SRT - [SourceRequestTweaks]
		// General
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceLimit, m_SourceLimit);

		// Management
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSwapLimit, m_SwapLimit);

			// NEO: NXC - [NewExtendedCategories]
			if(m_paFiles){
				DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiA4AFFlags, m_ForceA4AF); 
			}else if(!m_Category){
				DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiAdvancedA4AFMode, m_iAdvancedA4AFMode);
				DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSmartA4AFSwapping, m_bSmartA4AFSwapping);
			}
			// NEO: NXC END

			// NEO: XSC - [ExtremeSourceCache]
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSourceCache, m_UseSourceCache);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceCacheLimit, m_SourceCacheLimit);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceCacheTime, m_SourceCacheTime);
			// NEO: XSC END

			// NEO: ASL - [AutoSoftLock]
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiAutoSoftLock, m_AutoSoftLock);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAutoSoftLockLimit, m_AutoSoftLockLimit);
			// NEO: ASL END

			// NEO: AHL - [AutoHardLimit]
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiAutoHardLimit, m_AutoHardLimit);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAutoHardLimitTime, m_AutoHardLimitTime);
			// NEO: AHL END

			// NEO: CSL - [CategorySourceLimit]
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiCategorySourceLimit, m_CategorySourceLimit);
				if(!m_paFiles)
				{
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiCategorySourceLimitLimit, m_CategorySourceLimitLimit);
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiCategorySourceLimitTime, m_CategorySourceLimitTime);
				}
			// NEO: CSL END

			// NEO: GSL - [GlobalSourceLimit]
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiGlobalSourceLimit, m_GlobalSourceLimit);
				if(!m_paFiles && !m_Category)
				{
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiGlobalSourceLimitLimit, m_GlobalSourceLimitLimit);
					DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiGlobalSourceLimitTime, m_GlobalSourceLimitTime);
				}
			// NEO: GSL END

				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMinSourcePerFile, m_MinSourcePerFile);

		//XS
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiXs, m_XsEnable);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiXsLimit, m_XsLimit);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiXsIntervals, m_XsIntervals);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiXsClientIntervals, m_XsClientIntervals);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiXsCleintDelay, m_XsCleintDelay);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiXsRareLimit, m_XsRareLimit);

		// SVR
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSvr, m_SvrEnable);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSvrLimit, m_SvrLimit);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSvrIntervals, m_SvrIntervals);

		//KAD
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiKad, m_KadEnable);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiKadLimit, m_KadLimit);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiKadIntervals, m_KadIntervals);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiKadMaxFiles, m_KadMaxFiles);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiKadRepeatDelay, m_KadRepeatDelay);

		//UDP
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiUdp, m_UdpEnable);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiUdpLimit, m_UdpLimit);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiUdpIntervals, m_UdpIntervals);
			if(m_Category == NULL && m_paFiles == NULL){
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiUdpGlobalIntervals, m_UdpGlobalIntervals);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiUdpFilesPerServer, m_UdpFilesPerServer);
			}
	// NEO: SRT END

		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiTCPConnectionRetry, m_TCPConnectionRetry); // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
		if(m_Category == NULL && m_paFiles == NULL){
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSpreadReask, m_SpreadReaskEnable);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSpreadReask, m_SpreadReaskTime);
		}
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceReaskTime, m_SourceReaskTime);
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiFullQSourceReaskTime, m_FullQSourceReaskTime);
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiNNPSourceReaskTime, m_NNPSourceReaskTime);
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiDropTime, m_DropTime);

		//Bad
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiBad, m_BadSourceDrop);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiBadSourceLimit, m_BadSourceLimit);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiBadSourceLimit, m_BadSourceLimitMode);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiBadSourceDropTime, m_BadSourceDropTime);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiBadSourceDropTime, m_BadSourceDropMode);

		//NNP
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiNNP, m_NNPSourceDrop);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiNNPSourceLimit, m_NNPSourceLimit);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiNNPSourceLimit, m_NNPSourceLimitMode);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiNNPSourceDropTime, m_NNPSourceDropTime);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiNNPSourceDropTime, m_NNPSourceDropMode);

		//FullQ
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiFullQ, m_FullQSourceDrop);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiFullQSourceLimit, m_FullQSourceLimit);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiFullQSourceLimit, m_FullQSourceLimitMode);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiFullQSourceDropTime, m_FullQSourceDropTime);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiFullQSourceDropTime, m_FullQSourceDropMode);

		//HighQ
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHighQ, m_HighQSourceDrop);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiHighQSourceLimit, m_HighQSourceLimit);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHighQSourceLimit, m_HighQSourceLimitMode);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiHighQSourceDropTime, m_HighQSourceDropTime);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHighQSourceDropTime, m_HighQSourceDropMode);

			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiHighQSourceMaxRank, m_HighQSourceMaxRank);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHighQSourceMaxRank, m_HighQSourceRankMode);

		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiDeadTime, m_DeadTime);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiDeadTimeFWMulti, m_DeadTimeFWMulti);
		if(m_Category == NULL && m_paFiles == NULL){
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiGlobalDeadTime, m_GlobalDeadTime);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiGlobalDeadTimeFWMulti, m_GlobalDeadTimeFWMulti);
		}
	// NEO: SDT END
}

BOOL CPPgSources::OnInitDialog()
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

void CPPgSources::SetLimits(){
	// NEO: SRT - [SourceRequestTweaks]
	// General
	m_SourceLimit.MSMG(MIN_SOURCE_LIMIT,DEF_SOURCE_LIMIT,MAX_SOURCE_LIMIT,NeoPrefs.PartPrefs.m_SourceLimit,'n');

	// Management
	m_SwapLimit.MSMG(MIN_SWAP_LIMIT,DEF_SWAP_LIMIT,MAX_SWAP_LIMIT,NeoPrefs.PartPrefs.m_SwapLimit,'n');

	//XS
	m_XsLimit.MSMG(MIN_XS_LIMIT,DEF_XS_LIMIT,MAX_XS_LIMIT,NeoPrefs.PartPrefs.m_XsLimit,'n');
	m_XsIntervals.MSMG(MIN_XS_INTERVALS,DEF_XS_INTERVALS,MAX_XS_INTERVALS,NeoPrefs.PartPrefs.m_XsIntervals,'t');
	m_XsClientIntervals.MSMG(MIN_XS_CLIENT_INTERVALS,DEF_XS_CLIENT_INTERVALS,MAX_XS_CLIENT_INTERVALS,NeoPrefs.PartPrefs.m_XsClientIntervals,'t');
	m_XsCleintDelay.MSMG(MIN_XS_CLEINT_DELAY,DEF_XS_CLEINT_DELAY,MAX_XS_CLEINT_DELAY,NeoPrefs.PartPrefs.m_XsCleintDelay);
	m_XsRareLimit.MSMG(MIN_XS_RARE_LIMIT,DEF_XS_RARE_LIMIT,MAX_XS_RARE_LIMIT,NeoPrefs.PartPrefs.m_XsRareLimit);

	// SVR
	m_SvrLimit.MSMG(MIN_SVR_LIMIT,DEF_SVR_LIMIT,MAX_SVR_LIMIT,NeoPrefs.PartPrefs.m_SvrLimit,'n');
	m_SvrIntervals.MSMG(MIN_SVR_INTERVALS,DEF_SVR_INTERVALS,MAX_SVR_INTERVALS,NeoPrefs.PartPrefs.m_SvrIntervals,'t');

	//KAD
	m_KadLimit.MSMG(MIN_KAD_LIMIT,DEF_KAD_LIMIT,MAX_KAD_LIMIT,NeoPrefs.PartPrefs.m_KadLimit,'n');
	m_KadIntervals.MSMG(MIN_KAD_INTERVALS,DEF_KAD_INTERVALS,MAX_KAD_INTERVALS,NeoPrefs.PartPrefs.m_KadIntervals,'t');
	m_KadMaxFiles.MSMG(MIN_KAD_MAX_FILES,DEF_KAD_MAX_FILES,MAX_KAD_MAX_FILES,NeoPrefs.PartPrefs.m_KadMaxFiles);
	m_KadRepeatDelay.MSMG(MIN_KAD_REPEAT_DELAY,DEF_KAD_REPEAT_DELAY,MAX_KAD_REPEAT_DELAY,NeoPrefs.PartPrefs.m_KadRepeatDelay);

	//UDP
	m_UdpLimit.MSMG(MIN_UDP_LIMIT,DEF_UDP_LIMIT,MAX_UDP_LIMIT,NeoPrefs.PartPrefs.m_UdpLimit,'n');
	m_UdpIntervals.MSMG(MIN_UDP_INTERVALS,DEF_UDP_INTERVALS,MAX_UDP_INTERVALS,NeoPrefs.PartPrefs.m_UdpIntervals,'t');
	if(m_Category == NULL && m_paFiles == NULL){
		m_UdpGlobalIntervals.MSMG(MIN_UDP_GLOBAL_INTERVALS,DEF_UDP_GLOBAL_INTERVALS,MAX_UDP_GLOBAL_INTERVALS,NeoPrefs.PartPrefs.m_UdpGlobalIntervals,'t');
		m_UdpFilesPerServer.MSMG(MIN_UDP_FILES_PER_SERVER,DEF_UDP_FILES_PER_SERVER,MAX_UDP_FILES_PER_SERVER,NeoPrefs.PartPrefs.m_UdpFilesPerServer);
	}
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	m_SourceCacheLimit.MSMG(MIN_SOURCE_CACHE_LIMIT,DEF_SOURCE_CACHE_LIMIT,MAX_SOURCE_CACHE_LIMIT,NeoPrefs.PartPrefs.m_SourceCacheLimit,'n');
	m_SourceCacheTime.MSMG(MIN_SOURCE_CACHE_TIME,DEF_SOURCE_CACHE_TIME,MAX_SOURCE_CACHE_TIME,NeoPrefs.PartPrefs.m_SourceCacheTime,'t');
	// NEO: XSC END

	m_AutoSoftLockLimit.MSMG(MIN_AUTO_SOFT_LOCK_LIMIT,DEF_AUTO_SOFT_LOCK_LIMIT,DEF_AUTO_SOFT_LOCK_LIMIT,NeoPrefs.PartPrefs.m_AutoSoftLock,'n'); // NEO: ASL - [AutoSoftLock]

	m_AutoHardLimitTime.MSMG(MIN_AUTO_HARD_LIMIT_TIME,DEF_AUTO_HARD_LIMIT_TIME,MAX_AUTO_HARD_LIMIT_TIME,NeoPrefs.PartPrefs.m_AutoHardLimitTime,'t'); // NEO: AHL - [AutoHardLimit]

	// NEO: CSL - [CategorySourceLimit]
	m_CategorySourceLimitLimit.MSMG(MIN_CATEGORY_SOURCE_LIMIT_LIMIT,DEF_CATEGORY_SOURCE_LIMIT_LIMIT,MAX_CATEGORY_SOURCE_LIMIT_LIMIT,NeoPrefs.PartPrefs.m_CategorySourceLimitLimit,'n');
	m_CategorySourceLimitTime.MSMG(MIN_CATEGORY_SOURCE_LIMIT_TIME,DEF_CATEGORY_SOURCE_LIMIT_TIME,MAX_CATEGORY_SOURCE_LIMIT_TIME,NeoPrefs.PartPrefs.m_CategorySourceLimitTime,'t');
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	m_GlobalSourceLimitLimit.MSMG(MIN_GLOBAL_SOURCE_LIMIT_LIMIT,DEF_GLOBAL_SOURCE_LIMIT_LIMIT,MAX_GLOBAL_SOURCE_LIMIT_LIMIT,NeoPrefs.PartPrefs.m_GlobalSourceLimitLimit,'n');
	m_GlobalSourceLimitTime.MSMG(MIN_GLOBAL_SOURCE_LIMIT_TIME,DEF_GLOBAL_SOURCE_LIMIT_TIME,MAX_GLOBAL_SOURCE_LIMIT_TIME,NeoPrefs.PartPrefs.m_SourceCacheTime,'t');
	// NEO: GSL END

	m_MinSourcePerFile.MSMG(MIN_MIN_SOURCE_PER_FILE,DEF_MIN_SOURCE_PER_FILE,MAX_MIN_SOURCE_PER_FILE,NeoPrefs.PartPrefs.m_MinSourcePerFile,'n');

	m_TCPConnectionRetry.MSMG(MIN_TCP_CONNECTION_RETRY,DEF_TCP_CONNECTION_RETRY,MAX_TCP_CONNECTION_RETRY,NeoPrefs.PartPrefs.m_TCPConnectionRetry); // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	if(m_Category == NULL && m_paFiles == NULL)
		m_SpreadReaskTime.MSMG(MIN_SPREAD_REASK_TIME,DEF_SPREAD_REASK_TIME,MAX_SPREAD_REASK_TIME,NeoPrefs.PartPrefs.m_SpreadReaskTime,'t');
	m_SourceReaskTime.MSMG(MIN_SOURCE_REASK_TIME,DEF_SOURCE_REASK_TIME,MAX_SOURCE_REASK_TIME,NeoPrefs.PartPrefs.m_SourceReaskTime,'t');
	m_FullQSourceReaskTime.MSMG(MIN_FULLQ_SOURCE_REASK_TIME,DEF_FULLQ_SOURCE_REASK_TIME,MAX_FULLQ_SOURCE_REASK_TIME,NeoPrefs.PartPrefs.m_FullQSourceReaskTime,'t');
	m_NNPSourceReaskTime.MSMG(MIN_NNP_SOURCE_REASK_TIME,DEF_NNP_SOURCE_REASK_TIME,MAX_NNP_SOURCE_REASK_TIME,NeoPrefs.PartPrefs.m_NNPSourceReaskTime,'t');
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	m_DropTime.MSMG(MIN_DROP_TIME,DEF_DROP_TIME,MAX_DROP_TIME,NeoPrefs.PartPrefs.m_DropTime,'t');

	//Bad
	m_BadSourceLimit.MSMG(MIN_BAD_SOURCE_LIMIT_1,DEF_BAD_SOURCE_LIMIT_1,MAX_BAD_SOURCE_LIMIT_1,NeoPrefs.PartPrefs.m_BadSourceLimit,'n');
	m_BadSourceLimit.MSMa(MIN_BAD_SOURCE_LIMIT_2,DEF_BAD_SOURCE_LIMIT_2,MAX_BAD_SOURCE_LIMIT_2,1);
	m_BadSourceLimit.MSMa(MIN_BAD_SOURCE_LIMIT_3,DEF_BAD_SOURCE_LIMIT_3,MAX_BAD_SOURCE_LIMIT_3,2,'n');
	m_BadSourceDropTime.MSMG(MIN_BAD_SOURCE_DROP_TIME_1,DEF_BAD_SOURCE_DROP_TIME_1,MAX_BAD_SOURCE_DROP_TIME_1,NeoPrefs.PartPrefs.m_BadSourceDropTime,'t');
	m_BadSourceDropTime.MSMa(MIN_BAD_SOURCE_DROP_TIME_2,DEF_BAD_SOURCE_DROP_TIME_2,MAX_BAD_SOURCE_DROP_TIME_2,1,'t');
	m_BadSourceDropTime.MSMa(MIN_BAD_SOURCE_DROP_TIME_3,DEF_BAD_SOURCE_DROP_TIME_3,MAX_BAD_SOURCE_DROP_TIME_3,2,'t');

	//NNP
	m_NNPSourceLimit.MSMG(MIN_NNP_SOURCE_LIMIT_1,DEF_NNP_SOURCE_LIMIT_1,MAX_NNP_SOURCE_LIMIT_1,NeoPrefs.PartPrefs.m_NNPSourceLimit,'n');
	m_NNPSourceLimit.MSMa(MIN_NNP_SOURCE_LIMIT_2,DEF_NNP_SOURCE_LIMIT_2,MAX_NNP_SOURCE_LIMIT_2,1);
	m_NNPSourceLimit.MSMa(MIN_NNP_SOURCE_LIMIT_3,DEF_NNP_SOURCE_LIMIT_3,MAX_NNP_SOURCE_LIMIT_3,2,'n');
	m_NNPSourceDropTime.MSMG(MIN_NNP_SOURCE_DROP_TIME_1,DEF_NNP_SOURCE_DROP_TIME_1,MAX_NNP_SOURCE_DROP_TIME_1,NeoPrefs.PartPrefs.m_NNPSourceDropTime,'t');
	m_NNPSourceDropTime.MSMa(MIN_NNP_SOURCE_DROP_TIME_2,DEF_NNP_SOURCE_DROP_TIME_2,MAX_NNP_SOURCE_DROP_TIME_2,1,'t');
	m_NNPSourceDropTime.MSMa(MIN_NNP_SOURCE_DROP_TIME_3,DEF_NNP_SOURCE_DROP_TIME_3,MAX_NNP_SOURCE_DROP_TIME_3,2,'t');

	//FullQ
	m_FullQSourceLimit.MSMG(MIN_FULLQ_SOURCE_LIMIT_1,DEF_FULLQ_SOURCE_LIMIT_1,MAX_FULLQ_SOURCE_LIMIT_1,NeoPrefs.PartPrefs.m_FullQSourceLimit,'n');
	m_FullQSourceLimit.MSMa(MIN_FULLQ_SOURCE_LIMIT_2,DEF_FULLQ_SOURCE_LIMIT_2,MAX_FULLQ_SOURCE_LIMIT_2,1);
	m_FullQSourceLimit.MSMa(MIN_FULLQ_SOURCE_LIMIT_3,DEF_FULLQ_SOURCE_LIMIT_3,MAX_FULLQ_SOURCE_LIMIT_3,2,'n');
	m_FullQSourceDropTime.MSMG(MIN_FULLQ_SOURCE_DROP_TIME_1,DEF_FULLQ_SOURCE_DROP_TIME_1,MAX_FULLQ_SOURCE_DROP_TIME_1,NeoPrefs.PartPrefs.m_FullQSourceDropTime,'t');
	m_FullQSourceDropTime.MSMa(MIN_FULLQ_SOURCE_DROP_TIME_2,DEF_FULLQ_SOURCE_DROP_TIME_2,MAX_FULLQ_SOURCE_DROP_TIME_2,1,'t');
	m_FullQSourceDropTime.MSMa(MIN_FULLQ_SOURCE_DROP_TIME_3,DEF_FULLQ_SOURCE_DROP_TIME_3,MAX_FULLQ_SOURCE_DROP_TIME_3,2,'t');

	//HighQ
	m_HighQSourceLimit.MSMG(MIN_HIGHQ_SOURCE_LIMIT_1,DEF_HIGHQ_SOURCE_LIMIT_1,MAX_HIGHQ_SOURCE_LIMIT_1,NeoPrefs.PartPrefs.m_HighQSourceLimit,'n');
	m_HighQSourceLimit.MSMa(MIN_HIGHQ_SOURCE_LIMIT_2,DEF_HIGHQ_SOURCE_LIMIT_2,MAX_HIGHQ_SOURCE_LIMIT_2,1);
	m_HighQSourceLimit.MSMa(MIN_HIGHQ_SOURCE_LIMIT_3,DEF_HIGHQ_SOURCE_LIMIT_3,MAX_HIGHQ_SOURCE_LIMIT_3,2,'n');
	m_HighQSourceDropTime.MSMG(MIN_HIGHQ_SOURCE_DROP_TIME_1,DEF_HIGHQ_SOURCE_DROP_TIME_1,MAX_HIGHQ_SOURCE_DROP_TIME_1,NeoPrefs.PartPrefs.m_HighQSourceDropTime,'t');
	m_HighQSourceDropTime.MSMa(MIN_HIGHQ_SOURCE_DROP_TIME_2,DEF_HIGHQ_SOURCE_DROP_TIME_2,MAX_HIGHQ_SOURCE_DROP_TIME_2,1,'t');
	m_HighQSourceDropTime.MSMa(MIN_HIGHQ_SOURCE_DROP_TIME_3,DEF_HIGHQ_SOURCE_DROP_TIME_3,MAX_HIGHQ_SOURCE_DROP_TIME_3,2,'t');
	m_HighQSourceMaxRank.MSMG(MIN_HIGHQ_SOURCE_MAX_RANK_1,DEF_HIGHQ_SOURCE_MAX_RANK_1,MAX_HIGHQ_SOURCE_MAX_RANK_1,NeoPrefs.PartPrefs.m_HighQSourceMaxRank,'n');
	m_HighQSourceMaxRank.MSMa(MIN_HIGHQ_SOURCE_MAX_RANK_2,DEF_HIGHQ_SOURCE_MAX_RANK_2,MAX_HIGHQ_SOURCE_MAX_RANK_2,1);

	m_DeadTime.MSMG(MIN_DEAD_TIME,DEF_DEAD_TIME,MAX_DEAD_TIME,NeoPrefs.PartPrefs.m_DeadTime,'t');
	m_DeadTimeFWMulti.MSMG(MIN_DEAD_TIME_FW_MILTU,DEF_DEAD_TIME_FW_MILTU,MAX_DEAD_TIME_FW_MILTU,NeoPrefs.PartPrefs.m_DeadTimeFWMulti);
	if(m_Category == NULL && m_paFiles == NULL){
		m_GlobalDeadTime.MSMG(MIN_GLOBAL_DEAD_TIME,DEF_GLOBAL_DEAD_TIME,MIN_GLOBAL_DEAD_TIME,NeoPrefs.PartPrefs.m_GlobalDeadTime,'t');
		m_GlobalDeadTimeFWMulti.MSMG(MIN_GLOBAL_DEAD_TIME_FW_MILTU,DEF_GLOBAL_DEAD_TIME_FW_MILTU,MAX_GLOBAL_DEAD_TIME_FW_MILTU,NeoPrefs.PartPrefs.m_GlobalDeadTimeFWMulti);
	}
	// NEO: SDT END
}

void CPPgSources::LoadSettings()
{
	/*
	* Einstellungen Laden
	*/

	SetLimits();

	if(m_Category)
	{
		CPartPreferences* PartPrefs = m_Category->PartPrefs;

		// set the right default value (for categories its the global one)
		// and the category Value or the FCFG_DEF if category dont have a prefs object

		// NEO: SRT - [SourceRequestTweaks]
		// General
		m_SourceLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SourceLimit : FCFG_DEF);

		// Management
		m_SwapLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SwapLimit : FCFG_DEF);

		//XS
		m_XsEnable.CVDC(1, PartPrefs ? PartPrefs->m_XsEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_XsEnable);
		m_XsLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_XsLimit : FCFG_DEF);
		m_XsIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_XsIntervals : FCFG_DEF);
		m_XsClientIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_XsClientIntervals : FCFG_DEF);
		m_XsCleintDelay.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_XsCleintDelay : FCFG_DEF);
		m_XsRareLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_XsRareLimit : FCFG_DEF);

		// SVR
		m_SvrEnable.CVDC(1, PartPrefs ? PartPrefs->m_SvrEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_SvrEnable);
		m_SvrLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SvrLimit : FCFG_DEF);
		m_SvrIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SvrIntervals : FCFG_DEF);

		//KAD
		m_KadEnable.CVDC(1, PartPrefs ? PartPrefs->m_KadEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_KadEnable);
		m_KadLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_KadLimit : FCFG_DEF);
		m_KadIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_KadIntervals : FCFG_DEF);
		m_KadMaxFiles.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_KadMaxFiles : FCFG_DEF);
		m_KadRepeatDelay.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_KadRepeatDelay : FCFG_DEF);

		//UDP
		m_UdpEnable.CVDC(1, PartPrefs ? PartPrefs->m_UdpEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_UdpEnable);
		m_UdpLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_UdpLimit : FCFG_DEF);
		m_UdpIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_UdpIntervals : FCFG_DEF);
		// NEO: SRT END

		// NEO: XSC - [ExtremeSourceCache]
		m_UseSourceCache.CVDC(1, PartPrefs ? PartPrefs->m_UseSourceCache : FCFG_DEF, NeoPrefs.PartPrefs.m_UseSourceCache);
		m_SourceCacheLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SourceCacheLimit : FCFG_DEF);
		m_SourceCacheTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SourceCacheTime : FCFG_DEF);
		// NEO: XSC END

		// NEO: ASL - [AutoSoftLock]
		m_AutoSoftLock.CVDC(1, PartPrefs ? PartPrefs->m_AutoSoftLock : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoSoftLock);
		m_AutoSoftLockLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_AutoSoftLockLimit : FCFG_DEF);
		// NEO: ASL END

		// NEO: AHL - [AutoHardLimit]
		m_AutoHardLimit.CVDC(1, PartPrefs ? PartPrefs->m_AutoHardLimit : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoHardLimit);
		m_AutoHardLimitTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_AutoHardLimitTime : FCFG_DEF);
		// NEO: AHL END

		// NEO: CSL - [CategorySourceLimit]
		m_CategorySourceLimit.CVDC(1, PartPrefs ? PartPrefs->m_CategorySourceLimit : FCFG_DEF, NeoPrefs.PartPrefs.m_CategorySourceLimit);
		m_CategorySourceLimitLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_CategorySourceLimitLimit : FCFG_DEF);
		m_CategorySourceLimitTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_CategorySourceLimitTime : FCFG_DEF);
		// NEO: CSL END

		// NEO: GSL - [GlobalSourceLimit]
		m_GlobalSourceLimit.CVDC(1, PartPrefs ? PartPrefs->m_GlobalSourceLimit : FCFG_DEF, NeoPrefs.PartPrefs.m_GlobalSourceLimit);
		m_GlobalSourceLimitLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_GlobalSourceLimitLimit : FCFG_DEF);
		m_GlobalSourceLimitTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_GlobalSourceLimitTime : FCFG_DEF);
		// NEO: GSL END

		m_MinSourcePerFile.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_MinSourcePerFile : FCFG_DEF);

		m_TCPConnectionRetry.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_TCPConnectionRetry : FCFG_DEF); // NEO: TCR - [TCPConnectionRetry]

		// NEO: DRT - [DownloadReaskTweaks]
		m_SourceReaskTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_SourceReaskTime : FCFG_DEF);
		m_FullQSourceReaskTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_FullQSourceReaskTime : FCFG_DEF);
		m_NNPSourceReaskTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_NNPSourceReaskTime : FCFG_DEF);
		// NEO: DRT END

		// NEO: SDT - [SourcesDropTweaks]
		m_DropTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_DropTime : FCFG_DEF);

		//Bad
		m_BadSourceDrop.CVDC(1, PartPrefs ? PartPrefs->m_BadSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_BadSourceDrop);
		m_BadSourceLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_BadSourceLimit : FCFG_DEF);
		m_BadSourceLimitMode.CVDC(2, PartPrefs ? PartPrefs->m_BadSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_BadSourceLimitMode);
		m_BadSourceDropTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_BadSourceDropTime : FCFG_DEF);
		m_BadSourceDropMode.CVDC(2, PartPrefs ? PartPrefs->m_BadSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_BadSourceDropMode);

		//NNP
		m_NNPSourceDrop.CVDC(2, PartPrefs ? PartPrefs->m_NNPSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_NNPSourceDrop);
		m_NNPSourceLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_NNPSourceLimit : FCFG_DEF);
		m_NNPSourceLimitMode.CVDC(2, PartPrefs ? PartPrefs->m_NNPSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_NNPSourceLimitMode);
		m_NNPSourceDropTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_NNPSourceDropTime : FCFG_DEF);
		m_NNPSourceDropMode.CVDC(2, PartPrefs ? PartPrefs->m_NNPSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_NNPSourceDropMode);

		//FullQ
		m_FullQSourceDrop.CVDC(2, PartPrefs ? PartPrefs->m_FullQSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_FullQSourceDrop);
		m_FullQSourceLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_FullQSourceLimit : FCFG_DEF);
		m_FullQSourceLimitMode.CVDC(2, PartPrefs ? PartPrefs->m_FullQSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_FullQSourceLimitMode);
		m_FullQSourceDropTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_FullQSourceDropTime : FCFG_DEF);
		m_FullQSourceDropMode.CVDC(2, PartPrefs ? PartPrefs->m_FullQSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_FullQSourceDropMode);

		//HighQ
		m_HighQSourceDrop.CVDC(2, PartPrefs ? PartPrefs->m_HighQSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceDrop);
		m_HighQSourceLimit.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_HighQSourceLimit : FCFG_DEF);
		m_HighQSourceLimitMode.CVDC(2, PartPrefs ? PartPrefs->m_HighQSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceLimitMode);
		m_HighQSourceDropTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_HighQSourceDropTime : FCFG_DEF);
		m_HighQSourceDropMode.CVDC(2, PartPrefs ? PartPrefs->m_HighQSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceDropMode);
		m_HighQSourceMaxRank.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_HighQSourceMaxRank : FCFG_DEF);
		m_HighQSourceRankMode.CVDC(1, PartPrefs ? PartPrefs->m_HighQSourceRankMode : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceRankMode);

		m_DeadTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_DeadTime : FCFG_DEF);
		m_DeadTimeFWMulti.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_DeadTimeFWMulti : FCFG_DEF);
		// NEO: SDT END
	}
	else
	{
		// set the right default value (for global preferences its the hardcoded one)
		// and the global Value

		// NEO: NXC - [NewExtendedCategories]
		m_bSmartA4AFSwapping = NeoPrefs.m_bSmartA4AFSwapping;
		m_iAdvancedA4AFMode = NeoPrefs.m_iAdvancedA4AFMode;
		// NEO: NXC END

		// NEO: SRT - [SourceRequestTweaks]
		// General
		m_SourceLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SourceLimit, true);

		// Management
		m_SwapLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SwapLimit, true);

		//XS
		m_XsEnable.CVDC(1, NeoPrefs.PartPrefs.m_XsEnable);
		m_XsLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_XsLimit, true);
		m_XsIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_XsIntervals, true);
		m_XsClientIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_XsClientIntervals, true);
		m_XsCleintDelay.DV(FCFG_STD, NeoPrefs.PartPrefs.m_XsCleintDelay, true);
		m_XsRareLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_XsRareLimit, true);

		// SVR
		m_SvrEnable.CVDC(1, NeoPrefs.PartPrefs.m_SvrEnable);
		m_SvrLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SvrLimit, true);
		m_SvrIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SvrIntervals, true);

		//KAD
		m_KadEnable.CVDC(1, NeoPrefs.PartPrefs.m_KadEnable);
		m_KadLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_KadLimit, true);
		m_KadIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_KadIntervals, true);
		m_KadMaxFiles.DV(FCFG_STD, NeoPrefs.PartPrefs.m_KadMaxFiles, true);
		m_KadRepeatDelay.DV(FCFG_STD, NeoPrefs.PartPrefs.m_KadRepeatDelay, true);

		//UDP
		m_UdpEnable.CVDC(1, NeoPrefs.PartPrefs.m_UdpEnable);
		m_UdpLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_UdpLimit, true);
		m_UdpIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_UdpIntervals, true);
		m_UdpGlobalIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_UdpGlobalIntervals, true);
		m_UdpFilesPerServer.DV(FCFG_STD, NeoPrefs.PartPrefs.m_UdpFilesPerServer, true);
		// NEO: SRT END

		// NEO: XSC - [ExtremeSourceCache]
		m_UseSourceCache.CVDC(1, NeoPrefs.PartPrefs.m_UseSourceCache);
		m_SourceCacheLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SourceCacheLimit, true);
		m_SourceCacheTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SourceCacheTime, true);
		// NEO: XSC END

		// NEO: ASL - [AutoSoftLock]
		m_AutoSoftLock.CVDC(1, NeoPrefs.PartPrefs.m_AutoSoftLock);
		m_AutoSoftLockLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_AutoSoftLockLimit, true);
		// NEO: ASL END

		// NEO: AHL - [AutoHardLimit]
		m_AutoHardLimit.CVDC(1, NeoPrefs.PartPrefs.m_AutoHardLimit);
		m_AutoHardLimitTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_AutoHardLimitTime, true);
		// NEO: AHL END

		// NEO: CSL - [CategorySourceLimit]
		m_CategorySourceLimit.CVDC(1, NeoPrefs.PartPrefs.m_CategorySourceLimit);
		m_CategorySourceLimitLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_CategorySourceLimitLimit, true);
		m_CategorySourceLimitTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_CategorySourceLimitTime, true);
		// NEO: CSL END

		// NEO: GSL - [GlobalSourceLimit]
		m_GlobalSourceLimit.CVDC(1, NeoPrefs.PartPrefs.m_GlobalSourceLimit);
		m_GlobalSourceLimitLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_GlobalSourceLimitLimit, true);
		m_GlobalSourceLimitTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_GlobalSourceLimitTime, true);
		// NEO: GSL END

		m_MinSourcePerFile.DV(FCFG_STD, NeoPrefs.PartPrefs.m_MinSourcePerFile, true);

		m_TCPConnectionRetry.DV(FCFG_STD, NeoPrefs.PartPrefs.m_TCPConnectionRetry, true); // NEO: TCR - [TCPConnectionRetry]

		// NEO: DRT - [DownloadReaskTweaks]
		m_SpreadReaskEnable = (UINT)NeoPrefs.PartPrefs.m_SpreadReaskEnable;
		m_SpreadReaskTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SpreadReaskTime, true);
		m_SourceReaskTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_SourceReaskTime, true);
		m_FullQSourceReaskTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_FullQSourceReaskTime, true);
		m_NNPSourceReaskTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_NNPSourceReaskTime, true);
		// NEO: DRT END

		// NEO: SDT - [SourcesDropTweaks]
		m_DropTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_DropTime, true);

		//Bad
		m_BadSourceDrop.CVDC(1, NeoPrefs.PartPrefs.m_BadSourceDrop);
		m_BadSourceLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_BadSourceLimit, true);
		m_BadSourceLimitMode.CVDC(2, NeoPrefs.PartPrefs.m_BadSourceLimitMode);
		m_BadSourceDropTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_BadSourceDropTime, true);
		m_BadSourceDropMode.CVDC(2, NeoPrefs.PartPrefs.m_BadSourceDropMode);

		//NNP
		m_NNPSourceDrop.CVDC(2, NeoPrefs.PartPrefs.m_NNPSourceDrop);
		m_NNPSourceLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_NNPSourceLimit, true);
		m_NNPSourceLimitMode.CVDC(2, NeoPrefs.PartPrefs.m_NNPSourceLimitMode);
		m_NNPSourceDropTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_NNPSourceDropTime, true);
		m_NNPSourceDropMode.CVDC(2, NeoPrefs.PartPrefs.m_NNPSourceDropMode);

		//FullQ
		m_FullQSourceDrop.CVDC(2, NeoPrefs.PartPrefs.m_FullQSourceDrop);
		m_FullQSourceLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_FullQSourceLimit, true);
		m_FullQSourceLimitMode.CVDC(2, NeoPrefs.PartPrefs.m_FullQSourceLimitMode);
		m_FullQSourceDropTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_FullQSourceDropTime, true);
		m_FullQSourceDropMode.CVDC(2, NeoPrefs.PartPrefs.m_FullQSourceDropMode);

		//HighQ
		m_HighQSourceDrop.CVDC(2, NeoPrefs.PartPrefs.m_HighQSourceDrop);
		m_HighQSourceLimit.DV(FCFG_STD, NeoPrefs.PartPrefs.m_HighQSourceLimit, true);
		m_HighQSourceLimitMode.CVDC(2, NeoPrefs.PartPrefs.m_HighQSourceLimitMode);
		m_HighQSourceDropTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_HighQSourceDropTime, true);
		m_HighQSourceDropMode.CVDC(2, NeoPrefs.PartPrefs.m_HighQSourceDropMode);
		m_HighQSourceMaxRank.DV(FCFG_STD, NeoPrefs.PartPrefs.m_HighQSourceMaxRank, true);
		m_HighQSourceRankMode.CVDC(1, NeoPrefs.PartPrefs.m_HighQSourceRankMode);

		m_DeadTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_DeadTime, true);
		m_DeadTimeFWMulti.DV(FCFG_STD, NeoPrefs.PartPrefs.m_DeadTimeFWMulti, true);
		m_GlobalDeadTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_GlobalDeadTime, true);
		m_GlobalDeadTimeFWMulti.DV(FCFG_STD, NeoPrefs.PartPrefs.m_GlobalDeadTimeFWMulti, true);
		// NEO: SDT END
	}

	SetAuxSet(m_Category == NULL);
	SetAut();
}

void CPPgSources::SetAuxSet(bool bGlobal)
{
	// NEO: SDT - [SourcesDropTweaks]
	//Bad
	m_BadSourceLimit.aS(PrepAuxSet(m_BadSourceLimitMode.Val,bGlobal));
	m_BadSourceDropTime.aS(PrepAuxSet(m_BadSourceDropMode.Val,bGlobal));

	//NNP
	m_NNPSourceLimit.aS(PrepAuxSet(m_NNPSourceLimitMode.Val,bGlobal));
	m_NNPSourceDropTime.aS(PrepAuxSet(m_NNPSourceDropMode.Val,bGlobal));

	//FullQ
	m_FullQSourceLimit.aS(PrepAuxSet(m_FullQSourceLimitMode.Val,bGlobal));
	m_FullQSourceDropTime.aS(PrepAuxSet(m_FullQSourceDropMode.Val,bGlobal));

	//HighQ
	m_HighQSourceLimit.aS(PrepAuxSet(m_HighQSourceLimitMode.Val,bGlobal));
	m_HighQSourceDropTime.aS(PrepAuxSet(m_HighQSourceDropMode.Val,bGlobal));
	m_HighQSourceMaxRank.aS(PrepAuxSet(m_HighQSourceRankMode.Val,bGlobal));
	// NEO: SDT END
}

void CPPgSources::SetAutVal(SintD &Val, int Ref, int Fac, int Set)
{
	if(Ref == FCFG_UNK){
		Val.A(FCFG_UNK,Set);
	}else{
		int Lim = Ref * Fac / 10;
		MinMax(&Lim,Val.Min[Set],Val.Max[Set]);
		Val.A(Lim,Set);
	}
}

void CPPgSources::SetAut()
{
	int SourceLimit = GetRightDef(m_SourceLimit);

	// NEO: SRT - [SourceRequestTweaks]
	SetAutVal(m_SwapLimit,SourceLimit, 8);

	SetAutVal(m_XsLimit,SourceLimit, 9);
	SetAutVal(m_SvrLimit,SourceLimit, 9);
	SetAutVal(m_KadLimit,SourceLimit, 7);
	SetAutVal(m_UdpLimit,SourceLimit, 7);
	// NEO: SRT END

	SetAutVal(m_SourceCacheLimit,SourceLimit, 1); // NEO: XSC - [ExtremeSourceCache]
	
	// NEO: SDT - [SourcesDropTweaks]
	SetAutVal(m_BadSourceLimit,SourceLimit, 8);
	SetAutVal(m_BadSourceLimit,SourceLimit, 2, 2);
	SetAutVal(m_BadSourceDropTime,SourceLimit, 8);
	SetAutVal(m_BadSourceDropTime,SourceLimit, 2, 2);
	SetAutVal(m_NNPSourceLimit,SourceLimit, 8);
	SetAutVal(m_NNPSourceLimit,SourceLimit, 2, 2);
	SetAutVal(m_NNPSourceDropTime,SourceLimit, 8);
	SetAutVal(m_NNPSourceDropTime,SourceLimit, 2, 2);
	SetAutVal(m_FullQSourceLimit,SourceLimit, 8);
	SetAutVal(m_FullQSourceLimit,SourceLimit, 2, 2);
	SetAutVal(m_FullQSourceDropTime,SourceLimit, 8);
	SetAutVal(m_FullQSourceDropTime,SourceLimit, 2, 2);
	SetAutVal(m_HighQSourceLimit,SourceLimit, 8);
	SetAutVal(m_HighQSourceLimit,SourceLimit, 2, 2);
	SetAutVal(m_HighQSourceDropTime,SourceLimit, 8);
	SetAutVal(m_HighQSourceDropTime,SourceLimit, 2, 2);
	// NEO: SDT END
}

// NEO: FCFG - [FileConfiguration]
void CPPgSources::RefreshData()
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

	// set the right default value
	// and the value if the file have an prefs object

	m_ForceA4AF = PartPrefs->m_ForceA4AF; // NEO: NXC - [NewExtendedCategories]

	// NEO: SRT - [SourceRequestTweaks]
	// General
	m_SourceLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SourceLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SourceLimit : FCFG_DEF);

	// Management
	m_SwapLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SwapLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SwapLimit : FCFG_DEF);

	//XS
	m_XsEnable.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_XsEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_XsEnable, Category && Category->PartPrefs ? Category->PartPrefs->m_XsEnable : -1);
	m_XsLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_XsLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_XsLimit : FCFG_DEF);
	m_XsIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_XsIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_XsIntervals : FCFG_DEF);
	m_XsClientIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_XsClientIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_XsClientIntervals : FCFG_DEF);
	m_XsCleintDelay.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_XsCleintDelay : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_XsCleintDelay : FCFG_DEF);
	m_XsRareLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_XsRareLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_XsRareLimit : FCFG_DEF);

	// SVR
	m_SvrEnable.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SvrEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_SvrEnable, Category && Category->PartPrefs ? Category->PartPrefs->m_SvrEnable : -1);
	m_SvrLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SvrLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SvrLimit : FCFG_DEF);
	m_SvrIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SvrIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SvrIntervals : FCFG_DEF);

	//KAD
	m_KadEnable.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_KadEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_KadEnable, Category && Category->PartPrefs ? Category->PartPrefs->m_KadEnable : -1);
	m_KadLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_KadLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_KadLimit : FCFG_DEF);
	m_KadIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_KadIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_KadIntervals : FCFG_DEF);
	m_KadMaxFiles.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_KadMaxFiles : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_KadMaxFiles : FCFG_DEF);
	m_KadRepeatDelay.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_KadRepeatDelay : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_KadRepeatDelay : FCFG_DEF);

	//UDP
	m_UdpEnable.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_UdpEnable : FCFG_DEF, NeoPrefs.PartPrefs.m_UdpEnable, Category && Category->PartPrefs ? Category->PartPrefs->m_UdpEnable : -1);
	m_UdpLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_UdpLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_UdpLimit : FCFG_DEF);
	m_UdpIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_UdpIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_UdpIntervals : FCFG_DEF);
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	m_UseSourceCache.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_UseSourceCache : FCFG_DEF, NeoPrefs.PartPrefs.m_UseSourceCache, Category && Category->PartPrefs ? Category->PartPrefs->m_UseSourceCache : -1);
	m_SourceCacheLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SourceCacheLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SourceCacheLimit : FCFG_DEF);
	m_SourceCacheTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SourceCacheTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SourceCacheTime : FCFG_DEF);
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	m_AutoSoftLock.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoSoftLock : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoSoftLock, Category && Category->PartPrefs ? Category->PartPrefs->m_AutoSoftLock : -1);
	m_AutoSoftLockLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_AutoSoftLockLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoSoftLockLimit : FCFG_DEF);
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	m_AutoHardLimit.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoHardLimit : FCFG_DEF, NeoPrefs.PartPrefs.m_AutoHardLimit, Category && Category->PartPrefs ? Category->PartPrefs->m_AutoHardLimit : -1);
	m_AutoHardLimitTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_AutoHardLimitTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_AutoHardLimitTime : FCFG_DEF);
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	m_CategorySourceLimit.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_CategorySourceLimit : FCFG_DEF, NeoPrefs.PartPrefs.m_CategorySourceLimit, Category && Category->PartPrefs ? Category->PartPrefs->m_CategorySourceLimit : -1);
	m_CategorySourceLimitLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_CategorySourceLimitLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_CategorySourceLimitLimit : FCFG_DEF);
	m_CategorySourceLimitTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_CategorySourceLimitTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_CategorySourceLimitTime : FCFG_DEF);
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	m_GlobalSourceLimit.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_GlobalSourceLimit : FCFG_DEF, NeoPrefs.PartPrefs.m_GlobalSourceLimit, Category && Category->PartPrefs ? Category->PartPrefs->m_GlobalSourceLimit : -1);
	m_GlobalSourceLimitLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_GlobalSourceLimitLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_GlobalSourceLimitLimit : FCFG_DEF);
	m_GlobalSourceLimitTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_GlobalSourceLimitTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_GlobalSourceLimitTime : FCFG_DEF);
	// NEO: GSL END

	m_MinSourcePerFile.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_MinSourcePerFile : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_MinSourcePerFile : FCFG_DEF);

	m_TCPConnectionRetry.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_TCPConnectionRetry : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_TCPConnectionRetry : FCFG_DEF); // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	m_SourceReaskTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_SourceReaskTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_SourceReaskTime : FCFG_DEF);
	m_FullQSourceReaskTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_FullQSourceReaskTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_FullQSourceReaskTime : FCFG_DEF);
	m_NNPSourceReaskTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_NNPSourceReaskTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_NNPSourceReaskTime : FCFG_DEF);
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	m_DropTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_DropTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_DropTime : FCFG_DEF);

	//Bad
	m_BadSourceDrop.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_BadSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_BadSourceDrop, Category && Category->PartPrefs ? Category->PartPrefs->m_BadSourceDrop : -1);
	m_BadSourceLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_BadSourceLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_BadSourceLimit : FCFG_DEF);
	m_BadSourceLimitMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_BadSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_BadSourceLimitMode, Category && Category->PartPrefs ? Category->PartPrefs->m_BadSourceLimitMode : -1);
	m_BadSourceDropTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_BadSourceDropTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_BadSourceDropTime : FCFG_DEF);
	m_BadSourceDropMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_BadSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_BadSourceDropMode, Category && Category->PartPrefs ? Category->PartPrefs->m_BadSourceDropMode : -1);

	//NNP
	m_NNPSourceDrop.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_NNPSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_NNPSourceDrop, Category && Category->PartPrefs ? Category->PartPrefs->m_NNPSourceDrop : -1);
	m_NNPSourceLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_NNPSourceLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_NNPSourceLimit : FCFG_DEF);
	m_NNPSourceLimitMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_NNPSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_NNPSourceLimitMode, Category && Category->PartPrefs ? Category->PartPrefs->m_NNPSourceLimitMode : -1);
	m_NNPSourceDropTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_NNPSourceDropTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_NNPSourceDropTime : FCFG_DEF);
	m_NNPSourceDropMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_NNPSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_NNPSourceDropMode, Category && Category->PartPrefs ? Category->PartPrefs->m_NNPSourceDropMode : -1);

	//FullQ
	m_FullQSourceDrop.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_FullQSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_FullQSourceDrop, Category && Category->PartPrefs ? Category->PartPrefs->m_FullQSourceDrop : -1);
	m_FullQSourceLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_FullQSourceLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_FullQSourceLimit : FCFG_DEF);
	m_FullQSourceLimitMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_FullQSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_FullQSourceLimitMode, Category && Category->PartPrefs ? Category->PartPrefs->m_FullQSourceLimitMode : -1);
	m_FullQSourceDropTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_FullQSourceDropTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_FullQSourceDropTime : FCFG_DEF);
	m_FullQSourceDropMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_FullQSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_FullQSourceDropMode, Category && Category->PartPrefs ? Category->PartPrefs->m_FullQSourceDropMode : -1);

	//HighQ
	m_HighQSourceDrop.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceDrop : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceDrop, Category && Category->PartPrefs ? Category->PartPrefs->m_HighQSourceDrop : -1);
	m_HighQSourceLimit.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_HighQSourceLimit : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceLimit : FCFG_DEF);
	m_HighQSourceLimitMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceLimitMode : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceLimitMode, Category && Category->PartPrefs ? Category->PartPrefs->m_HighQSourceLimitMode : -1);
	m_HighQSourceDropTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_HighQSourceDropTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceDropTime : FCFG_DEF);
	m_HighQSourceDropMode.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceDropMode : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceDropMode, Category && Category->PartPrefs ? Category->PartPrefs->m_HighQSourceDropMode : -1);
	m_HighQSourceMaxRank.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_HighQSourceMaxRank : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceMaxRank : FCFG_DEF);
	m_HighQSourceRankMode.CVDC(1, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_HighQSourceRankMode : FCFG_DEF, NeoPrefs.PartPrefs.m_HighQSourceRankMode, Category && Category->PartPrefs ? Category->PartPrefs->m_HighQSourceRankMode : -1);

	m_DeadTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_DeadTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_DeadTime : FCFG_DEF);
	m_DeadTimeFWMulti.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_DeadTimeFWMulti : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_DeadTimeFWMulti : FCFG_DEF);
	// NEO: SDT END

	for (int i = 1; i < m_paFiles->GetSize(); i++)
	{
		if(!theApp.downloadqueue->IsPartFile((CPartFile*)(*m_paFiles)[i]))
			continue;

		PartFile = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[i]);

		if(Category && Category != thePrefs.GetCategory(PartFile->GetCategory()))
		{
			// we have to change the .def to FCFG_UNK becouse we have files form different categories
			// we hat to set .cat to -2 to inform that no cat defaults is available thow cats settings are present
		
			// NEO: SRT - [SourceRequestTweaks]
			// General
			m_SourceLimit.Def = FCFG_UNK;

			// Management
			m_SwapLimit.Def = FCFG_UNK;

			//XS
			m_XsEnable.Cat = -2;
			m_XsLimit.Def = FCFG_UNK;
			m_XsIntervals.Def = FCFG_UNK;
			m_XsClientIntervals.Def = FCFG_UNK;
			m_XsCleintDelay.Def = FCFG_UNK;
			m_XsRareLimit.Def = FCFG_UNK;

			// SVR
			m_SvrEnable.Cat = -2;
			m_SvrLimit.Def = FCFG_UNK;
			m_SvrIntervals.Def = FCFG_UNK;

			//KAD
			m_KadEnable.Cat = -2;
			m_KadLimit.Def = FCFG_UNK;
			m_KadIntervals.Def = FCFG_UNK;
			m_KadMaxFiles.Def = FCFG_UNK;
			m_KadRepeatDelay.Def = FCFG_UNK;

			//UDP
			m_UdpEnable.Cat = -2;
			m_UdpLimit.Def = FCFG_UNK;
			m_UdpIntervals.Def = FCFG_UNK;
			// NEO: SRT END

			// NEO: XSC - [ExtremeSourceCache]
			m_UseSourceCache.Cat = -2;
			m_SourceCacheLimit.Def = FCFG_UNK;
			m_SourceCacheTime.Def = FCFG_UNK;
			// NEO: XSC END

			// NEO: ASL - [AutoSoftLock]
			m_AutoSoftLock.Cat = -2;
			m_AutoSoftLockLimit.Def = FCFG_UNK;
			// NEO: ASL END

			// NEO: AHL - [AutoHardLimit]
			m_AutoHardLimit.Cat = -2;
			m_AutoHardLimitTime.Def = FCFG_UNK;
			// NEO: AHL END

			// NEO: CSL - [CategorySourceLimit]
			m_CategorySourceLimit.Cat = -2;
			m_CategorySourceLimitLimit.Def = FCFG_UNK;
			m_CategorySourceLimitTime.Def = FCFG_UNK;
			// NEO: CSL END

			// NEO: GSL - [GlobalSourceLimit]
			m_GlobalSourceLimit.Cat = -2;
			m_GlobalSourceLimitLimit.Def = FCFG_UNK;
			m_GlobalSourceLimitTime.Def = FCFG_UNK;
			// NEO: GSL END

			m_MinSourcePerFile.Def = FCFG_UNK;

			m_TCPConnectionRetry.Def = FCFG_UNK; // NEO: TCR - [TCPConnectionRetry]

			// NEO: DRT - [DownloadReaskTweaks]
			m_SourceReaskTime.Def = FCFG_UNK;
			m_FullQSourceReaskTime.Def = FCFG_UNK;
			m_NNPSourceReaskTime.Def = FCFG_UNK;
			// NEO: DRT END

			// NEO: SDT - [SourcesDropTweaks]
			m_DropTime.Def = FCFG_UNK;

			//Bad
			m_BadSourceDrop.Cat = -2;
			m_BadSourceLimit.Def = FCFG_UNK;
			m_BadSourceLimitMode.Cat = -2;
			m_BadSourceDropTime.Def = FCFG_UNK;
			m_BadSourceDropMode.Cat = -2;

			//NNP
			m_NNPSourceDrop.Cat = -2;
			m_NNPSourceLimit.Def = FCFG_UNK;
			m_NNPSourceLimitMode.Cat = -2;
			m_NNPSourceDropTime.Def = FCFG_UNK;
			m_NNPSourceDropMode.Cat = -2;

			//FullQ
			m_FullQSourceDrop.Cat = -2;
			m_FullQSourceLimit.Def = FCFG_UNK;
			m_FullQSourceLimitMode.Cat = -2;
			m_FullQSourceDropTime.Def = FCFG_UNK;
			m_FullQSourceDropMode.Cat = -2;

			//HighQ
			m_HighQSourceDrop.Cat = -2;
			m_HighQSourceLimit.Def = FCFG_UNK;
			m_HighQSourceLimitMode.Cat = -2;
			m_HighQSourceDropTime.Def = FCFG_UNK;
			m_HighQSourceDropMode.Cat = -2;
			m_HighQSourceMaxRank.Def = FCFG_UNK;
			m_HighQSourceRankMode.Cat = -2;

			m_DeadTime.Def = FCFG_UNK;
			m_DeadTimeFWMulti.Def = FCFG_UNK;
			// NEO: SDT END

			Category = NULL;
		}

		if(m_ForceA4AF != PartPrefs->m_ForceA4AF) m_ForceA4AF = -1; // NEO: NXC - [NewExtendedCategories]

		// we have to change the .Val to FCFG_UNK becouse we have different values
		// NEO: SRT - [SourceRequestTweaks]
		// General
		if(m_SourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SourceLimit : FCFG_DEF)) m_SourceLimit.Val = FCFG_UNK;

		// Management
		if(m_SwapLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SwapLimit : FCFG_DEF)) m_SwapLimit.Val = FCFG_UNK;

		//XS
		if(m_XsEnable.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_XsEnable : FCFG_DEF)) m_XsEnable.Val = FCFG_UNK;
		if(m_XsLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_XsLimit : FCFG_DEF)) m_XsLimit.Val = FCFG_UNK;
		if(m_XsIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_XsIntervals : FCFG_DEF)) m_XsIntervals.Val = FCFG_UNK;
		if(m_XsClientIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_XsClientIntervals : FCFG_DEF)) m_XsClientIntervals.Val = FCFG_UNK;
		if(m_XsCleintDelay.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_XsCleintDelay : FCFG_DEF)) m_XsCleintDelay.Val = FCFG_UNK;
		if(m_XsRareLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_XsRareLimit : FCFG_DEF)) m_XsRareLimit.Val = FCFG_UNK;

		// SVR
		if(m_SvrEnable.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SvrEnable : FCFG_DEF)) m_SvrEnable.Val = FCFG_UNK;
		if(m_SvrLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SvrLimit : FCFG_DEF)) m_SvrLimit.Val = FCFG_UNK;
		if(m_SvrIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SvrIntervals : FCFG_DEF)) m_SvrIntervals.Val = FCFG_UNK;

		//KAD
		if(m_KadEnable.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_KadEnable : FCFG_DEF)) m_KadEnable.Val = FCFG_UNK;
		if(m_KadLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_KadLimit : FCFG_DEF)) m_KadLimit.Val = FCFG_UNK;
		if(m_KadIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_KadIntervals : FCFG_DEF)) m_KadIntervals.Val = FCFG_UNK;
		if(m_KadMaxFiles.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_KadMaxFiles : FCFG_DEF)) m_KadMaxFiles.Val = FCFG_UNK;
		if(m_KadRepeatDelay.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_KadRepeatDelay : FCFG_DEF)) m_KadRepeatDelay.Val = FCFG_UNK;

		//UDP
		if(m_UdpEnable.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_UdpEnable : FCFG_DEF)) m_UdpEnable.Val = FCFG_UNK;
		if(m_UdpLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_UdpLimit : FCFG_DEF)) m_UdpLimit.Val = FCFG_UNK;
		if(m_UdpIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_UdpIntervals : FCFG_DEF)) m_UdpIntervals.Val = FCFG_UNK;
		// NEO: SRT END

		// NEO: XSC - [ExtremeSourceCache]
		if(m_UseSourceCache.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_UseSourceCache : FCFG_DEF)) m_UseSourceCache.Val = FCFG_UNK;
		if(m_SourceCacheLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SourceCacheLimit : FCFG_DEF)) m_SourceCacheLimit.Val = FCFG_UNK;
		if(m_SourceCacheTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SourceCacheTime : FCFG_DEF)) m_SourceCacheTime.Val = FCFG_UNK;
		// NEO: XSC END

		// NEO: ASL - [AutoSoftLock]
		if(m_AutoSoftLock.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoSoftLock : FCFG_DEF)) m_AutoSoftLock.Val = FCFG_UNK;
		if(m_AutoSoftLockLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoSoftLockLimit : FCFG_DEF)) m_AutoSoftLockLimit.Val = FCFG_UNK;
		// NEO: ASL END

		// NEO: AHL - [AutoHardLimit]
		if(m_AutoHardLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoHardLimit : FCFG_DEF)) m_AutoHardLimit.Val = FCFG_UNK;
		if(m_AutoHardLimitTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_AutoHardLimitTime : FCFG_DEF)) m_AutoHardLimitTime.Val = FCFG_UNK;
		// NEO: AHL END

		// NEO: CSL - [CategorySourceLimit]
		if(m_CategorySourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_CategorySourceLimit : FCFG_DEF)) m_CategorySourceLimit.Val = FCFG_UNK;
		if(m_CategorySourceLimitLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_CategorySourceLimitLimit : FCFG_DEF)) m_CategorySourceLimitLimit.Val = FCFG_UNK;
		if(m_CategorySourceLimitTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_CategorySourceLimitTime : FCFG_DEF)) m_CategorySourceLimitTime.Val = FCFG_UNK;
		// NEO: CSL END

		// NEO: GSL - [GlobalSourceLimit]
		if(m_GlobalSourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_GlobalSourceLimit : FCFG_DEF)) m_GlobalSourceLimit.Val = FCFG_UNK;
		if(m_GlobalSourceLimitLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_GlobalSourceLimitLimit : FCFG_DEF)) m_GlobalSourceLimitLimit.Val = FCFG_UNK;
		if(m_GlobalSourceLimitTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_GlobalSourceLimitTime : FCFG_DEF)) m_GlobalSourceLimitTime.Val = FCFG_UNK;
		// NEO: GSL END

		if(m_MinSourcePerFile.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_MinSourcePerFile : FCFG_DEF)) m_MinSourcePerFile.Val = FCFG_UNK;

		if(m_TCPConnectionRetry.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_TCPConnectionRetry : FCFG_DEF)) m_TCPConnectionRetry.Val = FCFG_UNK; // NEO: TCR - [TCPConnectionRetry]

		// NEO: DRT - [DownloadReaskTweaks]
		if(m_SourceReaskTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_SourceReaskTime : FCFG_DEF)) m_SourceReaskTime.Val = FCFG_UNK;
		if(m_FullQSourceReaskTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_FullQSourceReaskTime : FCFG_DEF)) m_FullQSourceReaskTime.Val = FCFG_UNK;
		if(m_NNPSourceReaskTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_NNPSourceReaskTime : FCFG_DEF)) m_NNPSourceReaskTime.Val = FCFG_UNK;
		// NEO: DRT END

		// NEO: SDT - [SourcesDropTweaks]
		if(m_DropTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_DropTime : FCFG_DEF)) m_DropTime.Val = FCFG_UNK;

		//Bad
		if(m_BadSourceDrop.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_BadSourceDrop : FCFG_DEF)) m_BadSourceDrop.Val = FCFG_UNK;
		if(m_BadSourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_BadSourceLimit : FCFG_DEF)) m_BadSourceLimit.Val = FCFG_UNK;
		if(m_BadSourceLimitMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_BadSourceLimitMode : FCFG_DEF)) m_BadSourceLimitMode.Val = FCFG_UNK;
		if(m_BadSourceDropTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_BadSourceDropTime : FCFG_DEF)) m_BadSourceDropTime.Val = FCFG_UNK;
		if(m_BadSourceDropMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_BadSourceDropMode : FCFG_DEF)) m_BadSourceDropMode.Val = FCFG_UNK;

		//NNP
		if(m_NNPSourceDrop.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_NNPSourceDrop : FCFG_DEF)) m_NNPSourceDrop.Val = FCFG_UNK;
		if(m_NNPSourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_NNPSourceLimit : FCFG_DEF)) m_NNPSourceLimit.Val = FCFG_UNK;
		if(m_NNPSourceLimitMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_NNPSourceLimitMode : FCFG_DEF)) m_NNPSourceLimitMode.Val = FCFG_UNK;
		if(m_NNPSourceDropTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_NNPSourceDropTime : FCFG_DEF)) m_NNPSourceDropTime.Val = FCFG_UNK;
		if(m_NNPSourceDropMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_NNPSourceDropMode : FCFG_DEF)) m_NNPSourceDropMode.Val = FCFG_UNK;

		//FullQ
		if(m_FullQSourceDrop.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_FullQSourceDrop : FCFG_DEF)) m_FullQSourceDrop.Val = FCFG_UNK;
		if(m_FullQSourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_FullQSourceLimit : FCFG_DEF)) m_FullQSourceLimit.Val = FCFG_UNK;
		if(m_FullQSourceLimitMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_FullQSourceLimitMode : FCFG_DEF)) m_FullQSourceLimitMode.Val = FCFG_UNK;
		if(m_FullQSourceDropTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_FullQSourceLimitMode : FCFG_DEF)) m_FullQSourceLimitMode.Val = FCFG_UNK;
		if(m_FullQSourceDropMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_FullQSourceDropTime : FCFG_DEF)) m_FullQSourceDropTime.Val = FCFG_UNK;

		//HighQ
		if(m_HighQSourceDrop.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceDrop : FCFG_DEF)) m_HighQSourceDrop.Val = FCFG_UNK;
		if(m_HighQSourceLimit.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceLimit : FCFG_DEF)) m_HighQSourceLimit.Val = FCFG_UNK;
		if(m_HighQSourceLimitMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceLimitMode : FCFG_DEF)) m_HighQSourceLimitMode.Val = FCFG_UNK;
		if(m_HighQSourceDropTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceLimitMode : FCFG_DEF)) m_HighQSourceLimitMode.Val = FCFG_UNK;
		if(m_HighQSourceDropMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceDropMode : FCFG_DEF)) m_HighQSourceDropMode.Val = FCFG_UNK;
		if(m_HighQSourceMaxRank.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceDropMode : FCFG_DEF)) m_HighQSourceDropMode.Val = FCFG_UNK;
		if(m_HighQSourceRankMode.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_HighQSourceRankMode : FCFG_DEF)) m_HighQSourceRankMode.Val = FCFG_UNK;

		if(m_DeadTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_DeadTime : FCFG_DEF)) m_DeadTime.Val = FCFG_UNK;
		if(m_DeadTimeFWMulti.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_DeadTimeFWMulti : FCFG_DEF)) m_DeadTimeFWMulti.Val = FCFG_UNK;
		// NEO: SDT END
	}

	SetAuxSet(false);
	SetAut();

	UpdateData(FALSE);
}

void CPPgSources::GetFilePreferences(CKnownPreferences* /*KnownPrefs*/, CPartPreferences* PartPrefs, bool OperateData)
{
	if(OperateData)
		UpdateData();

	if(PartPrefs)
	{
		if(m_ForceA4AF != -1) PartPrefs->m_ForceA4AF = m_ForceA4AF;

		// NEO: SRT - [SourceRequestTweaks]
		// General
		if(m_SourceLimit.Val != FCFG_UNK) PartPrefs->m_SourceLimit = m_SourceLimit.Val;

		// Management
		if(m_SwapLimit.Val != FCFG_UNK) PartPrefs->m_SwapLimit = m_SwapLimit.Val;

		//XS
		if(m_XsEnable.Val != FCFG_UNK) PartPrefs->m_XsEnable = m_XsEnable.Val;
		if(m_XsLimit.Val != FCFG_UNK) PartPrefs->m_XsLimit = m_XsLimit.Val;
		if(m_XsIntervals.Val != FCFG_UNK) PartPrefs->m_XsIntervals = m_XsIntervals.Val;
		if(m_XsClientIntervals.Val != FCFG_UNK) PartPrefs->m_XsClientIntervals = m_XsClientIntervals.Val;
		if(m_XsCleintDelay.Val != FCFG_UNK) PartPrefs->m_XsCleintDelay = m_XsCleintDelay.Val;
		if(m_XsRareLimit.Val != FCFG_UNK) PartPrefs->m_XsRareLimit = m_XsRareLimit.Val;

		// SVR
		if(m_SvrEnable.Val != FCFG_UNK) PartPrefs->m_SvrEnable = m_SvrEnable.Val;
		if(m_SvrLimit.Val != FCFG_UNK) PartPrefs->m_SvrLimit = m_SvrLimit.Val;
		if(m_SvrIntervals.Val != FCFG_UNK) PartPrefs->m_SvrIntervals = m_SvrIntervals.Val;

		//KAD
		if(m_KadEnable.Val != FCFG_UNK) PartPrefs->m_KadEnable = m_KadEnable.Val;
		if(m_KadLimit.Val != FCFG_UNK) PartPrefs->m_KadLimit = m_KadLimit.Val;
		if(m_KadIntervals.Val != FCFG_UNK) PartPrefs->m_KadIntervals = m_KadIntervals.Val;
		if(m_KadMaxFiles.Val != FCFG_UNK) PartPrefs->m_KadMaxFiles = m_KadMaxFiles.Val;
		if(m_KadRepeatDelay.Val != FCFG_UNK) PartPrefs->m_KadRepeatDelay = m_KadRepeatDelay.Val;

		//UDP
		if(m_UdpEnable.Val != FCFG_UNK) PartPrefs->m_UdpEnable = m_UdpEnable.Val;
		if(m_UdpLimit.Val != FCFG_UNK) PartPrefs->m_UdpLimit = m_UdpLimit.Val;
		if(m_UdpIntervals.Val != FCFG_UNK) PartPrefs->m_UdpIntervals = m_UdpIntervals.Val;
		if(m_Category == NULL && m_paFiles == NULL){
			if(m_UdpGlobalIntervals.Val != FCFG_UNK) PartPrefs->m_UdpGlobalIntervals = m_UdpGlobalIntervals.Val;
			if(m_UdpFilesPerServer.Val != FCFG_UNK) PartPrefs->m_UdpFilesPerServer = m_UdpFilesPerServer.Val;
		}
		// NEO: SRT END

		// NEO: XSC - [ExtremeSourceCache]
		if(m_UseSourceCache.Val != FCFG_UNK) PartPrefs->m_UseSourceCache = m_UseSourceCache.Val;
		if(m_SourceCacheLimit.Val != FCFG_UNK) PartPrefs->m_SourceCacheLimit = m_SourceCacheLimit.Val;
		if(m_SourceCacheTime.Val != FCFG_UNK) PartPrefs->m_SourceCacheTime = m_SourceCacheTime.Val;
		// NEO: XSC END

		// NEO: ASL - [AutoSoftLock]
		if(m_AutoSoftLock.Val != FCFG_UNK) PartPrefs->m_AutoSoftLock = m_AutoSoftLock.Val;
		if(m_AutoSoftLockLimit.Val != FCFG_UNK) PartPrefs->m_AutoSoftLockLimit = m_AutoSoftLockLimit.Val;
		// NEO: ASL END

		// NEO: AHL - [AutoHardLimit]
		if(m_AutoHardLimit.Val != FCFG_UNK) PartPrefs->m_AutoHardLimit = m_AutoHardLimit.Val;
		if(m_AutoHardLimitTime.Val != FCFG_UNK) PartPrefs->m_AutoHardLimitTime = m_AutoHardLimitTime.Val;
		// NEO: AHL END

		// NEO: CSL - [CategorySourceLimit]
		if(m_CategorySourceLimit.Val != FCFG_UNK) PartPrefs->m_CategorySourceLimit = m_CategorySourceLimit.Val;
		if(m_CategorySourceLimitLimit.Val != FCFG_UNK) PartPrefs->m_CategorySourceLimitLimit = m_CategorySourceLimitLimit.Val;
		if(m_CategorySourceLimitTime.Val != FCFG_UNK) PartPrefs->m_CategorySourceLimitTime = m_CategorySourceLimitTime.Val;
		// NEO: CSL END

		// NEO: GSL - [GlobalSourceLimit]
		if(m_GlobalSourceLimit.Val != FCFG_UNK) PartPrefs->m_GlobalSourceLimit = m_GlobalSourceLimit.Val;
		if(m_GlobalSourceLimitLimit.Val != FCFG_UNK) PartPrefs->m_GlobalSourceLimitLimit = m_GlobalSourceLimitLimit.Val;
		if(m_GlobalSourceLimitTime.Val != FCFG_UNK) PartPrefs->m_GlobalSourceLimitTime = m_GlobalSourceLimitTime.Val;
		// NEO: GSL END

		if(m_MinSourcePerFile.Val != FCFG_UNK) PartPrefs->m_MinSourcePerFile = m_MinSourcePerFile.Val;

		if(m_TCPConnectionRetry.Val != FCFG_UNK) PartPrefs->m_TCPConnectionRetry = m_TCPConnectionRetry.Val; // NEO: TCR - [TCPConnectionRetry]

		// NEO: DRT - [DownloadReaskTweaks]
		if(m_Category == NULL && m_paFiles == NULL){
			if(m_SpreadReaskEnable != FCFG_UNK) PartPrefs->m_SpreadReaskEnable = m_SpreadReaskEnable;
			if(m_SpreadReaskTime.Val != FCFG_UNK) PartPrefs->m_SpreadReaskTime = m_SpreadReaskTime.Val;
		}
		if(m_SourceReaskTime.Val != FCFG_UNK) PartPrefs->m_SourceReaskTime = m_SourceReaskTime.Val;
		if(m_FullQSourceReaskTime.Val != FCFG_UNK) PartPrefs->m_FullQSourceReaskTime = m_FullQSourceReaskTime.Val;
		if(m_NNPSourceReaskTime.Val != FCFG_UNK) PartPrefs->m_NNPSourceReaskTime = m_NNPSourceReaskTime.Val;
		// NEO: DRT END

		// NEO: SDT - [SourcesDropTweaks]
		if(m_DropTime.Val != FCFG_UNK) PartPrefs->m_DropTime = m_DropTime.Val;

		//Bad
		if(m_BadSourceDrop.Val != FCFG_UNK) PartPrefs->m_BadSourceDrop = m_BadSourceDrop.Val;
		if(m_BadSourceLimit.Val != FCFG_UNK) PartPrefs->m_BadSourceLimit = m_BadSourceLimit.Val;
		if(m_BadSourceLimitMode.Val != FCFG_UNK) PartPrefs->m_BadSourceLimitMode = m_BadSourceLimitMode.Val;
		if(m_BadSourceDropTime.Val != FCFG_UNK) PartPrefs->m_BadSourceDropTime = m_BadSourceDropTime.Val;
		if(m_BadSourceDropMode.Val != FCFG_UNK) PartPrefs->m_BadSourceDropMode = m_BadSourceDropMode.Val;

		//NNP
		if(m_NNPSourceDrop.Val != FCFG_UNK) PartPrefs->m_NNPSourceDrop = m_NNPSourceDrop.Val;
		if(m_NNPSourceLimit.Val != FCFG_UNK) PartPrefs->m_NNPSourceLimit = m_NNPSourceLimit.Val;
		if(m_NNPSourceLimitMode.Val != FCFG_UNK) PartPrefs->m_NNPSourceLimitMode = m_NNPSourceLimitMode.Val;
		if(m_NNPSourceDropTime.Val != FCFG_UNK) PartPrefs->m_NNPSourceDropTime = m_NNPSourceDropTime.Val;
		if(m_NNPSourceDropMode.Val != FCFG_UNK) PartPrefs->m_NNPSourceDropMode = m_NNPSourceDropMode.Val;

		//FullQ
		if(m_FullQSourceDrop.Val != FCFG_UNK) PartPrefs->m_FullQSourceDrop = m_FullQSourceDrop.Val;
		if(m_FullQSourceLimit.Val != FCFG_UNK) PartPrefs->m_FullQSourceLimit = m_FullQSourceLimit.Val;
		if(m_FullQSourceLimitMode.Val != FCFG_UNK) PartPrefs->m_FullQSourceLimitMode = m_FullQSourceLimitMode.Val;
		if(m_FullQSourceDropTime.Val != FCFG_UNK) PartPrefs->m_FullQSourceDropTime = m_FullQSourceDropTime.Val;
		if(m_FullQSourceDropMode.Val != FCFG_UNK) PartPrefs->m_FullQSourceDropMode = m_FullQSourceDropMode.Val;

		//HighQ
		if(m_HighQSourceDrop.Val != FCFG_UNK) PartPrefs->m_HighQSourceDrop = m_HighQSourceDrop.Val;
		if(m_HighQSourceLimit.Val != FCFG_UNK) PartPrefs->m_HighQSourceLimit = m_HighQSourceLimit.Val;
		if(m_HighQSourceLimitMode.Val != FCFG_UNK) PartPrefs->m_HighQSourceLimitMode = m_HighQSourceLimitMode.Val;
		if(m_HighQSourceDropTime.Val != FCFG_UNK) PartPrefs->m_HighQSourceDropTime = m_HighQSourceDropTime.Val;
		if(m_HighQSourceDropMode.Val != FCFG_UNK) PartPrefs->m_HighQSourceDropMode = m_HighQSourceDropMode.Val;
		if(m_HighQSourceMaxRank.Val != FCFG_UNK) PartPrefs->m_HighQSourceMaxRank = m_HighQSourceMaxRank.Val;
		if(m_HighQSourceRankMode.Val != FCFG_UNK) PartPrefs->m_HighQSourceRankMode = m_HighQSourceRankMode.Val;

		if(m_DeadTime.Val != FCFG_UNK) PartPrefs->m_DeadTime = m_DeadTime.Val;
		if(m_DeadTimeFWMulti.Val != FCFG_UNK) PartPrefs->m_DeadTimeFWMulti = m_DeadTimeFWMulti.Val;
		if(m_Category == NULL && m_paFiles == NULL){
			if(m_GlobalDeadTime.Val != FCFG_UNK) PartPrefs->m_GlobalDeadTime = m_GlobalDeadTime.Val;
			if(m_GlobalDeadTimeFWMulti.Val != FCFG_UNK) PartPrefs->m_GlobalDeadTimeFWMulti = m_GlobalDeadTimeFWMulti.Val;
		}
		// NEO: SDT END
	}
}

void CPPgSources::SetFilePreferences(CKnownPreferences* /*KnownPrefs*/, CPartPreferences* PartPrefs, bool OperateData)
{
	if(PartPrefs)
	{
		m_ForceA4AF = PartPrefs->m_ForceA4AF;

		// NEO: SRT - [SourceRequestTweaks]
		// General
		m_SourceLimit.Val = PartPrefs->m_SourceLimit;

		// Management
		m_SwapLimit.Val = PartPrefs->m_SwapLimit;

		//XS
		m_XsEnable.Val = PartPrefs->m_XsEnable;
		m_XsLimit.Val = PartPrefs->m_XsLimit;
		m_XsIntervals.Val = PartPrefs->m_XsIntervals;
		m_XsClientIntervals.Val = PartPrefs->m_XsClientIntervals;
		m_XsCleintDelay.Val = PartPrefs->m_XsCleintDelay;
		m_XsRareLimit.Val = PartPrefs->m_XsRareLimit;

		// SVR
		m_SvrEnable.Val = PartPrefs->m_SvrEnable;
		m_SvrLimit.Val = PartPrefs->m_SvrLimit;
		m_SvrIntervals.Val = PartPrefs->m_SvrIntervals;

		//KAD
		m_KadEnable.Val = PartPrefs->m_KadEnable;
		m_KadLimit.Val = PartPrefs->m_KadLimit;
		m_KadIntervals.Val = PartPrefs->m_KadIntervals;
		m_KadMaxFiles.Val = PartPrefs->m_KadMaxFiles;
		m_KadRepeatDelay.Val = PartPrefs->m_KadRepeatDelay;

		//UDP
		m_UdpEnable.Val = PartPrefs->m_UdpEnable;
		m_UdpLimit.Val = PartPrefs->m_UdpLimit;
		m_UdpIntervals.Val = PartPrefs->m_UdpIntervals;
		if(m_Category == NULL && m_paFiles == NULL){
			m_UdpGlobalIntervals.Val = PartPrefs->m_UdpGlobalIntervals;
			m_UdpFilesPerServer.Val = PartPrefs->m_UdpFilesPerServer;
		}
		// NEO: SRT END

		// NEO: XSC - [ExtremeSourceCache]
		m_UseSourceCache.Val = PartPrefs->m_UseSourceCache;
		m_SourceCacheLimit.Val = PartPrefs->m_SourceCacheLimit;
		m_SourceCacheTime.Val = PartPrefs->m_SourceCacheTime;
		// NEO: XSC END

		// NEO: ASL - [AutoSoftLock]
		m_AutoSoftLock.Val = PartPrefs->m_AutoSoftLock;
		m_AutoSoftLockLimit.Val = PartPrefs->m_AutoSoftLockLimit;
		// NEO: ASL END

		// NEO: AHL - [AutoHardLimit]
		m_AutoHardLimit.Val = PartPrefs->m_AutoHardLimit;
		m_AutoHardLimitTime.Val = PartPrefs->m_AutoHardLimitTime;
		// NEO: AHL END

		// NEO: CSL - [CategorySourceLimit]
		m_CategorySourceLimit.Val = PartPrefs->m_CategorySourceLimit;
		m_CategorySourceLimitLimit.Val = PartPrefs->m_CategorySourceLimitLimit;
		m_CategorySourceLimitTime.Val = PartPrefs->m_CategorySourceLimitTime;
		// NEO: CSL END

		// NEO: GSL - [GlobalSourceLimit]
		m_GlobalSourceLimit.Val = PartPrefs->m_GlobalSourceLimit;
		m_GlobalSourceLimitLimit.Val = PartPrefs->m_GlobalSourceLimitLimit;
		m_GlobalSourceLimitTime.Val = PartPrefs->m_GlobalSourceLimitTime;
		// NEO: GSL END

		m_MinSourcePerFile.Val = PartPrefs->m_MinSourcePerFile;

		m_TCPConnectionRetry.Val = PartPrefs->m_TCPConnectionRetry; // NEO: TCR - [TCPConnectionRetry]

		// NEO: DRT - [DownloadReaskTweaks]
		if(m_Category == NULL && m_paFiles == NULL){
			m_SpreadReaskEnable = (UINT)PartPrefs->m_SpreadReaskEnable;
			m_SpreadReaskTime.Val = PartPrefs->m_SpreadReaskTime;
		}
		m_SourceReaskTime.Val = PartPrefs->m_SourceReaskTime;
		m_FullQSourceReaskTime.Val = PartPrefs->m_FullQSourceReaskTime;
		m_NNPSourceReaskTime.Val = PartPrefs->m_NNPSourceReaskTime;
		// NEO: DRT END

		// NEO: SDT - [SourcesDropTweaks]
		m_DropTime.Val = PartPrefs->m_DropTime;

		//Bad
		m_BadSourceDrop.Val = PartPrefs->m_BadSourceDrop;
		m_BadSourceLimit.Val = PartPrefs->m_BadSourceLimit;
		m_BadSourceLimitMode.Val = PartPrefs->m_BadSourceLimitMode;
		m_BadSourceDropTime.Val = PartPrefs->m_BadSourceDropTime;
		m_BadSourceDropMode.Val = PartPrefs->m_BadSourceDropMode;

		//NNP
		m_NNPSourceDrop.Val = PartPrefs->m_NNPSourceDrop;
		m_NNPSourceLimit.Val = PartPrefs->m_NNPSourceLimit;
		m_NNPSourceLimitMode.Val = PartPrefs->m_NNPSourceLimitMode;
		m_NNPSourceDropTime.Val = PartPrefs->m_NNPSourceDropTime;
		m_NNPSourceDropMode.Val = PartPrefs->m_NNPSourceDropMode;

		//FullQ
		m_FullQSourceDrop.Val = PartPrefs->m_FullQSourceDrop;
		m_FullQSourceLimit.Val = PartPrefs->m_FullQSourceLimit;
		m_FullQSourceLimitMode.Val = PartPrefs->m_FullQSourceLimitMode;
		m_FullQSourceDropTime.Val = PartPrefs->m_FullQSourceDropTime;
		m_FullQSourceDropMode.Val = PartPrefs->m_FullQSourceDropMode;

		//HighQ
		m_HighQSourceDrop.Val = PartPrefs->m_HighQSourceDrop;
		m_HighQSourceLimit.Val = PartPrefs->m_HighQSourceLimit;
		m_HighQSourceLimitMode.Val = PartPrefs->m_HighQSourceLimitMode;
		m_HighQSourceDropTime.Val = PartPrefs->m_HighQSourceDropTime;
		m_HighQSourceDropMode.Val = PartPrefs->m_HighQSourceDropMode;
		m_HighQSourceMaxRank.Val = PartPrefs->m_HighQSourceMaxRank;
		m_HighQSourceRankMode.Val = PartPrefs->m_HighQSourceRankMode;

		m_DeadTime.Val = PartPrefs->m_DeadTime;
		m_DeadTimeFWMulti.Val = PartPrefs->m_DeadTimeFWMulti;
		if(m_Category == NULL && m_paFiles == NULL){
			m_GlobalDeadTime.Val = PartPrefs->m_GlobalDeadTime;
			m_GlobalDeadTimeFWMulti.Val = PartPrefs->m_GlobalDeadTimeFWMulti;
		}
		// NEO: SDT END
	}

	if(OperateData)
	{
		UpdateData(FALSE);
		SetModified();
	}
}
// NEO: FCFG END

BOOL CPPgSources::OnApply()
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
		// NEO: NXC - [NewExtendedCategories]
		NeoPrefs.m_bSmartA4AFSwapping = m_bSmartA4AFSwapping;
		NeoPrefs.m_iAdvancedA4AFMode = m_iAdvancedA4AFMode;
		// NEO: NXC END

		GetFilePreferences(NULL, &NeoPrefs.PartPrefs);

		NeoPrefs.PartPrefs.CheckTweaks();

		LoadSettings();

		theApp.scheduler->SaveOriginals();
	}


	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

// NEO: FCFG - [FileConfiguration]
void CPPgSources::OnTimer(UINT /*nIDEvent*/)
{
	if (m_bDataChanged)
	{
		if(m_paFiles)
			RefreshData();
		m_bDataChanged = false;
	}
}

BOOL CPPgSources::OnSetActive()
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

LRESULT CPPgSources::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}
// NEO: FCFG END

BOOL CPPgSources::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgSources::OnDestroy()
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

LRESULT CPPgSources::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){



		// NEO: SRT - [SourceRequestTweaks]
		// General
			if(m_htiSourceLimit && pton->hItem == m_htiSourceLimit){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceLimit, m_SourceLimit)) SetModified();
			}else

		// Management
				if(m_htiSwapLimit && pton->hItem == m_htiSwapLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSwapLimit, m_SwapLimit)) SetModified();
				}else

		//XS
				if(m_htiXsLimit && pton->hItem == m_htiXsLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiXsLimit, m_XsLimit)) SetModified();
				}else
				if(m_htiXsIntervals && pton->hItem == m_htiXsIntervals){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiXsIntervals, m_XsIntervals)) SetModified();
				}else
				if(m_htiXsClientIntervals && pton->hItem == m_htiXsClientIntervals){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiXsClientIntervals, m_XsClientIntervals)) SetModified();
				}else
				if(m_htiXsCleintDelay && pton->hItem == m_htiXsCleintDelay){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiXsCleintDelay, m_XsCleintDelay)) SetModified();
				}else
				if(m_htiXsRareLimit && pton->hItem == m_htiXsRareLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiXsRareLimit, m_XsRareLimit)) SetModified();
				}else

		// SVR
				if(m_htiSvrLimit && pton->hItem == m_htiSvrLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSvrLimit, m_SvrLimit)) SetModified();
				}else
				if(m_htiSvrIntervals && pton->hItem == m_htiSvrIntervals){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSvrIntervals, m_SvrIntervals)) SetModified();
				}else

		//KAD
				if(m_htiKadLimit && pton->hItem == m_htiKadLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiKadLimit, m_KadLimit)) SetModified();
				}else
				if(m_htiKadIntervals && pton->hItem == m_htiKadIntervals){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiKadIntervals, m_KadIntervals)) SetModified();
				}else
				if(m_htiKadMaxFiles && pton->hItem == m_htiKadMaxFiles){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiKadMaxFiles, m_KadMaxFiles)) SetModified();
				}else
				if(m_htiKadRepeatDelay && pton->hItem == m_htiKadRepeatDelay){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiKadRepeatDelay, m_KadRepeatDelay)) SetModified();
				}else

		//UDP
				if(m_htiUdpLimit && pton->hItem == m_htiUdpLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUdpLimit, m_UdpLimit)) SetModified();
				}else
				if(m_htiUdpIntervals && pton->hItem == m_htiUdpIntervals){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUdpIntervals, m_UdpIntervals)) SetModified();
				}else
				if(m_htiUdpGlobalIntervals && pton->hItem == m_htiUdpGlobalIntervals){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUdpGlobalIntervals, m_UdpGlobalIntervals)) SetModified();
				}else
				if(m_htiUdpFilesPerServer && pton->hItem == m_htiUdpFilesPerServer){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUdpFilesPerServer, m_UdpFilesPerServer)) SetModified();
				}else
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
				if(m_htiSourceCacheLimit && pton->hItem == m_htiSourceCacheLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceCacheLimit, m_SourceCacheLimit)) SetModified();
				}else
				if(m_htiSourceCacheTime && pton->hItem == m_htiSourceCacheTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceCacheTime, m_SourceCacheTime)) SetModified();
				}else
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
				if(m_htiAutoSoftLockLimit && pton->hItem == m_htiAutoSoftLockLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoSoftLockLimit, m_AutoSoftLockLimit)) SetModified();
				}else
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
				if(m_htiAutoHardLimitTime && pton->hItem == m_htiAutoHardLimitTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoHardLimitTime, m_AutoHardLimitTime)) SetModified();
				}else
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
				if(m_htiCategorySourceLimitLimit && pton->hItem == m_htiCategorySourceLimitLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiCategorySourceLimitLimit, m_CategorySourceLimitLimit)) SetModified();
				}else
				if(m_htiCategorySourceLimitTime && pton->hItem == m_htiCategorySourceLimitTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiCategorySourceLimitTime, m_CategorySourceLimitTime)) SetModified();
				}else
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
				if(m_htiGlobalSourceLimitLimit && pton->hItem == m_htiGlobalSourceLimitLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiGlobalSourceLimitLimit, m_GlobalSourceLimitLimit)) SetModified();
				}else
				if(m_htiGlobalSourceLimitTime && pton->hItem == m_htiGlobalSourceLimitTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiGlobalSourceLimitTime, m_GlobalSourceLimitTime)) SetModified();
				}else
	// NEO: GSL END
		
				if(m_htiMinSourcePerFile && pton->hItem == m_htiMinSourcePerFile){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMinSourcePerFile, m_MinSourcePerFile)) SetModified();
				}else

	// NEO: TCR - [TCPConnectionRetry]
				if(m_htiTCPConnectionRetry && pton->hItem == m_htiTCPConnectionRetry){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiTCPConnectionRetry, m_TCPConnectionRetry)) SetModified();
				}else
	// NEO: TCR END

	// NEO: DRT - [DownloadReaskTweaks]
				if(m_htiSpreadReask && pton->hItem == m_htiSpreadReask){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSpreadReask, m_SpreadReaskTime)) SetModified();
				}else
				if(m_htiSourceReaskTime && pton->hItem == m_htiSourceReaskTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceReaskTime, m_SourceReaskTime)) SetModified();
				}else
				if(m_htiFullQSourceReaskTime && pton->hItem == m_htiFullQSourceReaskTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFullQSourceReaskTime, m_FullQSourceReaskTime)) SetModified();
				}else
				if(m_htiNNPSourceReaskTime && pton->hItem == m_htiNNPSourceReaskTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiNNPSourceReaskTime, m_NNPSourceReaskTime)) SetModified();
				}else
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
			if(m_htiDropTime && pton->hItem == m_htiDropTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDropTime, m_DropTime)) SetModified();
			}else

		//Bad
				if(m_htiBadSourceLimit && pton->hItem == m_htiBadSourceLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBadSourceLimit, m_BadSourceLimit)) SetModified();
				}else
				if(m_htiBadSourceDropTime && pton->hItem == m_htiBadSourceDropTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBadSourceDropTime, m_BadSourceDropTime)) SetModified();
				}else

		//NNP
				if(m_htiNNPSourceLimit && pton->hItem == m_htiNNPSourceLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiNNPSourceLimit, m_NNPSourceLimit)) SetModified();
				}else
				if(m_htiNNPSourceDropTime && pton->hItem == m_htiNNPSourceDropTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiNNPSourceDropTime, m_NNPSourceDropTime)) SetModified();
				}else

		//FullQ
				if(m_htiFullQSourceLimit && pton->hItem == m_htiFullQSourceLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFullQSourceLimit, m_FullQSourceLimit)) SetModified();
				}else
				if(m_htiFullQSourceDropTime && pton->hItem == m_htiFullQSourceDropTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFullQSourceDropTime, m_FullQSourceDropTime)) SetModified();
				}else

		//HighQ
				if(m_htiHighQSourceLimit && pton->hItem == m_htiHighQSourceLimit){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiHighQSourceLimit, m_HighQSourceLimit)) SetModified();
				}else
				if(m_htiHighQSourceDropTime && pton->hItem == m_htiHighQSourceDropTime){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiHighQSourceDropTime, m_HighQSourceDropTime)) SetModified();
				}else
				if(m_htiHighQSourceMaxRank && pton->hItem == m_htiHighQSourceMaxRank){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiHighQSourceMaxRank, m_HighQSourceMaxRank)) SetModified();
				}else

			if(m_htiDeadTime && pton->hItem == m_htiDeadTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDeadTime, m_DeadTime)) SetModified();
			}else
				if(m_htiDeadTimeFWMulti && pton->hItem == m_htiDeadTimeFWMulti){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDeadTimeFWMulti, m_DeadTimeFWMulti)) SetModified();
				}else
			if(m_htiGlobalDeadTime && pton->hItem == m_htiGlobalDeadTime){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiGlobalDeadTime, m_GlobalDeadTime)) SetModified();
			}else
				if(m_htiGlobalDeadTimeFWMulti && pton->hItem == m_htiGlobalDeadTimeFWMulti){
					if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiGlobalDeadTimeFWMulti, m_GlobalDeadTimeFWMulti)) SetModified();
				}/*else*/
	// NEO: SDT END
		}else{
	// NEO: DRT - [DownloadReaskTweaks]
			if(m_htiSpreadReask && pton->hItem == m_htiSpreadReask){
				UINT bCheck;
				m_ctrlTreeOptions.GetCheckBox(m_htiSpreadReask, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiSpreadReask, bCheck, TRUE, TRUE);
			}
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]

		//Bad
			if(pton->hItem != NULL && (pton->hItem == m_htiBadSourceLimitTotal) || (pton->hItem == m_htiBadSourceLimitRelativ) || (pton->hItem == m_htiBadSourceLimitSpecyfic) || (pton->hItem == m_htiBadSourceLimitDefault) || (pton->hItem == m_htiBadSourceLimitGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiBadSourceLimit ,m_BadSourceLimitMode.Cut, m_BadSourceLimit,m_Category == NULL && m_paFiles == NULL);
			}else
			if(pton->hItem != NULL && (pton->hItem == m_htiBadSourceDropProgressiv) || (pton->hItem == m_htiBadSourceDropDistributiv) || (pton->hItem == m_htiBadSourceDropCummulativ) || (pton->hItem == m_htiBadSourceDropDefault) || (pton->hItem == m_htiBadSourceDropGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiBadSourceDropTime,m_BadSourceDropMode.Cut,m_BadSourceDropTime,m_Category == NULL && m_paFiles == NULL);
			}else

		//NNP
			if(pton->hItem != NULL && (pton->hItem == m_htiNNPSourceLimitTotal) || (pton->hItem == m_htiNNPSourceLimitRelativ) || (pton->hItem == m_htiNNPSourceLimitSpecyfic) || (pton->hItem == m_htiNNPSourceLimitDefault) || (pton->hItem == m_htiNNPSourceLimitGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiNNPSourceLimit ,m_NNPSourceLimitMode.Cut, m_NNPSourceLimit,m_Category == NULL && m_paFiles == NULL);
			}else
			if(pton->hItem != NULL && (pton->hItem == m_htiNNPSourceDropProgressiv) || (pton->hItem == m_htiNNPSourceDropDistributiv) || (pton->hItem == m_htiNNPSourceDropCummulativ) || (pton->hItem == m_htiNNPSourceDropDefault) || (pton->hItem == m_htiNNPSourceDropGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiNNPSourceDropTime,m_NNPSourceDropMode.Cut,m_NNPSourceDropTime,m_Category == NULL && m_paFiles == NULL);
			}else

		//FullQ
			if(pton->hItem != NULL && (pton->hItem == m_htiFullQSourceLimitTotal) || (pton->hItem == m_htiFullQSourceLimitRelativ) || (pton->hItem == m_htiFullQSourceLimitSpecyfic) || (pton->hItem == m_htiFullQSourceLimitDefault) || (pton->hItem == m_htiFullQSourceLimitGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiFullQSourceLimit ,m_FullQSourceLimitMode.Cut, m_FullQSourceLimit,m_Category == NULL && m_paFiles == NULL);
			}else
			if(pton->hItem != NULL && (pton->hItem == m_htiFullQSourceDropProgressiv) || (pton->hItem == m_htiFullQSourceDropDistributiv) || (pton->hItem == m_htiFullQSourceDropCummulativ) || (pton->hItem == m_htiFullQSourceDropDefault) || (pton->hItem == m_htiFullQSourceDropGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiFullQSourceDropTime,m_FullQSourceDropMode.Cut,m_FullQSourceDropTime,m_Category == NULL && m_paFiles == NULL);
			}else

		//HighQ
			if(pton->hItem != NULL && (pton->hItem == m_htiHighQSourceLimitTotal) || (pton->hItem == m_htiHighQSourceLimitRelativ) || (pton->hItem == m_htiHighQSourceLimitSpecyfic) || (pton->hItem == m_htiHighQSourceLimitDefault) || (pton->hItem == m_htiHighQSourceLimitGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiHighQSourceLimit ,m_HighQSourceLimitMode.Cut, m_HighQSourceLimit,m_Category == NULL && m_paFiles == NULL);
			}else
			if(pton->hItem != NULL && (pton->hItem == m_htiHighQSourceDropProgressiv) || (pton->hItem == m_htiHighQSourceDropDistributiv) || (pton->hItem == m_htiHighQSourceDropCummulativ) || (pton->hItem == m_htiHighQSourceDropDefault) || (pton->hItem == m_htiHighQSourceDropGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiHighQSourceDropTime,m_HighQSourceDropMode.Cut,m_HighQSourceDropTime,m_Category == NULL && m_paFiles == NULL);
			}else

			if(pton->hItem != NULL && (pton->hItem == m_htiHighQSourceRankNormal) || (pton->hItem == m_htiHighQSourceRankAverage) || (pton->hItem == m_htiHighQSourceRankDefault) || (pton->hItem == m_htiHighQSourceRankGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiHighQSourceMaxRank,m_HighQSourceRankMode.Cut,m_HighQSourceMaxRank,m_Category == NULL && m_paFiles == NULL);
			}/*else*/
	// NEO: SDT END
			SetModified();
		}
	}
	return 0;
}


LRESULT CPPgSources::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgSources::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgSources::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgSources::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
