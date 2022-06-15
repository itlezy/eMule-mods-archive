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
#include "PPgRelease.h"
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
#include "EmuleDlg.h"
#include "SharedFilesWnd.h" 
#include "SharedFilesCtrl.h" 
#include "KnownFileList.h" 
#include "sharedfilelist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// NEO: NPT - [NeoPartTraffic]
#define PT_EXPERT _T("Expert")
#define PT_NORMAL _T("Normal")
#define PT_RED	  _T("Red")
#define PT_BLUE   _T("Blue")

class CPartTrafficStyle : public CTreeOptionsCombo
{
public:
	CPartTrafficStyle();
	virtual ~CPartTrafficStyle();

protected:
	//{{AFX_VIRTUAL(CPartTrafficStyle)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPartTrafficStyle)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CPartTrafficStyle)
};


IMPLEMENT_DYNCREATE(CPartTrafficStyle, CTreeOptionsCombo)

CPartTrafficStyle::CPartTrafficStyle()
{
}

CPartTrafficStyle::~CPartTrafficStyle()
{
}

BEGIN_MESSAGE_MAP(CPartTrafficStyle, CTreeOptionsCombo)
	//{{AFX_MSG_MAP(CPartTrafficStyle)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CPartTrafficStyle::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  //Add strings to the combo
  AddString(PT_EXPERT);
  AddString(PT_NORMAL);
  AddString(PT_BLUE);
  AddString(PT_RED);
	
	return 0;
}
// NEO: NPT END 

///////////////////////////////////////////////////////////////////////////////
// CPPgRelease dialog

IMPLEMENT_DYNAMIC(CPPgRelease, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgRelease, CPropertyPage)
	ON_WM_TIMER() // NEO: FCFG - [FileConfiguration]
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged) // NEO: FCFG - [FileConfiguration]
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgRelease::CPPgRelease()
	: CPropertyPage(CPPgRelease::IDD)
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

CPPgRelease::~CPPgRelease()
{
}

void CPPgRelease::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
	m_htiPartTraffic = NULL; // NEO: NPT - [NeoPartTraffic]
	m_htiTweakUploadQueue = NULL;
		m_htiSaveUploadQueueWaitTime = NULL; // NEO: SQ - [SaveUploadQueue]
		m_htiUseMultiQueue = NULL; // NEO: MQ - [MultiQueue]
		m_htiNeoScoreSystem = NULL; // NEO: NFS - [NeoScoreSystem]
		// NEO: OCS - [OtherCreditSystems]
		m_htiCreditSystem = NULL;
			m_htiOfficialCreditSystem = NULL;
			m_htiNeoCreditSystem = NULL; // NEO: NCS - [NeoCreditSystem]
			m_htiOtherCreditSystem = NULL; 
		// NEO: OCS END
		m_htiUseRandomQueue = NULL; // NEO: RQ - [RandomQueue]
		// NEO: TQ - [TweakUploadQueue]
		m_htiInfiniteQueue = NULL;
		m_htiUploadQueueOverFlow = NULL;
			m_htiQueueOverFlowDef = NULL;
			m_htiQueueOverFlowEx = NULL;
			m_htiQueueOverFlowRelease = NULL;
			m_htiQueueOverFlowCF = NULL;
		// NEO: TQ END
		// NEO: PRSF - [PushSmallRareFiles]
		m_htiFilePushTweaks = NULL;
			m_htiPushSmallFiles = NULL;
			m_htiPushRareFiles = NULL;
			m_htiPushRatioFiles = NULL;
		// NEO: PRSF END
		// NEO: NMFS - [NiceMultiFriendSlots]
		m_htiFriendUpload = NULL;
			m_htiFriendSlotLimit = NULL;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			m_htiSeparateFriendBandwidth = NULL;
				m_htiFriendSlotSpeed = NULL;
				m_htiFriendBandwidthPercentage = NULL;
#endif // BW_MOD // NEO: BM END
		// NEO: NMFS END


	// NEO: IPS - [InteligentPartSharing]
	m_htiInteligentPartSharing = NULL;
		m_htiInteligentPartSharingDisable = NULL;
		m_htiInteligentPartSharingEnable = NULL;
		m_htiInteligentPartSharingDefault = NULL;
		m_htiInteligentPartSharingGlobal = NULL;

		m_htiInteligentPartSharingTimer = NULL;
		m_htiMaxProzentToHide = NULL;

	// OverAvalibly
		m_htiHideOverAvaliblyParts = NULL;
			m_htiHideOverAvaliblyPartsDisable = NULL;
			m_htiHideOverAvaliblyPartsOnleKF = NULL;
			m_htiHideOverAvaliblyPartsEnable = NULL;
			m_htiHideOverAvaliblyPartsOnlyPF = NULL;
			m_htiHideOverAvaliblyPartsDefault = NULL;
			m_htiHideOverAvaliblyPartsGlobal = NULL;
	
			m_htiHideOverAvaliblyMode = NULL;
				m_htiHideOverAvaliblyModeMultiplicativ = NULL;
				m_htiHideOverAvaliblyModeAdditiv = NULL;
				m_htiHideOverAvaliblyModeDefault = NULL;
				m_htiHideOverAvaliblyModeGlobal = NULL;

			m_htiHideOverAvaliblyValue = NULL;

			m_htiHideOverSharedCalc = NULL;
				m_htiHideOverSharedCalcHigh = NULL;
				m_htiHideOverSharedCalcLow = NULL;
				m_htiHideOverSharedCalcDefault = NULL;
				m_htiHideOverSharedCalcGlobal = NULL;

			m_htiBlockHighOverAvaliblyParts = NULL;
				m_htiBlockHighOverAvaliblyPartsDisable = NULL;
				m_htiBlockHighOverAvaliblyPartsOnleKF = NULL;
				m_htiBlockHighOverAvaliblyPartsEnable = NULL;
				m_htiBlockHighOverAvaliblyPartsOnlyPF = NULL;
				m_htiBlockHighOverAvaliblyPartsDefault = NULL;
				m_htiBlockHighOverAvaliblyPartsGlobal = NULL;

				m_htiBlockHighOverAvaliblyFactor = NULL;

	// OverShared
		m_htiHideOverSharedParts = NULL;
			m_htiHideOverSharedPartsDisable = NULL;
			m_htiHideOverSharedPartsOnleKF = NULL;
			m_htiHideOverSharedPartsEnable = NULL;
			m_htiHideOverSharedPartsOnlyPF = NULL;
			m_htiHideOverSharedPartsDefault = NULL;
			m_htiHideOverSharedPartsGlobal = NULL;
	
			m_htiHideOverSharedMode = NULL;
				m_htiHideOverSharedModeMultiplicativ = NULL;
				m_htiHideOverSharedModeAdditiv = NULL;
				m_htiHideOverSharedModeDefault = NULL;
				m_htiHideOverSharedModeGlobal = NULL;

			m_htiHideOverSharedValue = NULL;

			m_htiBlockHighOverSharedParts = NULL;
				m_htiBlockHighOverSharedPartsDisable = NULL;
				m_htiBlockHighOverSharedPartsOnleKF = NULL;
				m_htiBlockHighOverSharedPartsEnable = NULL;
				m_htiBlockHighOverSharedPartsOnlyPF = NULL;
				m_htiBlockHighOverSharedPartsDefault = NULL;
				m_htiBlockHighOverSharedPartsGlobal = NULL;

				m_htiBlockHighOverSharedFactor = NULL;

	// DontHideUnderAvalibly
			m_htiDontHideUnderAvaliblyParts = NULL;
				m_htiDontHideUnderAvaliblyPartsDisable = NULL;
				m_htiDontHideUnderAvaliblyPartsOnleKF = NULL;
				m_htiDontHideUnderAvaliblyPartsEnable = NULL;
				m_htiDontHideUnderAvaliblyPartsOnlyPF = NULL;
				m_htiDontHideUnderAvaliblyPartsDefault = NULL;
				m_htiDontHideUnderAvaliblyPartsGlobal = NULL;
		
				m_htiDontHideUnderAvaliblyMode = NULL;
					m_htiDontHideUnderAvaliblyModeMultiplicativ = NULL;
					m_htiDontHideUnderAvaliblyModeAdditiv = NULL;
					m_htiDontHideUnderAvaliblyModeDefault = NULL;
					m_htiDontHideUnderAvaliblyModeGlobal = NULL;

				m_htiDontHideUnderAvaliblyValue = NULL;

	// Other
			m_htiShowAlwaysSomeParts = NULL;
				m_htiShowAlwaysSomePartsDisable = NULL;
				m_htiShowAlwaysSomePartsOnleKF = NULL;
				m_htiShowAlwaysSomePartsEnable = NULL;
				m_htiShowAlwaysSomePartsOnlyPF = NULL;
				m_htiShowAlwaysSomePartsDefault = NULL;
				m_htiShowAlwaysSomePartsGlobal = NULL;

				m_htiShowAlwaysSomePartsValue = NULL;

			m_htiShowAlwaysIncompleteParts = NULL;
				m_htiShowAlwaysIncompletePartsDisable = NULL;
				m_htiShowAlwaysIncompletePartsOnleKF = NULL;
				m_htiShowAlwaysIncompletePartsEnable = NULL;
				m_htiShowAlwaysIncompletePartsOnlyPF = NULL;
				m_htiShowAlwaysIncompletePartsDefault = NULL;
				m_htiShowAlwaysIncompletePartsGlobal = NULL;
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	m_htiSmartReleaseSharing = NULL;
		m_htiReleaseModeMixed = NULL;
		m_htiReleaseModeBoost = NULL;
		m_htiReleaseModePower = NULL;
		m_htiReleaseModeDefault = NULL;
		m_htiReleaseModeGlobal = NULL;

		m_htiReleaseLevel = NULL;
		m_htiReleaseUpload = NULL;
			m_htiReleaseSlotLimit = NULL;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			m_htiSeparateReleaseBandwidth = NULL;
				m_htiReleaseSlotSpeed = NULL;
				m_htiReleaseBandwidthPercentage = NULL;
#endif // BW_MOD // NEO: BM END
			m_htiReleaseChunks = NULL;
		m_htiReleaseTimer = NULL;

	// release limit
		m_htiReleaseReleaseLimit = NULL;
			m_htiReleaseLimit = NULL;
				m_htiReleaseLimitDisable = NULL;
				m_htiReleaseLimitSingle = NULL;
				m_htiReleaseLimitBooth = NULL;
				m_htiReleaseLimitDefault = NULL;
				m_htiReleaseLimitGlobal = NULL;

				m_htiReleaseLimitHigh = NULL;
				m_htiReleaseLimitLow = NULL;

				m_htiReleaseLimitMode = NULL;
					m_htiReleaseLimitModeSimple = NULL;
					m_htiReleaseLimitModeLinear = NULL;
					m_htiReleaseLimitModeExplonential = NULL;
					m_htiReleaseLimitModeDefault = NULL;
					m_htiReleaseLimitModeGlobal = NULL;

			m_htiReleaseLimitLink = NULL;
				m_htiReleaseLimitLinkAnd = NULL;
				m_htiReleaseLimitLinkOr = NULL;
				m_htiReleaseLimitLinkDefault = NULL;
				m_htiReleaseLimitLinkGlobal = NULL;
				
			m_htiReleaseLimitComplete = NULL;
				m_htiReleaseLimitCompleteDisable = NULL;
				m_htiReleaseLimitCompleteSingle = NULL;
				m_htiReleaseLimitCompleteBooth = NULL;
				m_htiReleaseLimitCompleteDefault = NULL;
				m_htiReleaseLimitCompleteGlobal = NULL;

				m_htiReleaseLimitCompleteHigh = NULL;
				m_htiReleaseLimitCompleteLow = NULL;

				m_htiReleaseLimitCompleteMode = NULL;
					m_htiReleaseLimitCompleteModeSimple = NULL;
					m_htiReleaseLimitCompleteModeLinear = NULL;
					m_htiReleaseLimitCompleteModeExplonential = NULL;
					m_htiReleaseLimitCompleteModeDefault = NULL;
					m_htiReleaseLimitCompleteModeGlobal = NULL;
	// limit
		m_htiLimitLink = NULL;
			m_htiLimitLinkAnd = NULL;
			m_htiLimitLinkOr = NULL;
			m_htiLimitLinkDefault = NULL;
			m_htiLimitLinkGlobal = NULL;

	// source limit
		m_htiReleaseSourceLimit = NULL;
			m_htiSourceLimit = NULL;
				m_htiSourceLimitDisable = NULL;
				m_htiSourceLimitSingle = NULL;
				m_htiSourceLimitBooth = NULL;
				m_htiSourceLimitDefault = NULL;
				m_htiSourceLimitGlobal = NULL;

				m_htiSourceLimitHigh = NULL;
				m_htiSourceLimitLow = NULL;

				m_htiSourceLimitMode = NULL;
					m_htiSourceLimitModeSimple = NULL;
					m_htiSourceLimitModeLinear = NULL;
					m_htiSourceLimitModeExplonential = NULL;
					m_htiSourceLimitModeDefault = NULL;
					m_htiSourceLimitModeGlobal = NULL;

			m_htiSourceLimitLink = NULL;
				m_htiSourceLimitLinkAnd = NULL;
				m_htiSourceLimitLinkOr = NULL;
				m_htiSourceLimitLinkDefault = NULL;
				m_htiSourceLimitLinkGlobal = NULL;
				
			m_htiSourceLimitComplete = NULL;
				m_htiSourceLimitCompleteDisable = NULL;
				m_htiSourceLimitCompleteSingle = NULL;
				m_htiSourceLimitCompleteBooth = NULL;
				m_htiSourceLimitCompleteDefault = NULL;
				m_htiSourceLimitCompleteGlobal = NULL;

				m_htiSourceLimitCompleteHigh = NULL;
				m_htiSourceLimitCompleteLow = NULL;

				m_htiSourceLimitCompleteMode = NULL;
					m_htiSourceLimitCompleteModeSimple = NULL;
					m_htiSourceLimitCompleteModeLinear = NULL;
					m_htiSourceLimitCompleteModeExplonential = NULL;
					m_htiSourceLimitCompleteModeDefault = NULL;
					m_htiSourceLimitCompleteModeGlobal = NULL;
	// NEO: SRS END
}

// NEO: IPS - [InteligentPartSharing]
void CPPgRelease::SetTreeRadioForIPS(HTREEITEM &htiDisable, HTREEITEM &htiEnableKF, HTREEITEM &htiEnable, HTREEITEM &htiEnablePF, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),Value,0),htiParent,GetResString(IDS_X_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiEnableKF,MkRdBtnLbl(GetResString(IDS_X_ENABLE_KF),Value,1),htiParent,GetResString(IDS_X_ENABLE_KF_INFO),TRUE,Value.Val == 1);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable,MkRdBtnLbl(GetResString(IDS_X_ENABLE),Value,2),htiParent,GetResString(IDS_X_ENABLE_INFO),TRUE,Value.Val == 2);
	SetTreeRadio(m_ctrlTreeOptions,htiEnablePF,MkRdBtnLbl(GetResString(IDS_X_ENABLE_PF),Value,3),htiParent,GetResString(IDS_X_ENABLE_PF_INFO),TRUE,Value.Val == 3);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

void CPPgRelease::SetTreeRadioForIPSMode(HTREEITEM &htiGroup, int iImgMode, HTREEITEM &htiMultiplicativ, HTREEITEM &htiAdditiv, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeGroup(m_ctrlTreeOptions,htiGroup,GetResString(IDS_X_IPS_MODE),iImgMode, htiParent, GetResString(IDS_X_IPS_MODE_INFO));
		SetTreeRadio(m_ctrlTreeOptions,htiMultiplicativ,MkRdBtnLbl(GetResString(IDS_X_IPS_MODE_MULTI),Value,0),htiGroup,GetResString(IDS_X_IPS_MODE_MULTI_INFO),TRUE,Value.Val == 0);
		SetTreeRadio(m_ctrlTreeOptions,htiAdditiv,MkRdBtnLbl(GetResString(IDS_X_IPS_MODE_ADD),Value,1),htiGroup,GetResString(IDS_X_IPS_MODE_ADD_INFO),TRUE,Value.Val == 1);
		if(m_paFiles || m_Category)
			SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiGroup,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
		if(m_paFiles)
			SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiGroup,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}
// NEO: IPS END

// NEO: SRS - [SmartReleaseSharing]
void CPPgRelease::SetTreeRadioForSRS(HTREEITEM &htiDisable, HTREEITEM &htiSingle, HTREEITEM &htiBooth, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_SRS_DISABLE),Value,0),htiParent,GetResString(IDS_X_SRS_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiSingle,MkRdBtnLbl(GetResString(IDS_X_SRS_SINGLE),Value,1),htiParent,GetResString(IDS_X_SRS_SINGLE_INFO),TRUE,Value.Val == 1);
	SetTreeRadio(m_ctrlTreeOptions,htiBooth,MkRdBtnLbl(GetResString(IDS_X_SRS_BOOTH),Value,2),htiParent,GetResString(IDS_X_SRS_BOOTH_INFO),TRUE,Value.Val == 2);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

void CPPgRelease::SetTreeRadioForSRSMode(HTREEITEM &htiGroup, int iImgMode, HTREEITEM &htiSimple, HTREEITEM &htiLinear, HTREEITEM &htiExplonential, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeGroup(m_ctrlTreeOptions,htiGroup,GetResString(IDS_X_SRS_MODE),iImgMode, htiParent, GetResString(IDS_X_SRS_MODE_INFO));
		SetTreeRadio(m_ctrlTreeOptions,htiSimple,MkRdBtnLbl(GetResString(IDS_X_SRS_SIMPLE),Value,0),htiGroup,GetResString(IDS_X_SRS_SIMPLE_INFO),TRUE,Value.Val == 0);
		SetTreeRadio(m_ctrlTreeOptions,htiLinear,MkRdBtnLbl(GetResString(IDS_X_SRS_LINEAR),Value,1),htiGroup,GetResString(IDS_X_SRS_LINEAR_INFO),TRUE,Value.Val == 1);
		SetTreeRadio(m_ctrlTreeOptions,htiExplonential,MkRdBtnLbl(GetResString(IDS_X_SRS_EXPONENTIAL),Value,2),htiGroup,GetResString(IDS_X_SRS_EXPONENTIAL_INFO),TRUE,Value.Val == 2);
		if(m_paFiles || m_Category)
			SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiGroup,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
		if(m_paFiles)
			SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiGroup,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}
void CPPgRelease::SetTreeRadioForSRSLink(HTREEITEM &htiGroup, int iImgLink, HTREEITEM &htiAnd, HTREEITEM &htiOr, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeGroup(m_ctrlTreeOptions,htiGroup,GetResString(IDS_X_SRS_LINK),iImgLink, htiParent, GetResString(IDS_X_SRS_LINK_INFO));
		SetTreeRadio(m_ctrlTreeOptions,htiAnd,MkRdBtnLbl(GetResString(IDS_X_SRS_AND),Value,0),htiGroup,GetResString(IDS_X_SRS_AND),TRUE,Value.Val == 0);
		SetTreeRadio(m_ctrlTreeOptions,htiOr,MkRdBtnLbl(GetResString(IDS_X_SRS_OR),Value,1),htiGroup,GetResString(IDS_X_SRS_OR),TRUE,Value.Val == 1);
		if(m_paFiles || m_Category)
			SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiGroup,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
		if(m_paFiles)
			SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiGroup,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}
// NEO: SRS END

void CPPgRelease::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgQueue = 8;
		int iImgCS = 8;
		int iImgOverflow = 8;
		int iImgPush = 8;
		int iImgMFS = 8;
		int iImgRelUl = 8;

		int iImgIPS = 8;
		int iImgOA = 8;
		int iImgOS = 8;
		int iImgUA = 8;
		int iImgAS = 8;
		int iImgIS = 8;
		int iImgCalc = 8;
		int iImgMode = 8;
		int iImgBlock = 8;
		
		int iImgSRS = 8;
		int iImgRRL = 8;
		int iImgL = 8;
		int iImgLC = 8;
		int iImgM = 8;
		int iImgLL = 8;
		int iImgSSL = 8;
		
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgQueue = piml->Add(CTempIconLoader(_T("TWEAKQUEUE")));
			iImgCS = piml->Add(CTempIconLoader(_T("TWEAKQUEUECREDIT")));
			iImgOverflow = piml->Add(CTempIconLoader(_T("TWEAKQUEUEOVERFLOW")));
			iImgPush = piml->Add(CTempIconLoader(_T("PUSHSMALLRAREFILES")));
			iImgMFS = piml->Add(CTempIconLoader(_T("FRIENDSLOTUPOAD")));
			iImgRelUl = piml->Add(CTempIconLoader(_T("RELEASEUPOAD")));

			iImgIPS = piml->Add(CTempIconLoader(_T("IPS")));
			iImgOA = piml->Add(CTempIconLoader(_T("HIDEOA")));
			iImgOS = piml->Add(CTempIconLoader(_T("HIDEOS")));
			iImgUA = piml->Add(CTempIconLoader(_T("SHOWUA")));
			iImgAS = piml->Add(CTempIconLoader(_T("SHOWA")));
			iImgIS = piml->Add(CTempIconLoader(_T("SHOWI")));
			iImgCalc = piml->Add(CTempIconLoader(_T("CALCS")));
			iImgMode = piml->Add(CTempIconLoader(_T("IPSMODE")));
			iImgBlock = piml->Add(CTempIconLoader(_T("BLOCKPARTS")));

			iImgSRS = piml->Add(CTempIconLoader(_T("RELEASEPRIORITY")));
			iImgRRL = piml->Add(CTempIconLoader(_T("RELEASEPRIORITYELIMIT")));
			iImgL = piml->Add(CTempIconLoader(_T("RELEASEPRIORITYLIMIT")));
			iImgLC = piml->Add(CTempIconLoader(_T("RELEASEPRIORITYCOMPLETELIMIT")));
			iImgM = piml->Add(CTempIconLoader(_T("RELEASEPRIORITYMODE")));
			iImgLL = piml->Add(CTempIconLoader(_T("RELEASEPRIORITYLINK")));
			iImgSSL = piml->Add(CTempIconLoader(_T("RELEASEPRIORITYSOURCELIMIT")));
		}

		if(m_paFiles == NULL && m_Category == NULL)
		{
			m_htiPartTraffic = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_PART_TRAFFIC),TVI_ROOT,m_uUsePartTraffic,TRUE);
			m_ctrlTreeOptions.AddComboBox(m_htiPartTraffic,RUNTIME_CLASS(CPartTrafficStyle));
			m_ctrlTreeOptions.SetItemInfo(m_htiPartTraffic,GetResString(IDS_X_PART_TRAFFIC_INFO));

			SetTreeGroup(m_ctrlTreeOptions,m_htiTweakUploadQueue,GetResString(IDS_X_TWEAK_QUEUE),iImgQueue, TVI_ROOT, GetResString(IDS_X_TWEAK_QUEUE_INFO));
				SetTreeCheck(m_ctrlTreeOptions,m_htiSaveUploadQueueWaitTime,GetResString(IDS_X_SAVE_QUEUE),m_htiTweakUploadQueue,GetResString(IDS_X_SAVE_QUEUE_INFO),FALSE,m_bSaveUploadQueueWaitTime); // NEO: SQ - [SaveUploadQueue]
				SetTreeCheck(m_ctrlTreeOptions,m_htiUseMultiQueue,GetResString(IDS_X_MULTI_QUEUE),m_htiTweakUploadQueue,GetResString(IDS_X_MULTI_QUEUE_INFO),FALSE,m_bUseMultiQueue); // NEO: MQ - [MultiQueue]
				SetTreeCheck(m_ctrlTreeOptions,m_htiUseRandomQueue,GetResString(IDS_X_RANDOM_QUEUE),m_htiTweakUploadQueue,GetResString(IDS_X_RANDOM_QUEUE_INFO),TRUE,m_uUseRandomQueue); // NEO: RQ - [RandomQueue]
				SetTreeCheck(m_ctrlTreeOptions,m_htiNeoScoreSystem,GetResString(IDS_X_NEO_SCORE_SYSTEM),m_htiTweakUploadQueue,GetResString(IDS_X_NEO_SCORE_SYSTEM_INFO),FALSE,m_bNeoScoreSystem); // NEO: NFS - [NeoScoreSystem]
				// NEO: OCS - [OtherCreditSystems]
				SetTreeGroup(m_ctrlTreeOptions,m_htiCreditSystem,GetResString(IDS_X_CREDIT_SYSTEM),iImgCS, m_htiTweakUploadQueue, GetResString(IDS_X_CREDIT_SYSTEM_INFO));
					SetTreeRadio(m_ctrlTreeOptions,m_htiOfficialCreditSystem,GetResString(IDS_X_CREDIT_SYSTEM_OFFICIAL),m_htiCreditSystem,GetResString(IDS_X_CREDIT_SYSTEM_OFFICIAL_INFO),FALSE, m_iCreditSystem == 0);
					SetTreeRadio(m_ctrlTreeOptions,m_htiNeoCreditSystem,GetResString(IDS_X_NEO_CREDIT_SYSTEM),m_htiCreditSystem,GetResString(IDS_X_NEO_CREDIT_SYSTEM_INFO),FALSE, m_iCreditSystem == 1);
					//SetTreeCS(m_ctrlTreeOptions,m_htiOtherCreditSystem,GetResString(IDS_X_OTHER_CREDIT_SYSTEM),m_htiCreditSystem,GetResString(IDS_X_OTHER_CREDIT_SYSTEM_INFO), m_iCreditSystem == 2);
				// NEO: OCS END
				// NEO: TQ - [TweakUploadQueue]
				SetTreeCheck(m_ctrlTreeOptions,m_htiInfiniteQueue,GetResString(IDS_X_UNLIMITED_QUEUE),m_htiTweakUploadQueue,GetResString(IDS_X_UNLIMITED_QUEUE_INFO),FALSE,m_bUseInfiniteQueue);
				SetTreeGroup(m_ctrlTreeOptions,m_htiUploadQueueOverFlow,GetResString(IDS_X_OVERFLOW_QUEUE),iImgOverflow, m_htiTweakUploadQueue, GetResString(IDS_X_OVERFLOW_QUEUE_INFO));
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiQueueOverFlowDef,GetResString(IDS_X_OVERFLOW_QUEUE_DEFAULT), m_htiUploadQueueOverFlow,GetResString(IDS_X_OVERFLOW_QUEUE_DEFAULT_INFO),TRUE, m_uQueueOverFlowDef);
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiQueueOverFlowEx,GetResString(IDS_X_OVERFLOW_QUEUE_EXTENDED), m_htiUploadQueueOverFlow,GetResString(IDS_X_OVERFLOW_QUEUE_EXTENDED_INFO),TRUE, m_uQueueOverFlowEx);
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiQueueOverFlowRelease,GetResString(IDS_X_OVERFLOW_QUEUE_RELEASE), m_htiUploadQueueOverFlow,GetResString(IDS_X_OVERFLOW_QUEUE_RELEASE_INFO),TRUE, m_uQueueOverFlowRelease);
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiQueueOverFlowCF,GetResString(IDS_X_OVERFLOW_QUEUE_COMFIRND), m_htiUploadQueueOverFlow,GetResString(IDS_X_OVERFLOW_QUEUE_COMFIRND_INFO),TRUE, m_uQueueOverFlowCF);
				// NEO: TQ END
				// NEO: PRSF - [PushSmallRareFiles]
				SetTreeGroup(m_ctrlTreeOptions,m_htiFilePushTweaks,GetResString(IDS_X_PUSH_SMALL_RARE_FILES),iImgPush, m_htiTweakUploadQueue, GetResString(IDS_X_PUSH_SMALL_RARE_FILES_INFO));
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiPushSmallFiles,GetResString(IDS_X_PUSH_SMALL_FILES), m_htiFilePushTweaks,GetResString(IDS_X_PUSH_SMALL_FILES_INFO),FALSE, m_bPushSmallFiles);
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiPushRareFiles,GetResString(IDS_X_PUSH_RARE_FILES), m_htiFilePushTweaks,GetResString(IDS_X_PUSH_RARE_FILES_INFO),FALSE, m_bPushRareFiles);
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiPushRatioFiles,GetResString(IDS_X_PUSH_RATIO_FILES), m_htiFilePushTweaks,GetResString(IDS_X_PUSH_RATIO_FILES_INFO),FALSE, m_bPushRatioFiles);
				// NEO: PRSF END
				// NEO: NMFS - [NiceMultiFriendSlots]
				SetTreeGroup(m_ctrlTreeOptions,m_htiFriendUpload,GetResString(IDS_X_FRIEND_UPLOAD),iImgMFS, m_htiTweakUploadQueue, GetResString(IDS_X_FRIEND_UPLOAD_INFO));
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiFriendSlotLimit,GetResString(IDS_X_FRIEND_SLOT_LIMIT), m_htiFriendUpload,GetResString(IDS_X_FRIEND_SLOT_LIMIT_INFO),FALSE,m_uFriendSlotLimit);
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
					SetTreeCheck(m_ctrlTreeOptions,m_htiSeparateFriendBandwidth,GetResString(IDS_X_FRIEND_BANDWIDTH), m_htiFriendUpload,GetResString(IDS_X_FRIEND_BANDWIDTH_INFO),TRUE,m_uSeparateFriendBandwidth);
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiFriendSlotSpeed,GetResString(IDS_X_FRIEND_SLOT_SPEED), m_htiSeparateFriendBandwidth,GetResString(IDS_X_FRIEND_SLOT_SPEED_INFO));
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiFriendBandwidthPercentage,GetResString(IDS_X_FRIEND_BANDWIDTH_PERCENTAGE), m_htiSeparateFriendBandwidth,GetResString(IDS_X_FRIEND_BANDWIDTH_PERCENTAGE_INFO));
#endif // BW_MOD // NEO: BM END
				// NEO: NMFS END
		}

		// NEO: IPS - [InteligentPartSharing]
		SetTreeGroup(m_ctrlTreeOptions,m_htiInteligentPartSharing,GetResString(IDS_X_INTELIGENT_PART_SHARING),iImgIPS, TVI_ROOT, GetResString(IDS_X_INTELIGENT_PART_SHARING_INFO));
			SetTreeRadio(m_ctrlTreeOptions,m_htiInteligentPartSharingDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),m_UseInteligentPartSharing,0),m_htiInteligentPartSharing,GetResString(IDS_X_DISABLE_INFO),TRUE,m_UseInteligentPartSharing.Val == 0);
			SetTreeRadio(m_ctrlTreeOptions,m_htiInteligentPartSharingEnable,MkRdBtnLbl(GetResString(IDS_X_ENABLE),m_UseInteligentPartSharing,1),m_htiInteligentPartSharing,GetResString(IDS_X_ENABLE_INFO),TRUE,m_UseInteligentPartSharing.Val == 1);
			if(m_paFiles || m_Category)
				SetTreeRadio(m_ctrlTreeOptions,m_htiInteligentPartSharingDefault,GetResString(IDS_X_DEFAULT),m_htiInteligentPartSharing,GetResString(IDS_X_DEFAULT_INFO),TRUE,m_UseInteligentPartSharing.Val == FCFG_DEF);
			if(m_paFiles)
				SetTreeRadio(m_ctrlTreeOptions,m_htiInteligentPartSharingGlobal,GetResString(IDS_X_GLOBAL),m_htiInteligentPartSharing,GetResString(IDS_X_GLOBAL_INFO),TRUE,m_UseInteligentPartSharing.Val == FCFG_GLB);

			SetTreeNumEdit(m_ctrlTreeOptions,m_htiInteligentPartSharingTimer,GetResString(IDS_X_INTELIGENT_PART_SHARING_INTERVALS), m_htiInteligentPartSharing, GetResString(IDS_X_INTELIGENT_PART_SHARING_INTERVALS_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxProzentToHide,GetResString(IDS_X_INTELIGENT_PART_SHARING_MAXIMUM), m_htiInteligentPartSharing, GetResString(IDS_X_INTELIGENT_PART_SHARING_MAXIMUM_INFO));

		// OverAvalibly
			SetTreeGroup(m_ctrlTreeOptions,m_htiHideOverAvaliblyParts,GetResString(IDS_X_HIDE_OVER_AVALIBLY),iImgOA, m_htiInteligentPartSharing, GetResString(IDS_X_HIDE_OVER_AVALIBLY_INFO));
				SetTreeRadioForIPS(m_htiHideOverAvaliblyPartsDisable,m_htiHideOverAvaliblyPartsOnleKF,m_htiHideOverAvaliblyPartsEnable,m_htiHideOverAvaliblyPartsOnlyPF,m_htiHideOverAvaliblyPartsDefault,m_htiHideOverAvaliblyPartsGlobal,m_htiHideOverAvaliblyParts,m_HideOverAvaliblyParts);
			
				SetTreeRadioForIPSMode(m_htiHideOverAvaliblyMode,iImgMode,m_htiHideOverAvaliblyModeMultiplicativ,m_htiHideOverAvaliblyModeAdditiv,m_htiHideOverAvaliblyModeDefault,m_htiHideOverAvaliblyModeGlobal,m_htiHideOverAvaliblyParts,m_HideOverAvaliblyMode);

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiHideOverAvaliblyValue,GetResString(IDS_X_INTELIGENT_PART_SHARING_VALUE), m_htiHideOverAvaliblyParts, GetResString(IDS_X_INTELIGENT_PART_SHARING_VALUE_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiHideOverSharedCalc,GetResString(IDS_X_IPS_OS_CALC),iImgCalc, m_htiHideOverAvaliblyParts, GetResString(IDS_X_IPS_OS_CALC_INFO));
					SetTreeRadio(m_ctrlTreeOptions,m_htiHideOverSharedCalcHigh,MkRdBtnLbl(GetResString(IDS_X_IPS_OS_CALC_HIGH),m_HideOverSharedCalc,0),m_htiHideOverSharedCalc,GetResString(IDS_X_IPS_OS_CALC_HIGH_INFO),TRUE,m_HideOverSharedCalc.Val == 0);
					SetTreeRadio(m_ctrlTreeOptions,m_htiHideOverSharedCalcLow,MkRdBtnLbl(GetResString(IDS_X_IPS_OS_CALC_LOW),m_HideOverSharedCalc,1),m_htiHideOverSharedCalc,GetResString(IDS_X_IPS_OS_CALC_LOW_INFO),TRUE,m_HideOverSharedCalc.Val == 1);
					if(m_paFiles || m_Category)
						SetTreeRadio(m_ctrlTreeOptions,m_htiHideOverSharedCalcDefault,GetResString(IDS_X_DEFAULT),m_htiHideOverSharedCalc,GetResString(IDS_X_DEFAULT_INFO),TRUE,m_HideOverSharedCalc.Val == FCFG_DEF);
					if(m_paFiles)
						SetTreeRadio(m_ctrlTreeOptions,m_htiHideOverSharedCalcGlobal,GetResString(IDS_X_GLOBAL),m_htiHideOverSharedCalc,GetResString(IDS_X_GLOBAL_INFO),TRUE,m_HideOverSharedCalc.Val == FCFG_GLB);

				SetTreeGroup(m_ctrlTreeOptions,m_htiBlockHighOverAvaliblyParts,GetResString(IDS_X_IPS_BLOCK_PARTS),iImgBlock, m_htiHideOverAvaliblyParts, GetResString(IDS_X_IPS_BLOCK_PARTS_INFO));
					SetTreeRadioForIPS(m_htiBlockHighOverAvaliblyPartsDisable,m_htiBlockHighOverAvaliblyPartsOnleKF,m_htiBlockHighOverAvaliblyPartsEnable,m_htiBlockHighOverAvaliblyPartsOnlyPF,m_htiBlockHighOverAvaliblyPartsDefault,m_htiBlockHighOverAvaliblyPartsGlobal,m_htiBlockHighOverAvaliblyParts,m_BlockHighOverAvaliblyParts);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiBlockHighOverAvaliblyFactor,GetResString(IDS_X_IPS_BLOCK_PARTS_VALUE), m_htiBlockHighOverAvaliblyParts, GetResString(IDS_X_IPS_BLOCK_PARTS_VALUE_INFO));

		// OverShared
			SetTreeGroup(m_ctrlTreeOptions,m_htiHideOverSharedParts,GetResString(IDS_X_HIDE_OVER_SHARED),iImgOS, m_htiInteligentPartSharing, GetResString(IDS_X_HIDE_OVER_SHARED_INFO));
				SetTreeRadioForIPS(m_htiHideOverSharedPartsDisable,m_htiHideOverSharedPartsOnleKF,m_htiHideOverSharedPartsEnable,m_htiHideOverSharedPartsOnlyPF,m_htiHideOverSharedPartsDefault,m_htiHideOverSharedPartsGlobal,m_htiHideOverSharedParts,m_HideOverSharedParts);
			
				SetTreeRadioForIPSMode(m_htiHideOverSharedMode,iImgMode,m_htiHideOverSharedModeMultiplicativ,m_htiHideOverSharedModeAdditiv,m_htiHideOverSharedModeDefault,m_htiHideOverSharedModeGlobal,m_htiHideOverSharedParts,m_HideOverSharedMode);

				SetTreeNumEdit(m_ctrlTreeOptions,m_htiHideOverSharedValue,GetResString(IDS_X_INTELIGENT_PART_SHARING_VALUE), m_htiHideOverSharedParts, GetResString(IDS_X_INTELIGENT_PART_SHARING_VALUE_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiBlockHighOverSharedParts,GetResString(IDS_X_IPS_BLOCK_PARTS),iImgBlock, m_htiHideOverSharedParts, GetResString(IDS_X_IPS_BLOCK_PARTS_INFO));
					SetTreeRadioForIPS(m_htiBlockHighOverSharedPartsDisable,m_htiBlockHighOverSharedPartsOnleKF,m_htiBlockHighOverSharedPartsEnable,m_htiBlockHighOverSharedPartsOnlyPF,m_htiBlockHighOverSharedPartsDefault,m_htiBlockHighOverSharedPartsGlobal,m_htiBlockHighOverSharedParts,m_BlockHighOverSharedParts);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiBlockHighOverSharedFactor,GetResString(IDS_X_IPS_BLOCK_PARTS_VALUE), m_htiBlockHighOverSharedParts, GetResString(IDS_X_IPS_BLOCK_PARTS_VALUE));

		// DontHideUnderAvalibly
				SetTreeGroup(m_ctrlTreeOptions,m_htiDontHideUnderAvaliblyParts,GetResString(IDS_X_SHOW_UNDER_AVALIBLY),iImgUA, m_htiHideOverSharedParts, GetResString(IDS_X_SHOW_UNDER_AVALIBLY_INFO));
					SetTreeRadioForIPS(m_htiDontHideUnderAvaliblyPartsDisable,m_htiDontHideUnderAvaliblyPartsOnleKF,m_htiDontHideUnderAvaliblyPartsEnable,m_htiDontHideUnderAvaliblyPartsOnlyPF,m_htiDontHideUnderAvaliblyPartsDefault,m_htiDontHideUnderAvaliblyPartsGlobal,m_htiDontHideUnderAvaliblyParts,m_DontHideUnderAvaliblyParts);
				
					SetTreeRadioForIPSMode(m_htiDontHideUnderAvaliblyMode,iImgMode,m_htiDontHideUnderAvaliblyModeMultiplicativ,m_htiDontHideUnderAvaliblyModeAdditiv,m_htiDontHideUnderAvaliblyModeDefault,m_htiDontHideUnderAvaliblyModeGlobal,m_htiDontHideUnderAvaliblyParts,m_DontHideUnderAvaliblyMode);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiDontHideUnderAvaliblyValue,GetResString(IDS_X_INTELIGENT_PART_SHARING_VALUE), m_htiDontHideUnderAvaliblyParts, GetResString(IDS_X_INTELIGENT_PART_SHARING_VALUE_INFO));

		// Other
				SetTreeGroup(m_ctrlTreeOptions,m_htiShowAlwaysSomeParts,GetResString(IDS_X_SHOW_ALWAYS_SOME_PARTS),iImgAS, m_htiInteligentPartSharing, GetResString(IDS_X_SHOW_ALWAYS_SOME_PARTS_INFO));
					SetTreeRadioForIPS(m_htiShowAlwaysSomePartsDisable,m_htiShowAlwaysSomePartsOnleKF,m_htiShowAlwaysSomePartsEnable,m_htiShowAlwaysSomePartsOnlyPF,m_htiShowAlwaysSomePartsDefault,m_htiShowAlwaysSomePartsGlobal,m_htiShowAlwaysSomeParts,m_ShowAlwaysSomeParts);
				
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiShowAlwaysSomePartsValue,GetResString(IDS_X_SHOW_ALWAYS_SOME_PARTS_VALUE), m_htiShowAlwaysSomeParts, GetResString(IDS_X_SHOW_ALWAYS_SOME_PARTS_VALUE_INFO));

				SetTreeGroup(m_ctrlTreeOptions,m_htiShowAlwaysIncompleteParts,GetResString(IDS_X_SHOW_ALWAYS_INCPMPLETE_PARTS),iImgIS, m_htiInteligentPartSharing, GetResString(IDS_X_SHOW_ALWAYS_INCPMPLETE_PARTS_INFO));
					SetTreeRadioForIPS(m_htiShowAlwaysIncompletePartsDisable,m_htiShowAlwaysIncompletePartsOnleKF,m_htiShowAlwaysIncompletePartsEnable,m_htiShowAlwaysIncompletePartsOnlyPF,m_htiShowAlwaysIncompletePartsDefault,m_htiShowAlwaysIncompletePartsGlobal,m_htiShowAlwaysIncompleteParts,m_ShowAlwaysIncompleteParts);
		// NEO: IPS END


		// NEO: SRS - [SmartReleaseSharing]
		SetTreeGroup(m_ctrlTreeOptions,m_htiSmartReleaseSharing,GetResString(IDS_X_SMART_RELEASE_SHARING),iImgSRS, TVI_ROOT, GetResString(IDS_X_SMART_RELEASE_SHARING_INFO));
			SetTreeRadio(m_ctrlTreeOptions,m_htiReleaseModeMixed,MkRdBtnLbl(GetResString(IDS_X_SRS_MIXED),m_ReleaseMode,0),m_htiSmartReleaseSharing,GetResString(IDS_X_SRS_MIXED_INFO),TRUE,m_ReleaseMode.Val == 0);
			SetTreeRadio(m_ctrlTreeOptions,m_htiReleaseModeBoost,MkRdBtnLbl(GetResString(IDS_X_SRS_BOOST),m_ReleaseMode,1),m_htiSmartReleaseSharing,GetResString(IDS_X_SRS_BOOST_INFO),TRUE,m_ReleaseMode.Val == 1);
			SetTreeRadio(m_ctrlTreeOptions,m_htiReleaseModePower,MkRdBtnLbl(GetResString(IDS_X_SRS_POWER),m_ReleaseMode,2),m_htiSmartReleaseSharing,GetResString(IDS_X_SRS_POWER_INFO),TRUE,m_ReleaseMode.Val == 2);
			if(m_paFiles || m_Category)
				SetTreeRadio(m_ctrlTreeOptions,m_htiReleaseModeDefault,GetResString(IDS_X_DEFAULT),m_htiSmartReleaseSharing,GetResString(IDS_X_DEFAULT_INFO),TRUE,m_ReleaseMode.Val == FCFG_DEF);
			if(m_paFiles)
				SetTreeRadio(m_ctrlTreeOptions,m_htiReleaseModeGlobal,GetResString(IDS_X_GLOBAL),m_htiSmartReleaseSharing,GetResString(IDS_X_GLOBAL_INFO),TRUE,m_ReleaseMode.Val == FCFG_GLB);

			SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseLevel,GetResString(IDS_X_RELEASE_LEVEL), m_htiSmartReleaseSharing, GetResString(IDS_X_RELEASE_LEVEL_INFO));
			if(m_paFiles == NULL && m_Category == NULL)
			{
				SetTreeGroup(m_ctrlTreeOptions,m_htiReleaseUpload,GetResString(IDS_X_RELEASE_UPLOAD),iImgRelUl, m_htiSmartReleaseSharing, GetResString(IDS_X_RELEASE_UPLOAD_INFO));
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiReleaseSlotLimit,GetResString(IDS_X_RELEASE_SLOT_LIMIT), m_htiReleaseUpload,GetResString(IDS_X_RELEASE_SLOT_LIMIT_INFO),FALSE,m_uReleaseSlotLimit);
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
					SetTreeCheck(m_ctrlTreeOptions,m_htiSeparateReleaseBandwidth,GetResString(IDS_X_RELEASE_BANDWIDTH), m_htiReleaseUpload,GetResString(IDS_X_RELEASE_BANDWIDTH_INFO),TRUE,m_uSeparateReleaseBandwidth);
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseSlotSpeed,GetResString(IDS_X_RELEASE_SLOT_SPEED), m_htiSeparateReleaseBandwidth,GetResString(IDS_X_RELEASE_SLOT_SPEED_INFO));
						SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseBandwidthPercentage,GetResString(IDS_X_RELEASE_BANDWIDTH_PERCENTAGE), m_htiSeparateReleaseBandwidth,GetResString(IDS_X_RELEASE_BANDWIDTH_PERCENTAGE_INFO));
#endif // BW_MOD // NEO: BM END
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseChunks,GetResString(IDS_X_RELEASE_CHUNKS), m_htiReleaseUpload,GetResString(IDS_X_RELEASE_CHUNKS_INFO));
			}
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseTimer,GetResString(IDS_X_RELEASE_TIMER), m_htiSmartReleaseSharing, GetResString(IDS_X_RELEASE_TIMER_INFO));


	// release limit
			SetTreeGroup(m_ctrlTreeOptions,m_htiReleaseReleaseLimit,GetResString(IDS_X_RELEASE_RELEASE_LIMIT),iImgRRL, m_htiSmartReleaseSharing, GetResString(IDS_X_RELEASE_RELEASE_LIMIT_INFO));
				SetTreeGroup(m_ctrlTreeOptions,m_htiReleaseLimit,GetResString(IDS_X_RELEASE_LIMIT),iImgL, m_htiReleaseReleaseLimit, GetResString(IDS_X_RELEASE_LIMIT_INFO));
					SetTreeRadioForSRS(m_htiReleaseLimitDisable,m_htiReleaseLimitSingle,m_htiReleaseLimitBooth,m_htiReleaseLimitDefault,m_htiReleaseLimitGlobal,m_htiReleaseLimit,m_ReleaseLimit);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseLimitHigh,GetResString(IDS_X_SRS_LIMIT_HIGH), m_htiReleaseLimit, GetResString(IDS_X_SRS_LIMIT_HIGH_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseLimitLow,GetResString(IDS_X_SRS_LIMIT_LOW), m_htiReleaseLimit, GetResString(IDS_X_SRS_LIMIT_LOW_INFO));

					SetTreeRadioForSRSMode(m_htiReleaseLimitMode,iImgM,m_htiReleaseLimitModeSimple,m_htiReleaseLimitModeLinear,m_htiReleaseLimitModeExplonential,m_htiReleaseLimitModeDefault,m_htiReleaseLimitModeGlobal,m_htiReleaseLimit,m_ReleaseLimitMode);
					
				SetTreeRadioForSRSLink(m_htiReleaseLimitLink,iImgLL,m_htiReleaseLimitLinkAnd,m_htiReleaseLimitLinkOr,m_htiReleaseLimitLinkDefault,m_htiReleaseLimitLinkGlobal,m_htiReleaseReleaseLimit,m_ReleaseLimitLink);

				SetTreeGroup(m_ctrlTreeOptions,m_htiReleaseLimitComplete,GetResString(IDS_X_RELEASE_COMPLETE_LIMIT),iImgLC, m_htiReleaseReleaseLimit, GetResString(IDS_X_RELEASE_COMPLETE_LIMIT_INFO));
					SetTreeRadioForSRS(m_htiReleaseLimitCompleteDisable,m_htiReleaseLimitCompleteSingle,m_htiReleaseLimitCompleteBooth,m_htiReleaseLimitCompleteDefault,m_htiReleaseLimitCompleteGlobal,m_htiReleaseLimitComplete,m_ReleaseLimitComplete);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseLimitCompleteHigh,GetResString(IDS_X_SRS_LIMIT_HIGH), m_htiReleaseLimitComplete, GetResString(IDS_X_SRS_LIMIT_HIGH_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiReleaseLimitCompleteLow,GetResString(IDS_X_SRS_LIMIT_LOW), m_htiReleaseLimitComplete, GetResString(IDS_X_SRS_LIMIT_LOW_INFO));

					SetTreeRadioForSRSMode(m_htiReleaseLimitCompleteMode,iImgM,m_htiReleaseLimitCompleteModeSimple,m_htiReleaseLimitCompleteModeLinear,m_htiReleaseLimitCompleteModeExplonential,m_htiReleaseLimitCompleteModeDefault,m_htiReleaseLimitCompleteModeGlobal,m_htiReleaseLimitComplete,m_ReleaseLimitCompleteMode);

	// limit
			SetTreeRadioForSRSLink(m_htiLimitLink,iImgLL,m_htiLimitLinkAnd,m_htiLimitLinkOr,m_htiLimitLinkDefault,m_htiLimitLinkGlobal,m_htiSmartReleaseSharing,m_LimitLink);

	// source limit
			SetTreeGroup(m_ctrlTreeOptions,m_htiReleaseSourceLimit,GetResString(IDS_X_RELEASE_SOURCE_LIMIT),iImgSSL, m_htiSmartReleaseSharing, GetResString(IDS_X_RELEASE_SOURCE_LIMIT_INFO));
				SetTreeGroup(m_ctrlTreeOptions,m_htiSourceLimit,GetResString(IDS_X_SOURCE_LIMIT),iImgL, m_htiReleaseSourceLimit, GetResString(IDS_X_SOURCE_LIMIT_INFO));
					SetTreeRadioForSRS(m_htiSourceLimitDisable,m_htiSourceLimitSingle,m_htiSourceLimitBooth,m_htiSourceLimitDefault,m_htiSourceLimitGlobal,m_htiSourceLimit,m_SourceLimit);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceLimitHigh,GetResString(IDS_X_SRS_LIMIT_HIGH), m_htiSourceLimit, GetResString(IDS_X_SRS_LIMIT_HIGH_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceLimitLow,GetResString(IDS_X_SRS_LIMIT_LOW), m_htiSourceLimit, GetResString(IDS_X_SRS_LIMIT_LOW_INFO));

					SetTreeRadioForSRSMode(m_htiSourceLimitMode,iImgM,m_htiSourceLimitModeSimple,m_htiSourceLimitModeLinear,m_htiSourceLimitModeExplonential,m_htiSourceLimitModeDefault,m_htiSourceLimitModeGlobal,m_htiSourceLimit,m_SourceLimitMode);
					
				SetTreeRadioForSRSLink(m_htiSourceLimitLink,iImgLL,m_htiSourceLimitLinkAnd,m_htiSourceLimitLinkOr,m_htiSourceLimitLinkDefault,m_htiSourceLimitLinkGlobal,m_htiReleaseSourceLimit,m_SourceLimitLink);

				SetTreeGroup(m_ctrlTreeOptions,m_htiSourceLimitComplete,GetResString(IDS_X_SOURCE_COMPLETE_LIMIT),iImgLC, m_htiReleaseSourceLimit, GetResString(IDS_X_SOURCE_COMPLETE_LIMIT_INFO));
					SetTreeRadioForSRS(m_htiSourceLimitCompleteDisable,m_htiSourceLimitCompleteSingle,m_htiSourceLimitCompleteBooth,m_htiSourceLimitCompleteDefault,m_htiSourceLimitCompleteGlobal,m_htiSourceLimitComplete,m_SourceLimitComplete);

					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceLimitCompleteHigh,GetResString(IDS_X_SRS_LIMIT_HIGH), m_htiSourceLimitComplete, GetResString(IDS_X_SRS_LIMIT_HIGH_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiSourceLimitCompleteLow,GetResString(IDS_X_SRS_LIMIT_LOW), m_htiSourceLimitComplete, GetResString(IDS_X_SRS_LIMIT_LOW_INFO));

					SetTreeRadioForSRSMode(m_htiSourceLimitCompleteMode,iImgM,m_htiSourceLimitCompleteModeSimple,m_htiSourceLimitCompleteModeLinear,m_htiSourceLimitCompleteModeExplonential,m_htiSourceLimitCompleteModeDefault,m_htiSourceLimitCompleteModeGlobal,m_htiSourceLimitComplete,m_SourceLimitCompleteMode);
		// NEO: SRS END

		if(m_paFiles == NULL && m_Category == NULL){
			m_ctrlTreeOptions.SetItemEnable(m_htiPartTraffic, m_uUsePartTraffic, TRUE, TRUE); // NEO: NPT - [NeoPartTraffic]

			// NEO: TQ - [TweakUploadQueue]
			m_ctrlTreeOptions.SetGroupEnable(m_htiUploadQueueOverFlow, m_bUseInfiniteQueue ? FALSE : TRUE);

			// NEO: NMFS - [NiceMultiFriendSlots]
			m_ctrlTreeOptions.SetItemEnable(m_htiFriendSlotLimit, m_uFriendSlotLimit, TRUE, TRUE);
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			m_ctrlTreeOptions.SetItemEnable(m_htiFriendSlotSpeed, m_uSeparateFriendBandwidth);
			m_ctrlTreeOptions.SetItemEnable(m_htiFriendBandwidthPercentage, m_uSeparateFriendBandwidth);
#endif // BW_MOD // NEO: BM END
			// NEO: NMFS END

			m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowDef, m_uQueueOverFlowDef == 2, TRUE, TRUE);
			m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowEx, m_uQueueOverFlowEx == 2, TRUE, TRUE);
			m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowRelease, m_uQueueOverFlowRelease == 2, TRUE, TRUE);
			m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowCF, m_uQueueOverFlowCF == 2, TRUE, TRUE);
			// NEO: TQ END

			m_ctrlTreeOptions.SetItemEnable(m_htiReleaseSlotLimit, m_uReleaseSlotLimit, TRUE, TRUE);
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			m_ctrlTreeOptions.SetItemEnable(m_htiReleaseSlotSpeed, m_uSeparateReleaseBandwidth);
			m_ctrlTreeOptions.SetItemEnable(m_htiReleaseBandwidthPercentage, m_uSeparateReleaseBandwidth);
#endif // BW_MOD // NEO: BM END
		}

		// NEO: IPS - [InteligentPartSharing]
		m_ctrlTreeOptions.Expand(m_htiInteligentPartSharing, TVE_EXPAND);
		m_ctrlTreeOptions.SetItemEnable(m_htiHideOverSharedParts,NeoPrefs.UsePartTraffic());
		// NEO: IPS END

		// NEO: SRS - [SmartReleaseSharing]
		if(m_paFiles || m_Category)
			m_ctrlTreeOptions.Expand(m_htiSmartReleaseSharing, TVE_EXPAND);
		m_ctrlTreeOptions.SetItemEnable(m_htiReleaseLimitComplete,NeoPrefs.UsePartTraffic());
		// NEO: SRS END
		m_bInitializedTreeOpts = true;
	}

	if(m_paFiles == NULL && m_Category == NULL)
	{
		// NEO: NPT - [NeoPartTraffic]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPartTraffic, m_uUsePartTraffic);
		
		if (pDX->m_bSaveAndValidate){
			CString sText;
			sText = m_ctrlTreeOptions.GetComboText(m_htiPartTraffic);
			if(sText == PT_EXPERT)
				m_iPartTrafficCollors = 0;
			else if(sText == PT_NORMAL)
				m_iPartTrafficCollors = 1;
			else if(sText == PT_BLUE)
				m_iPartTrafficCollors = 2;
			else if(sText == PT_RED)
				m_iPartTrafficCollors = 3;
		}else{
			CString sText;
			switch(m_iPartTrafficCollors){
			case 0: sText = PT_EXPERT; break;
			case 1: sText = PT_NORMAL; break;
			case 2: sText = PT_BLUE; break;
			case 3: sText = PT_RED; break;
			}
			m_ctrlTreeOptions.SetComboText(m_htiPartTraffic, sText);
		}
		// NEO: NPT END

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSaveUploadQueueWaitTime, m_bSaveUploadQueueWaitTime); // NEO: SQ - [SaveUploadQueue]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseMultiQueue, m_bUseMultiQueue); // NEO: MQ - [MultiQueue]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseRandomQueue, m_uUseRandomQueue); // NEO: RQ - [RandomQueue]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiNeoScoreSystem, m_bNeoScoreSystem); // NEO: NFS - [NeoScoreSystem]
		// NEO: OCS - [OtherCreditSystems]
		DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiCreditSystem, m_iCreditSystem);
		//DDX_TreeCS(&m_ctrlTreeOptions,pDX, IDC_MOD_OPTS, m_htiOtherCreditSystem, m_iOtherCreditSystem);
		// NEO: OCS END
		// NEO: TQ - [TweakUploadQueue]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiInfiniteQueue, m_bUseInfiniteQueue);

			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowRelease, m_uQueueOverFlowRelease);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowRelease, m_iQueueOverFlowRelease);

			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowEx, m_uQueueOverFlowEx);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowEx, m_iQueueOverFlowEx);

			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowDef, m_uQueueOverFlowDef);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowDef, m_iQueueOverFlowDef);

			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowCF, m_uQueueOverFlowCF);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQueueOverFlowCF, m_iQueueOverFlowCF);
		// NEO: TQ END

		// NEO: PRSF - [PushSmallRareFiles]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPushSmallFiles, m_bPushSmallFiles);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPushSmallFiles, m_iPushSmallFilesSize);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPushRareFiles, m_bPushRareFiles);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPushRareFiles, m_iPushRareFilesValue);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPushRatioFiles, m_bPushRatioFiles);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPushRatioFiles, m_iPushRatioFilesValue);
		// NEO: PRSF END

			// NEO: NMFS - [NiceMultiFriendSlots]
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiFriendSlotLimit, m_uFriendSlotLimit);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiFriendSlotLimit, m_iFriendSlotLimit);
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSeparateFriendBandwidth, m_uSeparateFriendBandwidth);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiFriendSlotSpeed, m_fFriendSlotSpeed);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiFriendBandwidthPercentage, m_fFriendBandwidthPercentage);
#endif // BW_MOD // NEO: BM END
			// NEO: NMFS END
	}

	// NEO: IPS - [InteligentPartSharing]
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiInteligentPartSharing, m_UseInteligentPartSharing);
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiInteligentPartSharingTimer, m_InteligentPartSharingTimer);
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiMaxProzentToHide, m_MaxProzentToHide);

	// OverAvalibly
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHideOverAvaliblyParts, m_HideOverAvaliblyParts);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHideOverAvaliblyMode, m_HideOverAvaliblyMode);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiHideOverAvaliblyValue, m_HideOverAvaliblyValue);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHideOverSharedCalc, m_HideOverSharedCalc);

				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiBlockHighOverAvaliblyParts, m_BlockHighOverAvaliblyParts);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiBlockHighOverAvaliblyFactor, m_BlockHighOverAvaliblyFactor);
	// OverShared
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHideOverSharedParts, m_HideOverSharedParts);
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiHideOverSharedMode, m_HideOverSharedMode);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiHideOverSharedValue, m_HideOverSharedValue);

				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiBlockHighOverSharedParts, m_BlockHighOverSharedParts);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiBlockHighOverSharedFactor, m_BlockHighOverSharedFactor);


	// DontHideUnderAvalibly
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiDontHideUnderAvaliblyParts, m_DontHideUnderAvaliblyParts);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiDontHideUnderAvaliblyMode, m_DontHideUnderAvaliblyMode);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiDontHideUnderAvaliblyValue, m_DontHideUnderAvaliblyValue);
	// Other
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiShowAlwaysSomeParts, m_ShowAlwaysSomeParts);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiShowAlwaysSomePartsValue, m_ShowAlwaysSomePartsValue);

			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiShowAlwaysIncompleteParts, m_ShowAlwaysIncompleteParts);
	// NEO: IPS END


	// NEO: SRS - [SmartReleaseSharing]
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSmartReleaseSharing, m_ReleaseMode);
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReleaseLevel, m_ReleaseLevel);
	if(m_paFiles == NULL && m_Category == NULL)
	{
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiReleaseSlotLimit, m_uReleaseSlotLimit);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiReleaseSlotLimit, m_iReleaseSlotLimit); 
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSeparateReleaseBandwidth, m_uSeparateReleaseBandwidth);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiReleaseSlotSpeed, m_fReleaseSlotSpeed);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiReleaseBandwidthPercentage, m_fReleaseBandwidthPercentage);
#endif // BW_MOD // NEO: BM END
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiReleaseChunks, m_iReleaseChunks); 
	}

		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReleaseTimer, m_ReleaseTimer);

	// release limit
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimit, m_ReleaseLimit);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitHigh, m_ReleaseLimitHigh);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitLow, m_ReleaseLimitLow);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitMode, m_ReleaseLimitMode);

				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitLink, m_ReleaseLimitLink);

				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitComplete, m_ReleaseLimitComplete);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitCompleteHigh, m_ReleaseLimitCompleteHigh);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitCompleteLow, m_ReleaseLimitCompleteLow);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiReleaseLimitCompleteMode, m_ReleaseLimitCompleteMode);

	// limit
			DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiLimitLink, m_LimitLink);

	// source limit
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSourceLimit, m_SourceLimit);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitHigh, m_SourceLimitHigh);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitLow, m_SourceLimitLow);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitMode, m_SourceLimitMode);

				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitLink, m_SourceLimitLink);

				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitComplete, m_SourceLimitComplete);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitCompleteHigh, m_SourceLimitCompleteHigh);
				DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitCompleteLow, m_SourceLimitCompleteLow);
				DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiSourceLimitCompleteMode, m_SourceLimitCompleteMode);
		// NEO: SRS END
}

BOOL CPPgRelease::OnInitDialog()
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

void CPPgRelease::SetLimits()
{
	// NEO: IPS - [InteligentPartSharing]
	m_InteligentPartSharingTimer.MSMG(MIN_INTELIGENT_PART_SHARING_TIMER, DEF_INTELIGENT_PART_SHARING_TIMER, MAX_INTELIGENT_PART_SHARING_TIMER, NeoPrefs.KnownPrefs.m_InteligentPartSharingTimer, 't');

	m_MaxProzentToHide.MSMG(MIN_MAX_PROZENT_TO_HIDE, DEF_MAX_PROZENT_TO_HIDE, MAX_MAX_PROZENT_TO_HIDE, NeoPrefs.KnownPrefs.m_MaxProzentToHide);

	// OverAvalibly
	m_HideOverAvaliblyValue.MSMG(MIN_HIDE_OVER_AVALIBLY_VALUE_1, DEF_HIDE_OVER_AVALIBLY_VALUE_1, MAX_HIDE_OVER_AVALIBLY_VALUE_1, NeoPrefs.KnownPrefs.m_HideOverAvaliblyValue, 'f');
	m_HideOverAvaliblyValue.MSMa(MIN_HIDE_OVER_AVALIBLY_VALUE_2, DEF_HIDE_OVER_AVALIBLY_VALUE_2, MAX_HIDE_OVER_AVALIBLY_VALUE_2,1, 'f');

	m_BlockHighOverAvaliblyFactor.MSMG(MIN_BLOCK_HIGH_OVER_AVALIBLY_FACTOR, DEF_BLOCK_HIGH_OVER_AVALIBLY_FACTOR, MAX_BLOCK_HIGH_OVER_AVALIBLY_FACTOR, NeoPrefs.KnownPrefs.m_BlockHighOverAvaliblyFactor, 'f');

	// OverShared
	m_HideOverSharedValue.MSMG(MIN_HIDE_OVER_SHARED_VALUE_1, DEF_HIDE_OVER_SHARED_VALUE_1, MAX_HIDE_OVER_SHARED_VALUE_1, NeoPrefs.KnownPrefs.m_HideOverSharedValue, 'f');
	m_HideOverSharedValue.MSMa(MIN_HIDE_OVER_SHARED_VALUE_2, DEF_HIDE_OVER_SHARED_VALUE_2, MAX_HIDE_OVER_SHARED_VALUE_2,1, 'f');

	m_BlockHighOverSharedFactor.MSMG(MIN_BLOCK_HIGH_OVER_SHARED_FACTOR, DEF_BLOCK_HIGH_OVER_SHARED_FACTOR, MAX_BLOCK_HIGH_OVER_SHARED_FACTOR, NeoPrefs.KnownPrefs.m_BlockHighOverSharedFactor, 'f');

	// DontHideUnderAvalibly
	m_DontHideUnderAvaliblyValue.MSMG(MIN_DONT_HIDEUNDER_AVALIBLY_VALUE_1, DEF_DONT_HIDEUNDER_AVALIBLY_VALUE_1, MAX_DONT_HIDEUNDER_AVALIBLY_VALUE_1, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyValue, 'f');
	m_DontHideUnderAvaliblyValue.MSMa(MIN_DONT_HIDEUNDER_AVALIBLY_VALUE_2, DEF_DONT_HIDEUNDER_AVALIBLY_VALUE_2, MAX_DONT_HIDEUNDER_AVALIBLY_VALUE_2,1, 'f');

	// Other
	m_ShowAlwaysSomePartsValue.MSMG(MIN_SHOW_ALWAYS_SOME_PARTS_VALUE, DEF_SHOW_ALWAYS_SOME_PARTS_VALUE, MAX_SHOW_ALWAYS_SOME_PARTS_VALUE, NeoPrefs.KnownPrefs.m_ShowAlwaysSomePartsValue);
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	m_ReleaseLevel.MSMG(MIN_RELEASE_LEVEL, DEF_RELEASE_LEVEL, MAX_RELEASE_LEVEL, NeoPrefs.KnownPrefs.m_ReleaseLevel);
	m_ReleaseTimer.MSMG(MIN_RELEASE_TIMER, DEF_RELEASE_TIMER, MAX_RELEASE_TIMER, NeoPrefs.KnownPrefs.m_ReleaseTimer, 't');

	// release limit
	m_ReleaseLimitHigh.MSMG(MIN_RELEASE_LIMIT_HIGH, DEF_RELEASE_LIMIT_HIGH, MAX_RELEASE_LIMIT_HIGH, NeoPrefs.KnownPrefs.m_ReleaseLimitHigh);
	m_ReleaseLimitLow.MSMG(MIN_RELEASE_LIMIT_LOW, DEF_RELEASE_LIMIT_LOW, MAX_RELEASE_LIMIT_LOW, NeoPrefs.KnownPrefs.m_ReleaseLimitLow);

	m_ReleaseLimitCompleteHigh.MSMG(MIN_RELEASE_LIMIT_COMPLETE_HIGH, DEF_RELEASE_LIMIT_COMPLETE_HIGH, MAX_RELEASE_LIMIT_COMPLETE_HIGH, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteHigh);
	m_ReleaseLimitCompleteLow.MSMG(MIN_RELEASE_LIMIT_COMPLETE_LOW, DEF_RELEASE_LIMIT_COMPLETE_LOW, MAX_RELEASE_LIMIT_COMPLETE_LOW, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteLow);

	// source limit
	m_SourceLimitHigh.MSMG(MIN_SOURCE_LIMIT_HIGH, DEF_SOURCE_LIMIT_HIGH, MAX_SOURCE_LIMIT_HIGH, NeoPrefs.KnownPrefs.m_SourceLimitHigh);
	m_SourceLimitLow.MSMG(MIN_SOURCE_LIMIT_LOW, DEF_SOURCE_LIMIT_LOW, MAX_SOURCE_LIMIT_LOW, NeoPrefs.KnownPrefs.m_SourceLimitLow);

	m_SourceLimitCompleteHigh.MSMG(MIN_SOURCE_LIMIT_COMPLETE_HIGH, DEF_SOURCE_LIMIT_COMPLETE_HIGH, MAX_SOURCE_LIMIT_COMPLETE_HIGH, NeoPrefs.KnownPrefs.m_SourceLimitCompleteHigh);
	m_SourceLimitCompleteLow.MSMG(MIN_SOURCE_LIMIT_COMPLETE_LOW, DEF_SOURCE_LIMIT_COMPLETE_LOW, MAX_SOURCE_LIMIT_COMPLETE_LOW, NeoPrefs.KnownPrefs.m_SourceLimitCompleteLow);
	// NEO: SRS END
}

void CPPgRelease::LoadSettings()
{
	/*
	* Einstellungen Laden
	*/

	SetLimits();

	if(m_Category)
	{
		CKnownPreferences* KnownPrefs = m_Category->KnownPrefs;

		// NEO: IPS - [InteligentPartSharing]
		m_UseInteligentPartSharing.CVDC(1, KnownPrefs ? KnownPrefs->m_UseInteligentPartSharing : FCFG_DEF, NeoPrefs.KnownPrefs.m_UseInteligentPartSharing);
		m_InteligentPartSharingTimer.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_InteligentPartSharingTimer : FCFG_DEF);

		m_MaxProzentToHide.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_MaxProzentToHide : FCFG_DEF);

		// OverAvalibly
		m_HideOverAvaliblyParts.CVDC(3, KnownPrefs ? KnownPrefs->m_HideOverAvaliblyParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverAvaliblyParts);
		m_HideOverAvaliblyMode.CVDC(1, KnownPrefs ? KnownPrefs->m_HideOverAvaliblyMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverAvaliblyMode);
		m_HideOverAvaliblyValue.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_HideOverAvaliblyValue : FCFG_DEF);

		m_BlockHighOverAvaliblyParts.CVDC(3, KnownPrefs ? KnownPrefs->m_BlockHighOverAvaliblyParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_BlockHighOverAvaliblyParts);
		m_BlockHighOverAvaliblyFactor.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_BlockHighOverAvaliblyFactor : FCFG_DEF);

		// OverShared
		m_HideOverSharedParts.CVDC(3, KnownPrefs ? KnownPrefs->m_HideOverSharedParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverSharedParts);
		m_HideOverSharedMode.CVDC(1, KnownPrefs ? KnownPrefs->m_HideOverSharedMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverSharedMode);
		m_HideOverSharedValue.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_HideOverSharedValue : FCFG_DEF);
		m_HideOverSharedCalc.CVDC(1, KnownPrefs ? KnownPrefs->m_HideOverSharedCalc : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverSharedCalc);

		m_BlockHighOverSharedParts.CVDC(3, KnownPrefs ? KnownPrefs->m_BlockHighOverSharedParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_BlockHighOverSharedParts);
		m_BlockHighOverSharedFactor.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_BlockHighOverSharedFactor : FCFG_DEF);

		// DontHideUnderAvalibly
		m_DontHideUnderAvaliblyParts.CVDC(3, KnownPrefs ? KnownPrefs->m_DontHideUnderAvaliblyParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyParts);
		m_DontHideUnderAvaliblyMode.CVDC(1, KnownPrefs ? KnownPrefs->m_DontHideUnderAvaliblyMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyMode);
		m_DontHideUnderAvaliblyValue.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_DontHideUnderAvaliblyValue : FCFG_DEF);

		// Other
		m_ShowAlwaysSomeParts.CVDC(3, KnownPrefs ? KnownPrefs->m_ShowAlwaysSomeParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_ShowAlwaysSomeParts);
		m_ShowAlwaysSomePartsValue.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ShowAlwaysSomePartsValue : FCFG_DEF);

		m_ShowAlwaysIncompleteParts.CVDC(3, KnownPrefs ? KnownPrefs->m_ShowAlwaysIncompleteParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_ShowAlwaysIncompleteParts);
		// NEO: IPS END

		// NEO: SRS - [SmartReleaseSharing]
		m_ReleaseMode.CVDC(2, KnownPrefs ? KnownPrefs->m_ReleaseMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseMode);
		m_ReleaseLevel.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ReleaseLevel : FCFG_DEF);
		m_ReleaseTimer.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ReleaseTimer : FCFG_DEF);

		// release limit
		m_ReleaseLimit.CVDC(2, KnownPrefs ? KnownPrefs->m_ReleaseLimit : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimit);
		m_ReleaseLimitMode.CVDC(2, KnownPrefs ? KnownPrefs->m_ReleaseLimitMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitMode);
		m_ReleaseLimitHigh.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ReleaseLimitHigh : FCFG_DEF);
		m_ReleaseLimitLow.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ReleaseLimitLow : FCFG_DEF);

		m_ReleaseLimitLink.CVDC(1, KnownPrefs ? KnownPrefs->m_ReleaseLimitLink : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitLink);

		m_ReleaseLimitComplete.CVDC(2, KnownPrefs ? KnownPrefs->m_ReleaseLimitComplete : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitComplete);
		m_ReleaseLimitCompleteMode.CVDC(2, KnownPrefs ? KnownPrefs->m_ReleaseLimitCompleteMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteMode);
		m_ReleaseLimitCompleteHigh.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ReleaseLimitCompleteHigh : FCFG_DEF);
		m_ReleaseLimitCompleteLow.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_ReleaseLimitCompleteLow : FCFG_DEF);

		// limit
		m_LimitLink.CVDC(1, KnownPrefs ? KnownPrefs->m_LimitLink : FCFG_DEF, NeoPrefs.KnownPrefs.m_LimitLink);

		// source limit
		m_SourceLimit.CVDC(2, KnownPrefs ? KnownPrefs->m_SourceLimit : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimit);
		m_SourceLimitMode.CVDC(2, KnownPrefs ? KnownPrefs->m_SourceLimitMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitMode);
		m_SourceLimitHigh.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_SourceLimitHigh : FCFG_DEF);
		m_SourceLimitLow.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_SourceLimitLow : FCFG_DEF);

		m_SourceLimitLink.CVDC(1, KnownPrefs ? KnownPrefs->m_SourceLimitLink : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitLink);

		m_SourceLimitComplete.CVDC(2, KnownPrefs ? KnownPrefs->m_SourceLimitComplete : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitComplete);
		m_SourceLimitCompleteMode.CVDC(2, KnownPrefs ? KnownPrefs->m_SourceLimitCompleteMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitCompleteMode);
		m_SourceLimitCompleteHigh.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_SourceLimitCompleteHigh : FCFG_DEF);
		m_SourceLimitCompleteLow.DV(FCFG_GLB, KnownPrefs ? KnownPrefs->m_SourceLimitCompleteLow : FCFG_DEF);
		// NEO: SRS END
	}
	else
	{

		// NEO: NPT - [NeoPartTraffic]
		m_uUsePartTraffic = NeoPrefs.m_uUsePartTraffic;
		m_iPartTrafficCollors = NeoPrefs.m_iPartTrafficCollors; 
		// NEO: NPT END
		m_bSaveUploadQueueWaitTime = NeoPrefs.m_bSaveUploadQueueWaitTime; // NEO: SQ - [SaveUploadQueue]
		m_bUseMultiQueue = NeoPrefs.m_bUseMultiQueue; // NEO: MQ - [MultiQueue]
		m_uUseRandomQueue = NeoPrefs.m_uUseRandomQueue; // NEO: RQ - [RandomQueue]

		m_bNeoScoreSystem = NeoPrefs.m_bNeoScoreSystem; // NEO: NFS - [NeoScoreSystem]
		// NEO: OCS - [OtherCreditSystems]
		if(NeoPrefs.m_bNeoCreditSystem){
			m_iCreditSystem = 1;
			m_iOtherCreditSystem = 0;
		}else if(NeoPrefs.m_iOtherCreditSystem > 0){
			m_iCreditSystem = 2;
			m_iOtherCreditSystem = NeoPrefs.m_iOtherCreditSystem;
		}else{
			m_iCreditSystem = 0;
			m_iOtherCreditSystem = 0;
		}
		// NEO: OCS END

		// NEO: TQ - [TweakUploadQueue]
		m_bUseInfiniteQueue = NeoPrefs.m_bUseInfiniteQueue;

		m_uQueueOverFlowRelease = NeoPrefs.m_uQueueOverFlowRelease;
		m_iQueueOverFlowRelease = NeoPrefs.m_iQueueOverFlowRelease;

		m_uQueueOverFlowEx = NeoPrefs.m_uQueueOverFlowEx;
		m_iQueueOverFlowEx = NeoPrefs.m_iQueueOverFlowEx;

		m_uQueueOverFlowDef = NeoPrefs.m_uQueueOverFlowDef;
		m_iQueueOverFlowDef = NeoPrefs.m_iQueueOverFlowDef;

		m_uQueueOverFlowCF = NeoPrefs.m_uQueueOverFlowCF;
		m_iQueueOverFlowCF = NeoPrefs.m_iQueueOverFlowCF;
		// NEO: TQ END

		// NEO: PRSF - [PushSmallRareFiles]
		m_bPushSmallFiles = NeoPrefs.m_bPushSmallFiles;
		m_iPushSmallFilesSize = NeoPrefs.m_iPushSmallFilesSize;

		m_bPushRareFiles = NeoPrefs.m_bPushRareFiles;
		m_iPushRareFilesValue = NeoPrefs.m_iPushRareFilesValue;

		m_bPushRatioFiles = NeoPrefs.m_bPushRatioFiles;
		m_iPushRatioFilesValue = NeoPrefs.m_iPushRatioFilesValue;
		// NEO: PRSF END

		// NEO: NMFS - [NiceMultiFriendSlots]
		m_uFriendSlotLimit = NeoPrefs.m_uFriendSlotLimit;
		m_iFriendSlotLimit = NeoPrefs.m_iFriendSlotLimit;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
		m_uSeparateFriendBandwidth = NeoPrefs.m_uSeparateFriendBandwidth;
		m_fFriendSlotSpeed = NeoPrefs.m_fFriendSlotSpeed;
		m_fFriendBandwidthPercentage = NeoPrefs.m_fFriendBandwidthPercentage;
#endif // BW_MOD // NEO: BM END
		// NEO: NMFS END

		// NEO: IPS - [InteligentPartSharing]
		m_UseInteligentPartSharing.CVDC(1, NeoPrefs.KnownPrefs.m_UseInteligentPartSharing);
		m_InteligentPartSharingTimer.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_InteligentPartSharingTimer, true);

		m_MaxProzentToHide.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_MaxProzentToHide, true);

		// OverAvalibly
		m_HideOverAvaliblyParts.CVDC(3, NeoPrefs.KnownPrefs.m_HideOverAvaliblyParts);
		m_HideOverAvaliblyMode.CVDC(1, NeoPrefs.KnownPrefs.m_HideOverAvaliblyMode);
		m_HideOverAvaliblyValue.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_HideOverAvaliblyValue, true);

		m_BlockHighOverAvaliblyParts.CVDC(3, NeoPrefs.KnownPrefs.m_BlockHighOverAvaliblyParts);
		m_BlockHighOverAvaliblyFactor.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_BlockHighOverAvaliblyFactor, true);

		// OverShared
		m_HideOverSharedParts.CVDC(3, NeoPrefs.KnownPrefs.m_HideOverSharedParts);
		m_HideOverSharedMode.CVDC(1, NeoPrefs.KnownPrefs.m_HideOverSharedMode);
		m_HideOverSharedValue.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_HideOverSharedValue, true);
		m_HideOverSharedCalc.CVDC(1, NeoPrefs.KnownPrefs.m_HideOverSharedCalc);

		m_BlockHighOverSharedParts.CVDC(3, NeoPrefs.KnownPrefs.m_BlockHighOverSharedParts);
		m_BlockHighOverSharedFactor.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_BlockHighOverSharedFactor, true);

		// DontHideUnderAvalibly
		m_DontHideUnderAvaliblyParts.CVDC(3, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyParts);
		m_DontHideUnderAvaliblyMode.CVDC(1, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyMode);
		m_DontHideUnderAvaliblyValue.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyValue, true);

		// Other
		m_ShowAlwaysSomeParts.CVDC(3, NeoPrefs.KnownPrefs.m_ShowAlwaysSomeParts);
		m_ShowAlwaysSomePartsValue.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ShowAlwaysSomePartsValue, true);

		m_ShowAlwaysIncompleteParts.CVDC(3, NeoPrefs.KnownPrefs.m_ShowAlwaysIncompleteParts);
		// NEO: IPS END

		// NEO: SRS - [SmartReleaseSharing]
		m_ReleaseMode.CVDC(2, NeoPrefs.KnownPrefs.m_ReleaseMode);
		m_ReleaseLevel.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ReleaseLevel, true);
		m_iReleaseChunks = NeoPrefs.m_iReleaseChunks;
		m_uReleaseSlotLimit = NeoPrefs.m_uReleaseSlotLimit;
		m_iReleaseSlotLimit = NeoPrefs.m_iReleaseSlotLimit;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
		m_uSeparateReleaseBandwidth = NeoPrefs.m_uSeparateReleaseBandwidth;
		m_fReleaseSlotSpeed = NeoPrefs.m_fReleaseSlotSpeed;
		m_fReleaseBandwidthPercentage = NeoPrefs.m_fReleaseBandwidthPercentage;
#endif // BW_MOD // NEO: BM END
	// NEO: RT END
		m_ReleaseTimer.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ReleaseTimer, true);

		// release limit
		m_ReleaseLimit.CVDC(2, NeoPrefs.KnownPrefs.m_ReleaseLimit);
		m_ReleaseLimitMode.CVDC(2, NeoPrefs.KnownPrefs.m_ReleaseLimitMode);
		m_ReleaseLimitHigh.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ReleaseLimitHigh, true);
		m_ReleaseLimitLow.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ReleaseLimitLow, true);

		m_ReleaseLimitLink.CVDC(1, NeoPrefs.KnownPrefs.m_ReleaseLimitLink);

		m_ReleaseLimitComplete.CVDC(2, NeoPrefs.KnownPrefs.m_ReleaseLimitComplete);
		m_ReleaseLimitCompleteMode.CVDC(2, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteMode);
		m_ReleaseLimitCompleteHigh.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteHigh, true);
		m_ReleaseLimitCompleteLow.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteLow, true);

		// limit
		m_LimitLink.CVDC(1, NeoPrefs.KnownPrefs.m_LimitLink);

		// source limit
		m_SourceLimit.CVDC(2, NeoPrefs.KnownPrefs.m_SourceLimit);
		m_SourceLimitMode.CVDC(2, NeoPrefs.KnownPrefs.m_SourceLimitMode);
		m_SourceLimitHigh.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_SourceLimitHigh, true);
		m_SourceLimitLow.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_SourceLimitLow, true);

		m_SourceLimitLink.CVDC(1, NeoPrefs.KnownPrefs.m_SourceLimitLink);

		m_SourceLimitComplete.CVDC(2, NeoPrefs.KnownPrefs.m_SourceLimitComplete);
		m_SourceLimitCompleteMode.CVDC(2, NeoPrefs.KnownPrefs.m_SourceLimitCompleteMode);
		m_SourceLimitCompleteHigh.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_SourceLimitCompleteHigh, true);
		m_SourceLimitCompleteLow.DV(FCFG_STD, NeoPrefs.KnownPrefs.m_SourceLimitCompleteLow, true);
		// NEO: SRS END
	}

	SetAuxSet(m_Category == NULL);
}

void CPPgRelease::SetAuxSet(bool bGlobal)
{
	// NEO: IPS - [InteligentPartSharing]
	// OverAvalibly
	m_HideOverAvaliblyValue.aS(PrepAuxSet(m_HideOverAvaliblyMode.Val,bGlobal));
	// OverShared
	m_HideOverSharedValue.aS(PrepAuxSet(m_HideOverSharedMode.Val,bGlobal));
	// DontHideUnderAvalibly
	m_DontHideUnderAvaliblyValue.aS(PrepAuxSet(m_DontHideUnderAvaliblyMode.Val,bGlobal));
	// NEO: IPS END
}

// NEO: FCFG - [FileConfiguration]
void CPPgRelease::RefreshData()
{
	/*
	* Datei Einstellungen Laden
	*/

	SetLimits();

	if(!theApp.downloadqueue->IsPartFile((CKnownFile*)(*m_paFiles)[0]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)(*m_paFiles)[0]))
		return;

	const CKnownFile* KnownFile = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[0]);

	CKnownPreferences* KnownPrefs = KnownFile->KnownPrefs;
	Category_Struct* Category = thePrefs.GetCategory(KnownFile->GetCategory());
	ASSERT(Category);

	// set the right default value
	// and the value if the file have an prefs object

	// NEO: IPS - [InteligentPartSharing]
	m_UseInteligentPartSharing.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_UseInteligentPartSharing : FCFG_DEF, NeoPrefs.KnownPrefs.m_UseInteligentPartSharing, Category && Category->KnownPrefs ? Category->KnownPrefs->m_UseInteligentPartSharing : -1);
	m_InteligentPartSharingTimer.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_InteligentPartSharingTimer : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_InteligentPartSharingTimer : FCFG_DEF);

	m_MaxProzentToHide.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_MaxProzentToHide : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_MaxProzentToHide : FCFG_DEF);

	// OverAvalibly
	m_HideOverAvaliblyParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverAvaliblyParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverAvaliblyParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_HideOverAvaliblyParts : -1);
	m_HideOverAvaliblyMode.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverAvaliblyMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverAvaliblyMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_HideOverAvaliblyMode : -1);
	m_HideOverAvaliblyValue.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_HideOverAvaliblyValue : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverAvaliblyValue : FCFG_DEF);

	m_BlockHighOverAvaliblyParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_BlockHighOverAvaliblyParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_BlockHighOverAvaliblyParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_BlockHighOverAvaliblyParts : -1);
	m_BlockHighOverAvaliblyFactor.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_BlockHighOverAvaliblyFactor : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_BlockHighOverAvaliblyFactor : FCFG_DEF);

	// OverShared
	m_HideOverSharedParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverSharedParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverSharedParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_HideOverSharedParts : -1);
	m_HideOverSharedMode.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverSharedMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverSharedMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_HideOverSharedMode : -1);
	m_HideOverSharedValue.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_HideOverSharedValue : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverSharedValue : FCFG_DEF);
	m_HideOverSharedCalc.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_HideOverSharedCalc : FCFG_DEF, NeoPrefs.KnownPrefs.m_HideOverSharedCalc, Category && Category->KnownPrefs ? Category->KnownPrefs->m_HideOverSharedCalc : -1);

	m_BlockHighOverSharedParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_BlockHighOverSharedParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_BlockHighOverSharedParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_BlockHighOverSharedParts : -1);
	m_BlockHighOverSharedFactor.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_BlockHighOverSharedFactor : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_BlockHighOverSharedFactor : FCFG_DEF);

	// DontHideUnderAvalibly
	m_DontHideUnderAvaliblyParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_DontHideUnderAvaliblyParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_DontHideUnderAvaliblyParts : -1);
	m_DontHideUnderAvaliblyMode.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_DontHideUnderAvaliblyMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_DontHideUnderAvaliblyMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_DontHideUnderAvaliblyMode : -1);
	m_DontHideUnderAvaliblyValue.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_DontHideUnderAvaliblyValue : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_DontHideUnderAvaliblyValue : FCFG_DEF);

	// Other
	m_ShowAlwaysSomeParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ShowAlwaysSomeParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_ShowAlwaysSomeParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ShowAlwaysSomeParts : -1);
	m_ShowAlwaysSomePartsValue.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ShowAlwaysSomePartsValue : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ShowAlwaysSomePartsValue : FCFG_DEF);

	m_ShowAlwaysIncompleteParts.CVDC(3, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ShowAlwaysIncompleteParts : FCFG_DEF, NeoPrefs.KnownPrefs.m_ShowAlwaysIncompleteParts, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ShowAlwaysIncompleteParts : -1);
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	m_ReleaseMode.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseMode : -1);
	m_ReleaseLevel.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLevel : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLevel : FCFG_DEF);
	m_ReleaseTimer.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseTimer : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseTimer : FCFG_DEF);

	// release limit
	m_ReleaseLimit.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimit : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimit, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimit : -1);
	m_ReleaseLimitMode.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitMode : -1);
	m_ReleaseLimitHigh.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitHigh : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitHigh : FCFG_DEF);
	m_ReleaseLimitLow.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitLow : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitLow : FCFG_DEF);

	m_ReleaseLimitLink.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitLink : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitLink, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitLink : -1);

	m_ReleaseLimitComplete.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitComplete : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitComplete, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitComplete : -1);
	m_ReleaseLimitCompleteMode.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitCompleteMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_ReleaseLimitCompleteMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitCompleteMode : -1);
	m_ReleaseLimitCompleteHigh.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitCompleteHigh : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitCompleteHigh : FCFG_DEF);
	m_ReleaseLimitCompleteLow.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_ReleaseLimitCompleteLow : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_ReleaseLimitCompleteLow : FCFG_DEF);

	// limit
	m_LimitLink.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_LimitLink : FCFG_DEF, NeoPrefs.KnownPrefs.m_LimitLink, Category && Category->KnownPrefs ? Category->KnownPrefs->m_LimitLink : -1);

	// source limit
	m_SourceLimit.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimit : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimit, Category && Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimit : -1);
	m_SourceLimitMode.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitMode : -1);
	m_SourceLimitHigh.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitHigh : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitHigh : FCFG_DEF);
	m_SourceLimitLow.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitLow : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitLow : FCFG_DEF);

	m_SourceLimitLink.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitLink : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitLink, Category && Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitLink : -1);

	m_SourceLimitComplete.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitComplete : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitComplete, Category && Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitComplete : -1);
	m_SourceLimitCompleteMode.CVDC(2, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitCompleteMode : FCFG_DEF, NeoPrefs.KnownPrefs.m_SourceLimitCompleteMode, Category && Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitCompleteMode : -1);
	m_SourceLimitCompleteHigh.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitCompleteHigh : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitCompleteHigh : FCFG_DEF);
	m_SourceLimitCompleteLow.DV(Category == NULL ? FCFG_DEF : Category->KnownPrefs ? Category->KnownPrefs->m_SourceLimitCompleteLow : FCFG_DEF, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_SourceLimitCompleteLow : FCFG_DEF);
	// NEO: SRS END

	for (int i = 1; i < m_paFiles->GetSize(); i++)
	{
		if(!theApp.downloadqueue->IsPartFile((CKnownFile*)(*m_paFiles)[i]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)(*m_paFiles)[i]))
			continue;

		KnownFile = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);

		KnownPrefs = KnownFile->KnownPrefs;

		if(Category && Category != thePrefs.GetCategory(KnownFile->GetCategory()))
		{
			// we have to change the .def to FCFG_UNK becouse we have files form different categories
			// we hat to set .cat to -2 to inform that no cat defaults is available thow cats settings are present
		
			// NEO: IPS - [InteligentPartSharing]
			m_UseInteligentPartSharing.Cat = -2;
			m_InteligentPartSharingTimer.Def = FCFG_UNK;

			m_MaxProzentToHide.Def = FCFG_UNK;

			// OverAvalibly
			m_HideOverAvaliblyParts.Cat = -2;
			m_HideOverAvaliblyMode.Cat = -2;
			m_HideOverAvaliblyValue.Def = FCFG_UNK;

			m_BlockHighOverAvaliblyParts.Cat = -2;
			m_BlockHighOverAvaliblyFactor.Def = FCFG_UNK;

			// OverShared
			m_HideOverSharedParts.Cat = -2;
			m_HideOverSharedMode.Cat = -2;
			m_HideOverSharedValue.Def = FCFG_UNK;
			m_HideOverSharedCalc.Cat = -2;

			m_BlockHighOverSharedParts.Cat = -2;
			m_BlockHighOverSharedFactor.Def = FCFG_UNK;

			// DontHideUnderAvalibly
			m_DontHideUnderAvaliblyParts.Cat = -2;
			m_DontHideUnderAvaliblyMode.Cat = -2;
			m_DontHideUnderAvaliblyValue.Def = FCFG_UNK;

			// Other
			m_ShowAlwaysSomeParts.Cat = -2;
			m_ShowAlwaysSomePartsValue.Def = FCFG_UNK;

			m_ShowAlwaysIncompleteParts.Cat = -2;
			// NEO: IPS END

			// NEO: SRS - [SmartReleaseSharing]
			m_ReleaseMode.Cat = -2;
			m_ReleaseLevel.Def = FCFG_UNK;
			m_ReleaseTimer.Def = FCFG_UNK;

			// release limit
			m_ReleaseLimit.Cat = -2;
			m_ReleaseLimitMode.Cat = -2;
			m_ReleaseLimitHigh.Def = FCFG_UNK;
			m_ReleaseLimitLow.Def = FCFG_UNK;

			m_ReleaseLimitLink.Cat = -2;

			m_ReleaseLimitComplete.Cat = -2;
			m_ReleaseLimitCompleteMode.Cat = -2;
			m_ReleaseLimitCompleteHigh.Def = FCFG_UNK;
			m_ReleaseLimitCompleteLow.Def = FCFG_UNK;

			// limit
			m_LimitLink.Cat = -2;

			// source limit
			m_SourceLimit.Cat = -2;
			m_SourceLimitMode.Cat = -2;
			m_SourceLimitHigh.Def = FCFG_UNK;
			m_SourceLimitLow.Def = FCFG_UNK;

			m_SourceLimitLink.Cat = -2;

			m_SourceLimitComplete.Cat = -2;
			m_SourceLimitCompleteMode.Cat = -2;
			m_SourceLimitCompleteHigh.Def = FCFG_UNK;
			m_SourceLimitCompleteLow.Def = FCFG_UNK;
			// NEO: SRS END

			Category = NULL;
		}

		// we have to change the .Val to FCFG_UNK becouse we have different values
		// NEO: IPS - [InteligentPartSharing]
		if(m_UseInteligentPartSharing.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_UseInteligentPartSharing : FCFG_DEF)) m_UseInteligentPartSharing.Val = FCFG_UNK;
		if(m_InteligentPartSharingTimer.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_InteligentPartSharingTimer : FCFG_DEF)) m_InteligentPartSharingTimer.Val = FCFG_UNK;

		if(m_MaxProzentToHide.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_MaxProzentToHide : FCFG_DEF)) m_MaxProzentToHide.Val = FCFG_UNK;

		// OverAvalibly
		if(m_HideOverAvaliblyParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverAvaliblyParts : FCFG_DEF)) m_HideOverAvaliblyParts.Val = FCFG_UNK;
		if(m_HideOverAvaliblyMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverAvaliblyMode : FCFG_DEF)) m_HideOverAvaliblyMode.Val = FCFG_UNK;
		if(m_HideOverAvaliblyValue.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverAvaliblyValue : FCFG_DEF)) m_HideOverAvaliblyValue.Val = FCFG_UNK;

		if(m_BlockHighOverAvaliblyParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_BlockHighOverAvaliblyParts : FCFG_DEF)) m_BlockHighOverAvaliblyParts.Val = FCFG_UNK;
		if(m_BlockHighOverAvaliblyFactor.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_BlockHighOverAvaliblyFactor : FCFG_DEF)) m_BlockHighOverAvaliblyFactor.Val = FCFG_UNK;

		// OverShared
		if(m_HideOverSharedParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverSharedParts : FCFG_DEF)) m_HideOverSharedParts.Val = FCFG_UNK;
		if(m_HideOverSharedMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverSharedMode : FCFG_DEF)) m_HideOverSharedMode.Val = FCFG_UNK;
		if(m_HideOverSharedValue.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverSharedValue : FCFG_DEF)) m_HideOverSharedValue.Val = FCFG_UNK;
		if(m_HideOverSharedCalc.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_HideOverSharedCalc : FCFG_DEF)) m_HideOverSharedCalc.Val = FCFG_UNK;

		if(m_BlockHighOverSharedParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_BlockHighOverSharedParts : FCFG_DEF)) m_BlockHighOverSharedParts.Val = FCFG_UNK;
		if(m_BlockHighOverSharedFactor.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_BlockHighOverSharedFactor : FCFG_DEF)) m_BlockHighOverSharedFactor.Val = FCFG_UNK;

		// DontHideUnderAvalibly
		if(m_DontHideUnderAvaliblyParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_DontHideUnderAvaliblyParts : FCFG_DEF)) m_DontHideUnderAvaliblyParts.Val = FCFG_UNK;
		if(m_DontHideUnderAvaliblyMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_DontHideUnderAvaliblyMode : FCFG_DEF)) m_DontHideUnderAvaliblyMode.Val = FCFG_UNK;
		if(m_DontHideUnderAvaliblyValue.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_DontHideUnderAvaliblyValue : FCFG_DEF)) m_DontHideUnderAvaliblyValue.Val = FCFG_UNK;

		// Other
		if(m_ShowAlwaysSomeParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ShowAlwaysSomeParts : FCFG_DEF)) m_ShowAlwaysSomeParts.Val = FCFG_UNK;
		if(m_ShowAlwaysSomePartsValue.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ShowAlwaysSomePartsValue : FCFG_DEF)) m_ShowAlwaysSomePartsValue.Val = FCFG_UNK;

		if(m_ShowAlwaysIncompleteParts.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ShowAlwaysIncompleteParts : FCFG_DEF)) m_ShowAlwaysIncompleteParts.Val = FCFG_UNK;
		// NEO: IPS END

		// NEO: SRS - [SmartReleaseSharing]
		if(m_ReleaseMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseMode : FCFG_DEF)) m_ReleaseMode.Val = FCFG_UNK;
		if(m_ReleaseLevel.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLevel : FCFG_DEF)) m_ReleaseLevel.Val = FCFG_UNK;
		if(m_ReleaseTimer.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseTimer : FCFG_DEF)) m_ReleaseTimer.Val = FCFG_UNK;

		// release limit
		if(m_ReleaseLimit.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimit : FCFG_DEF)) m_ReleaseLimit.Val = FCFG_UNK;
		if(m_ReleaseLimitMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitMode : FCFG_DEF)) m_ReleaseLimitMode.Val = FCFG_UNK;
		if(m_ReleaseLimitHigh.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitHigh : FCFG_DEF)) m_ReleaseLimitHigh.Val = FCFG_UNK;
		if(m_ReleaseLimitLow.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitLow : FCFG_DEF)) m_ReleaseLimitLow.Val = FCFG_UNK;

		if(m_ReleaseLimitLink.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitLink : FCFG_DEF)) m_ReleaseLimitLink.Val = FCFG_UNK;

		if(m_ReleaseLimitComplete.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitComplete : FCFG_DEF)) m_ReleaseLimitComplete.Val = FCFG_UNK;
		if(m_ReleaseLimitCompleteMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitCompleteMode : FCFG_DEF)) m_ReleaseLimitCompleteMode.Val = FCFG_UNK;
		if(m_ReleaseLimitCompleteHigh.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitCompleteHigh : FCFG_DEF)) m_ReleaseLimitCompleteHigh.Val = FCFG_UNK;
		if(m_ReleaseLimitCompleteLow.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_ReleaseLimitCompleteLow : FCFG_DEF)) m_ReleaseLimitCompleteLow.Val = FCFG_UNK;

		// limit
		if(m_LimitLink.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_LimitLink : FCFG_DEF)) m_LimitLink.Val = FCFG_UNK;

		// source limit
		if(m_SourceLimit.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimit : FCFG_DEF)) m_SourceLimit.Val = FCFG_UNK;
		if(m_SourceLimitMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitMode : FCFG_DEF)) m_SourceLimitMode.Val = FCFG_UNK;
		if(m_SourceLimitHigh.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitHigh : FCFG_DEF)) m_SourceLimitHigh.Val = FCFG_UNK;
		if(m_SourceLimitLow.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitLow : FCFG_DEF)) m_SourceLimitLow.Val = FCFG_UNK;

		if(m_SourceLimitLink.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitLink : FCFG_DEF)) m_SourceLimitLink.Val = FCFG_UNK;

		if(m_SourceLimitComplete.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitComplete : FCFG_DEF)) m_SourceLimitComplete.Val = FCFG_UNK;
		if(m_SourceLimitCompleteMode.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitCompleteMode : FCFG_DEF)) m_SourceLimitCompleteMode.Val = FCFG_UNK;
		if(m_SourceLimitCompleteHigh.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitCompleteHigh : FCFG_DEF)) m_SourceLimitCompleteHigh.Val = FCFG_UNK;
		if(m_SourceLimitCompleteLow.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_SourceLimitCompleteLow : FCFG_DEF)) m_SourceLimitCompleteLow.Val = FCFG_UNK;
		// NEO: SRS END
	}

	// set current aux Set
	SetAuxSet(false);

	UpdateData(FALSE);
}

void CPPgRelease::GetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* /*PartPrefs*/, bool OperateData)
{
	if(OperateData)
		UpdateData();

	if(KnownPrefs)
	{
		// NEO: IPS - [InteligentPartSharing]
		if(m_UseInteligentPartSharing.Val != FCFG_UNK) KnownPrefs->m_UseInteligentPartSharing = m_UseInteligentPartSharing.Val;
		if(m_InteligentPartSharingTimer.Val != FCFG_UNK) KnownPrefs->m_InteligentPartSharingTimer = m_InteligentPartSharingTimer.Val;

		if(m_MaxProzentToHide.Val != FCFG_UNK) KnownPrefs->m_MaxProzentToHide = m_MaxProzentToHide.Val;

		// OverAvalibly
		if(m_HideOverAvaliblyParts.Val != FCFG_UNK) KnownPrefs->m_HideOverAvaliblyParts = m_HideOverAvaliblyParts.Val;
		if(m_HideOverAvaliblyMode.Val != FCFG_UNK) KnownPrefs->m_HideOverAvaliblyMode = m_HideOverAvaliblyMode.Val;
		if(m_HideOverAvaliblyValue.Val != FCFG_UNK) KnownPrefs->m_HideOverAvaliblyValue = m_HideOverAvaliblyValue.Val;

		if(m_BlockHighOverAvaliblyParts.Val != FCFG_UNK) KnownPrefs->m_BlockHighOverAvaliblyParts = m_BlockHighOverAvaliblyParts.Val;
		if(m_BlockHighOverAvaliblyFactor.Val != FCFG_UNK) KnownPrefs->m_BlockHighOverAvaliblyFactor = m_BlockHighOverAvaliblyFactor.Val;

		// OverShared
		if(m_HideOverSharedParts.Val != FCFG_UNK) KnownPrefs->m_HideOverSharedParts = m_HideOverSharedParts.Val;
		if(m_HideOverSharedMode.Val != FCFG_UNK) KnownPrefs->m_HideOverSharedMode = m_HideOverSharedMode.Val;
		if(m_HideOverSharedValue.Val != FCFG_UNK) KnownPrefs->m_HideOverSharedValue = m_HideOverSharedValue.Val;
		if(m_HideOverSharedCalc.Val != FCFG_UNK) KnownPrefs->m_HideOverSharedCalc = m_HideOverSharedCalc.Val;

		if(m_BlockHighOverSharedParts.Val != FCFG_UNK) KnownPrefs->m_BlockHighOverSharedParts = m_BlockHighOverSharedParts.Val;
		if(m_BlockHighOverSharedFactor.Val != FCFG_UNK) KnownPrefs->m_BlockHighOverSharedFactor = m_BlockHighOverSharedFactor.Val;

		// DontHideUnderAvalibly
		if(m_DontHideUnderAvaliblyParts.Val != FCFG_UNK) KnownPrefs->m_DontHideUnderAvaliblyParts = m_DontHideUnderAvaliblyParts.Val;
		if(m_DontHideUnderAvaliblyMode.Val != FCFG_UNK) KnownPrefs->m_DontHideUnderAvaliblyMode = m_DontHideUnderAvaliblyMode.Val;
		if(m_DontHideUnderAvaliblyValue.Val != FCFG_UNK) KnownPrefs->m_DontHideUnderAvaliblyValue = m_DontHideUnderAvaliblyValue.Val;

		// Other
		if(m_ShowAlwaysSomeParts.Val != FCFG_UNK) KnownPrefs->m_ShowAlwaysSomeParts = m_ShowAlwaysSomeParts.Val;
		if(m_ShowAlwaysSomePartsValue.Val != FCFG_UNK) KnownPrefs->m_ShowAlwaysSomePartsValue = m_ShowAlwaysSomePartsValue.Val;

		if(m_ShowAlwaysIncompleteParts.Val != FCFG_UNK) KnownPrefs->m_ShowAlwaysIncompleteParts = m_ShowAlwaysIncompleteParts.Val;
		// NEO: IPS END

		// NEO: SRS - [SmartReleaseSharing]
		if(m_ReleaseMode.Val != FCFG_UNK) KnownPrefs->m_ReleaseMode = m_ReleaseMode.Val;
		if(m_ReleaseLevel.Val != FCFG_UNK) KnownPrefs->m_ReleaseLevel = m_ReleaseLevel.Val;
		if(m_ReleaseTimer.Val != FCFG_UNK) KnownPrefs->m_ReleaseTimer = m_ReleaseTimer.Val;

		// release limit
		if(m_ReleaseLimit.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimit = m_ReleaseLimit.Val;
		if(m_ReleaseLimitMode.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitMode = m_ReleaseLimitMode.Val;
		if(m_ReleaseLimitHigh.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitHigh = m_ReleaseLimitHigh.Val;
		if(m_ReleaseLimitLow.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitLow = m_ReleaseLimitLow.Val;

		if(m_ReleaseLimitLink.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitLink = m_ReleaseLimitLink.Val;

		if(m_ReleaseLimitComplete.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitComplete = m_ReleaseLimitComplete.Val;
		if(m_ReleaseLimitCompleteMode.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitCompleteMode = m_ReleaseLimitCompleteMode.Val;
		if(m_ReleaseLimitCompleteHigh.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitCompleteHigh = m_ReleaseLimitCompleteHigh.Val;
		if(m_ReleaseLimitCompleteLow.Val != FCFG_UNK) KnownPrefs->m_ReleaseLimitCompleteLow = m_ReleaseLimitCompleteLow.Val;

		// limit
		if(m_LimitLink.Val != FCFG_UNK) KnownPrefs->m_LimitLink = m_LimitLink.Val;

		// source limit
		if(m_SourceLimit.Val != FCFG_UNK) KnownPrefs->m_SourceLimit = m_SourceLimit.Val;
		if(m_SourceLimitMode.Val != FCFG_UNK) KnownPrefs->m_SourceLimitMode = m_SourceLimitMode.Val;
		if(m_SourceLimitHigh.Val != FCFG_UNK) KnownPrefs->m_SourceLimitHigh = m_SourceLimitHigh.Val;
		if(m_SourceLimitLow.Val != FCFG_UNK) KnownPrefs->m_SourceLimitLow = m_SourceLimitLow.Val;

		if(m_SourceLimitLink.Val != FCFG_UNK) KnownPrefs->m_SourceLimitLink = m_SourceLimitLink.Val;

		if(m_SourceLimitComplete.Val != FCFG_UNK) KnownPrefs->m_SourceLimitComplete = m_SourceLimitComplete.Val;
		if(m_SourceLimitCompleteMode.Val != FCFG_UNK) KnownPrefs->m_SourceLimitCompleteMode = m_SourceLimitCompleteMode.Val;
		if(m_SourceLimitCompleteHigh.Val != FCFG_UNK) KnownPrefs->m_SourceLimitCompleteHigh = m_SourceLimitCompleteHigh.Val;
		if(m_SourceLimitCompleteLow.Val != FCFG_UNK) KnownPrefs->m_SourceLimitCompleteLow = m_SourceLimitCompleteLow.Val;
		// NEO: SRS END
	}
}

void CPPgRelease::SetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* /*PartPrefs*/, bool OperateData)
{
	if(KnownPrefs)
	{
		// NEO: IPS - [InteligentPartSharing]
		m_UseInteligentPartSharing.Val = KnownPrefs->m_UseInteligentPartSharing;
		m_InteligentPartSharingTimer.Val = KnownPrefs->m_InteligentPartSharingTimer;

		m_MaxProzentToHide.Val = KnownPrefs->m_MaxProzentToHide;

		// OverAvalibly
		m_HideOverAvaliblyParts.Val = KnownPrefs->m_HideOverAvaliblyParts;
		m_HideOverAvaliblyMode.Val = KnownPrefs->m_HideOverAvaliblyMode;
		m_HideOverAvaliblyValue.Val = KnownPrefs->m_HideOverAvaliblyValue;

		m_BlockHighOverAvaliblyParts.Val = KnownPrefs->m_BlockHighOverAvaliblyParts;
		m_BlockHighOverAvaliblyFactor.Val = KnownPrefs->m_BlockHighOverAvaliblyFactor;

		// OverShared
		m_HideOverSharedParts.Val = KnownPrefs->m_HideOverSharedParts;
		m_HideOverSharedMode.Val = KnownPrefs->m_HideOverSharedMode;
		m_HideOverSharedValue.Val = KnownPrefs->m_HideOverSharedValue;
		m_HideOverSharedCalc.Val = KnownPrefs->m_HideOverSharedCalc;

		m_BlockHighOverSharedParts.Val = KnownPrefs->m_BlockHighOverSharedParts;
		m_BlockHighOverSharedFactor.Val = KnownPrefs->m_BlockHighOverSharedFactor;

		// DontHideUnderAvalibly
		m_DontHideUnderAvaliblyParts.Val = KnownPrefs->m_DontHideUnderAvaliblyParts;
		m_DontHideUnderAvaliblyMode.Val = KnownPrefs->m_DontHideUnderAvaliblyMode;
		m_DontHideUnderAvaliblyValue.Val = KnownPrefs->m_DontHideUnderAvaliblyValue;

		// Other
		m_ShowAlwaysSomeParts.Val = KnownPrefs->m_ShowAlwaysSomeParts;
		m_ShowAlwaysSomePartsValue.Val = KnownPrefs->m_ShowAlwaysSomePartsValue;

		m_ShowAlwaysIncompleteParts.Val = KnownPrefs->m_ShowAlwaysIncompleteParts;
		// NEO: IPS END

		// NEO: SRS - [SmartReleaseSharing]
		m_ReleaseMode.Val = KnownPrefs->m_ReleaseMode;
		m_ReleaseLevel.Val = KnownPrefs->m_ReleaseLevel;
		m_ReleaseTimer.Val = KnownPrefs->m_ReleaseTimer;

		// release limit
		m_ReleaseLimit.Val = KnownPrefs->m_ReleaseLimit;
		m_ReleaseLimitMode.Val = KnownPrefs->m_ReleaseLimitMode;
		m_ReleaseLimitHigh.Val = KnownPrefs->m_ReleaseLimitHigh;
		m_ReleaseLimitLow.Val = KnownPrefs->m_ReleaseLimitLow;

		m_ReleaseLimitLink.Val = KnownPrefs->m_ReleaseLimitLink;

		m_ReleaseLimitComplete.Val = KnownPrefs->m_ReleaseLimitComplete;
		m_ReleaseLimitCompleteMode.Val = KnownPrefs->m_ReleaseLimitCompleteMode;
		m_ReleaseLimitCompleteHigh.Val = KnownPrefs->m_ReleaseLimitCompleteHigh;
		m_ReleaseLimitCompleteLow.Val = KnownPrefs->m_ReleaseLimitCompleteLow;

		// limit
		m_LimitLink.Val = KnownPrefs->m_LimitLink;

		// source limit
		m_SourceLimit.Val = KnownPrefs->m_SourceLimit;
		m_SourceLimitMode.Val = KnownPrefs->m_SourceLimitMode;
		m_SourceLimitHigh.Val = KnownPrefs->m_SourceLimitHigh;
		m_SourceLimitLow.Val = KnownPrefs->m_SourceLimitLow;

		m_SourceLimitLink.Val = KnownPrefs->m_SourceLimitLink;

		m_SourceLimitComplete.Val = KnownPrefs->m_SourceLimitComplete;
		m_SourceLimitCompleteMode.Val = KnownPrefs->m_SourceLimitCompleteMode;
		m_SourceLimitCompleteHigh.Val = KnownPrefs->m_SourceLimitCompleteHigh;
		m_SourceLimitCompleteLow.Val = KnownPrefs->m_SourceLimitCompleteLow;
		// NEO: SRS END
	}

	if(OperateData)
	{
		UpdateData(FALSE);
		SetModified();
	}
}
// NEO: FCFG END

BOOL CPPgRelease::OnApply()
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
		CKnownFile* KnownFile;
		CKnownPreferences* KnownPrefs;
		for (int i = 0; i < m_paFiles->GetSize(); i++)
		{
			// check if the file is still valid
			if(!theApp.downloadqueue->IsPartFile((CKnownFile*)(*m_paFiles)[i]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)(*m_paFiles)[i]))
				continue;

			KnownFile = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);

			// get ot create Preferences 
			KnownPrefs = KnownFile->KnownPrefs;
			if(!KnownPrefs->IsFilePrefs())
				KnownPrefs = new CKnownPreferencesEx(CFP_FILE);

			// save preferences
			GetFilePreferences(KnownPrefs,NULL);

			// check validiti of the tweaks
			KnownPrefs->CheckTweaks();

			// valiate pointers and update (!)
			KnownFile->UpdateKnownPrefs(KnownPrefs);
		}

		RefreshData();
	}
	else if(m_Category)
	{
		int cat = thePrefs.FindCategory(m_Category);
		if(cat != -1)
		{
			// get ot create Preferences 
			CKnownPreferences* KnownPrefs = m_Category->KnownPrefs;
			if(KnownPrefs == NULL)
				KnownPrefs = new CKnownPreferencesEx(CFP_CATEGORY);

			// save preferences
			GetFilePreferences(KnownPrefs, NULL);

			// check validiti of the tweaks
			KnownPrefs->CheckTweaks();

			// valiate pointers and update (!)
			theApp.knownfiles->UpdateKnownPrefs(KnownPrefs, (UINT)cat); // may delete KnownPrefs

			thePrefs.SaveCats();

			LoadSettings();
		}
		else
			AfxMessageBox(GetResString(IDS_X_INVALID_CAT),MB_OK | MB_ICONSTOP, NULL);
	}
	else
	// NEO: FCFG END
	{
		// NEO: NPT - [NeoPartTraffic]
		UINT OldNPTState = NeoPrefs.m_uUsePartTraffic; 
		int OldColor = NeoPrefs.m_iPartTrafficCollors;
		NeoPrefs.m_uUsePartTraffic = m_uUsePartTraffic; 
		NeoPrefs.m_iPartTrafficCollors = m_iPartTrafficCollors; 
		if(OldColor != m_iPartTrafficCollors)
			theApp.emuledlg->sharedfileswnd->sharedfilesctrl.SetColoring(m_iPartTrafficCollors);
		if(OldNPTState != NeoPrefs.m_uUsePartTraffic) {
			if(NeoPrefs.m_uUsePartTraffic && !theApp.knownfiles->IsPartTrafficLoaded())
				theApp.knownfiles->LoadPartTraffic();
			if(NeoPrefs.m_uUsePartTraffic == 1)
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.ShowColumn(13);
			else
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.HideColumn(13);
			if(NeoPrefs.m_uUsePartTraffic)
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.ShowColumn(14);
			else
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.HideColumn(14);
		}
		// NEO: NPT END

		NeoPrefs.m_bSaveUploadQueueWaitTime = m_bSaveUploadQueueWaitTime; // NEO: SQ - [SaveUploadQueue]
		NeoPrefs.m_bUseMultiQueue = m_bUseMultiQueue; // NEO: MQ - [MultiQueue]
		NeoPrefs.m_uUseRandomQueue = m_uUseRandomQueue; // NEO: RQ - [RandomQueue]

		NeoPrefs.m_bNeoScoreSystem = m_bNeoScoreSystem; // NEO: NFS - [NeoScoreSystem]
		// NEO: OCS - [OtherCreditSystems]
		if(m_iCreditSystem == 0){
			NeoPrefs.m_bNeoCreditSystem = false;
			NeoPrefs.m_iOtherCreditSystem = 0;
		}else if(m_iCreditSystem == 1){
			NeoPrefs.m_bNeoCreditSystem = true;
			NeoPrefs.m_iOtherCreditSystem = 0;
		}else{
			NeoPrefs.m_bNeoCreditSystem = false;
			NeoPrefs.m_iOtherCreditSystem = m_iOtherCreditSystem;
		}
		// NEO: OCS END

		// NEO: TQ - [TweakUploadQueue]
		NeoPrefs.m_bUseInfiniteQueue = m_bUseInfiniteQueue;

		NeoPrefs.m_uQueueOverFlowRelease = m_uQueueOverFlowRelease;
		if(m_iQueueOverFlowRelease) NeoPrefs.m_iQueueOverFlowRelease = m_iQueueOverFlowRelease;

		NeoPrefs.m_uQueueOverFlowEx = m_uQueueOverFlowEx;
		if(m_iQueueOverFlowEx) NeoPrefs.m_iQueueOverFlowEx = m_iQueueOverFlowEx;

		NeoPrefs.m_uQueueOverFlowDef = m_uQueueOverFlowDef;
		if(m_iQueueOverFlowDef) NeoPrefs.m_iQueueOverFlowDef = m_iQueueOverFlowDef;

		NeoPrefs.m_uQueueOverFlowCF = m_uQueueOverFlowCF;
		if(m_iQueueOverFlowCF) NeoPrefs.m_iQueueOverFlowCF = m_iQueueOverFlowCF;
		// NEO: TQ END

		// NEO: PRSF - [PushSmallRareFiles]
		NeoPrefs.m_bPushSmallFiles = m_bPushSmallFiles;
		if(m_iPushSmallFilesSize) NeoPrefs.m_iPushSmallFilesSize = m_iPushSmallFilesSize;

		NeoPrefs.m_bPushRareFiles = m_bPushRareFiles;
		if(m_iPushRareFilesValue) NeoPrefs.m_iPushRareFilesValue = m_iPushRareFilesValue;

		NeoPrefs.m_bPushRatioFiles = m_bPushRatioFiles;
		if(m_iPushRatioFilesValue) NeoPrefs.m_iPushRatioFilesValue = m_iPushRatioFilesValue;
		// NEO: PRSF END

		// NEO: NMFS - [NiceMultiFriendSlots]
		NeoPrefs.m_uFriendSlotLimit = m_uFriendSlotLimit;
		if(m_iFriendSlotLimit) NeoPrefs.m_iFriendSlotLimit = m_iFriendSlotLimit;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
		NeoPrefs.m_uSeparateFriendBandwidth = m_uSeparateFriendBandwidth;
		if(m_fFriendSlotSpeed) NeoPrefs.m_fFriendSlotSpeed = m_fFriendSlotSpeed;
		if(m_fFriendBandwidthPercentage) NeoPrefs.m_fFriendBandwidthPercentage = m_fFriendBandwidthPercentage;
#endif // BW_MOD // NEO: BM END
		// NEO: NMFS END

		// NEO: SRS - [SmartReleaseSharing]
		if(m_iReleaseChunks) NeoPrefs.m_iReleaseChunks = m_iReleaseChunks;
		NeoPrefs.m_uReleaseSlotLimit = m_uReleaseSlotLimit;
		if(m_iReleaseSlotLimit) NeoPrefs.m_iReleaseSlotLimit = m_iReleaseSlotLimit;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
		NeoPrefs.m_uSeparateReleaseBandwidth = m_uSeparateReleaseBandwidth;
		if(m_fReleaseSlotSpeed) NeoPrefs.m_fReleaseSlotSpeed = m_fReleaseSlotSpeed;
		if(m_fReleaseBandwidthPercentage) NeoPrefs.m_fReleaseBandwidthPercentage = m_fReleaseBandwidthPercentage;
#endif // BW_MOD // NEO: BM END
		// NEO: SRS END

		GetFilePreferences(&NeoPrefs.KnownPrefs, NULL);

		NeoPrefs.KnownPrefs.CheckTweaks();

		LoadSettings();
	}


	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

// NEO: FCFG - [FileConfiguration]
void CPPgRelease::OnTimer(UINT /*nIDEvent*/)
{
	if (m_bDataChanged)
	{
		if(m_paFiles)
			RefreshData();
		m_bDataChanged = false;
	}
}

BOOL CPPgRelease::OnSetActive()
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

LRESULT CPPgRelease::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}
// NEO: FCFG END

BOOL CPPgRelease::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgRelease::OnDestroy()
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

LRESULT CPPgRelease::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){

			// NEO: TQ - [TweakUploadQueue]
			if(m_htiQueueOverFlowRelease && pton->hItem == m_htiQueueOverFlowRelease){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiQueueOverFlowRelease, MIN_QUEUE_OVERFLOW, DEF_QUEUE_OVERFLOW, MAX_QUEUE_OVERFLOW)) SetModified();
			}else if(m_htiQueueOverFlowEx && pton->hItem == m_htiQueueOverFlowEx){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiQueueOverFlowEx, MIN_QUEUE_OVERFLOW, DEF_QUEUE_OVERFLOW, MAX_QUEUE_OVERFLOW)) SetModified();
			}else if(m_htiQueueOverFlowDef && pton->hItem == m_htiQueueOverFlowDef){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiQueueOverFlowDef, MIN_QUEUE_OVERFLOW, DEF_QUEUE_OVERFLOW, MAX_QUEUE_OVERFLOW)) SetModified();
			}else if(m_htiQueueOverFlowCF && pton->hItem == m_htiQueueOverFlowCF){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiQueueOverFlowCF, MIN_QUEUE_OVERFLOW, DEF_QUEUE_OVERFLOW, MAX_QUEUE_OVERFLOW)) SetModified();
			}
			// NEO: TQ END

			// NEO: NMFS - [NiceMultiFriendSlots]
			else if(m_htiFriendSlotLimit && pton->hItem == m_htiFriendSlotLimit){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFriendSlotLimit, MIN_FRIEND_SLOT_LIMIT, DEF_FRIEND_SLOT_LIMIT, MAX_FRIEND_SLOT_LIMIT)) SetModified();
			}
			// NEO: NMFS END

#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			else if(m_htiFriendSlotSpeed && pton->hItem == m_htiFriendSlotSpeed){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFriendSlotSpeed, MIN_FRIEND_SLOT_SPEED, DEF_FRIEND_SLOT_SPEED, MAX_FRIEND_SLOT_SPEED)) SetModified();
			}
			else if(m_htiFriendBandwidthPercentage && pton->hItem == m_htiFriendBandwidthPercentage){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiFriendBandwidthPercentage, MIN_FRIEND_BANDWIDTH_PERCENTAGE, DEF_FRIEND_BANDWIDTH_PERCENTAGE, MAX_FRIEND_BANDWIDTH_PERCENTAGE)) SetModified();
			}
			else if(m_htiReleaseSlotSpeed && pton->hItem == m_htiReleaseSlotSpeed){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseSlotSpeed, MIN_RELEASE_SLOT_SPEED, DEF_RELEASE_SLOT_SPEED, MAX_RELEASE_SLOT_SPEED)) SetModified();
			}
			else if(m_htiReleaseBandwidthPercentage && pton->hItem == m_htiReleaseBandwidthPercentage){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseBandwidthPercentage, MIN_RELEASE_BANDWIDTH_PERCENTAGE, DEF_RELEASE_BANDWIDTH_PERCENTAGE, MAX_RELEASE_BANDWIDTH_PERCENTAGE)) SetModified();
			}
#endif // BW_MOD // NEO: BM END

			// NEO: IPS - [InteligentPartSharing]
			if(m_htiInteligentPartSharingTimer && pton->hItem == m_htiInteligentPartSharingTimer){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiInteligentPartSharingTimer, m_InteligentPartSharingTimer)) SetModified();
			}
			if(m_htiMaxProzentToHide && pton->hItem == m_htiMaxProzentToHide){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxProzentToHide, m_MaxProzentToHide)) SetModified();
			}

			// OverAvalibly
			if(m_htiHideOverAvaliblyValue && pton->hItem == m_htiHideOverAvaliblyValue){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiHideOverAvaliblyValue, m_HideOverAvaliblyValue)) SetModified();
			}
			if(m_htiBlockHighOverAvaliblyFactor && pton->hItem == m_htiBlockHighOverAvaliblyFactor){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBlockHighOverAvaliblyFactor, m_BlockHighOverAvaliblyFactor)) SetModified();
			}

			// OverShared
			if(m_htiHideOverSharedValue && pton->hItem == m_htiHideOverSharedValue){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiHideOverSharedValue, m_HideOverSharedValue)) SetModified();
			}
			if(m_htiBlockHighOverSharedFactor && pton->hItem == m_htiBlockHighOverSharedFactor){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBlockHighOverSharedFactor, m_BlockHighOverSharedFactor)) SetModified();
			}

			// DontHideUnderAvalibly
			if(m_htiDontHideUnderAvaliblyValue && pton->hItem == m_htiDontHideUnderAvaliblyValue){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDontHideUnderAvaliblyValue, m_DontHideUnderAvaliblyValue)) SetModified();
			}

			// Other
			if(m_htiShowAlwaysSomePartsValue && pton->hItem == m_htiShowAlwaysSomePartsValue){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiShowAlwaysSomePartsValue, m_ShowAlwaysSomePartsValue)) SetModified();
			}
			// NEO: IPS END

			// NEO: SRS - [SmartReleaseSharing]
			if(m_htiReleaseLevel && pton->hItem == m_htiReleaseLevel){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseLevel, m_ReleaseLevel)) SetModified();
			}
			else if(m_htiReleaseSlotLimit && pton->hItem == m_htiReleaseSlotLimit){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseSlotLimit, MIN_RELEASE_SLOT_LIMIT, DEF_RELEASE_SLOT_LIMIT, MAX_RELEASE_SLOT_LIMIT)) SetModified();
			}
			if(m_htiReleaseTimer && pton->hItem == m_htiReleaseTimer){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseTimer, m_ReleaseTimer)) SetModified();
			}
			else if(m_htiReleaseSlotLimit && pton->hItem == m_htiReleaseChunks){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseChunks, MIN_RELEASE_CHUNKS, DEF_RELEASE_CHUNKS, MAX_RELEASE_CHUNKS)) SetModified();
			}
			
			// release limit
			if(m_htiReleaseLimitHigh && pton->hItem == m_htiReleaseLimitHigh){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseLimitHigh, m_ReleaseLimitHigh)) SetModified();
			}
			if(m_htiReleaseLimitLow && pton->hItem == m_htiReleaseLimitLow){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseLimitLow, m_ReleaseLimitLow)) SetModified();
			}

			if(m_htiReleaseLimitCompleteHigh && pton->hItem == m_htiReleaseLimitCompleteHigh){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseLimitCompleteHigh, m_ReleaseLimitCompleteHigh)) SetModified();
			}
			if(m_htiReleaseLimitCompleteLow && pton->hItem == m_htiReleaseLimitCompleteLow){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReleaseLimitCompleteLow, m_ReleaseLimitCompleteLow)) SetModified();
			}

			// source limit
			if(m_htiSourceLimitHigh && pton->hItem == m_htiSourceLimitHigh){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceLimitHigh, m_SourceLimitHigh)) SetModified();
			}
			if(m_htiSourceLimitLow && pton->hItem == m_htiSourceLimitLow){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceLimitLow, m_SourceLimitLow)) SetModified();
			}

			if(m_htiSourceLimitCompleteHigh && pton->hItem == m_htiSourceLimitCompleteHigh){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceLimitCompleteHigh, m_SourceLimitCompleteHigh)) SetModified();
			}
			if(m_htiSourceLimitCompleteLow && pton->hItem == m_htiSourceLimitCompleteLow){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiSourceLimitCompleteLow, m_SourceLimitCompleteLow)) SetModified();
			}
			// NEO: SRS END

		}else{
			// NEO: NPT - [NeoPartTraffic]
			UINT bCheck;
			if (m_htiPartTraffic && pton->hItem == m_htiPartTraffic){
				m_ctrlTreeOptions.GetCheckBox(m_htiPartTraffic, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiPartTraffic, bCheck, TRUE, TRUE);
				m_ctrlTreeOptions.SetItemEnable(m_htiHideOverSharedParts,bCheck); // NEO: IPS - [InteligentPartSharing]
				m_ctrlTreeOptions.SetItemEnable(m_htiReleaseLimitComplete,bCheck); // NEO: SRS - [SmartReleaseSharing]
			}
			// NEO: NPT END
			
			// NEO: MQ - [MultiQueue]
			else if(m_htiUseMultiQueue && pton->hItem == m_htiUseMultiQueue){
				m_ctrlTreeOptions.GetCheckBox(m_htiUseMultiQueue, bCheck); 
			}
			// NEO: MQ END
			// NEO: RQ - [RandomQueue]
			else if(m_htiUseRandomQueue && pton->hItem == m_htiUseRandomQueue){
				m_ctrlTreeOptions.GetCheckBox(m_htiUseRandomQueue, bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiNeoScoreSystem, bCheck ? FALSE : TRUE); // ofcorse no need for time based score improvement
			}
			// NEO: RQ END
			// NEO: TQ - [TweakUploadQueue]
			else if(m_htiInfiniteQueue && pton->hItem == m_htiInfiniteQueue){
				m_ctrlTreeOptions.GetCheckBox(m_htiInfiniteQueue, bCheck);
				m_ctrlTreeOptions.SetGroupEnable(m_htiUploadQueueOverFlow, bCheck ? FALSE : TRUE);
			}
			else if(m_htiQueueOverFlowRelease && pton->hItem == m_htiQueueOverFlowRelease){
				m_ctrlTreeOptions.GetCheckBox(m_htiQueueOverFlowRelease, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowRelease, bCheck == 2, TRUE, TRUE);
			}else if(m_htiQueueOverFlowEx && pton->hItem == m_htiQueueOverFlowEx){
				m_ctrlTreeOptions.GetCheckBox(m_htiQueueOverFlowEx, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowEx, bCheck == 2, TRUE, TRUE);
			}else if(m_htiQueueOverFlowDef && pton->hItem == m_htiQueueOverFlowDef){
				m_ctrlTreeOptions.GetCheckBox(m_htiQueueOverFlowDef, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowDef, bCheck == 2, TRUE, TRUE);
			}else if(m_htiQueueOverFlowCF && pton->hItem == m_htiQueueOverFlowCF){
				m_ctrlTreeOptions.GetCheckBox(m_htiQueueOverFlowCF, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiQueueOverFlowCF, bCheck == 2, TRUE, TRUE);
			}else
			// NEO: TQ END

			// NEO: NMFS - [NiceMultiFriendSlots]
			if (m_htiFriendSlotLimit && pton->hItem == m_htiFriendSlotLimit){
				m_ctrlTreeOptions.GetCheckBox(m_htiFriendSlotLimit, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiFriendSlotLimit, bCheck, TRUE, TRUE);
			}else
			// NEO: NMFS END

#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			if (m_htiSeparateFriendBandwidth && pton->hItem == m_htiSeparateFriendBandwidth){
				m_ctrlTreeOptions.GetCheckBox(m_htiSeparateFriendBandwidth, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiFriendSlotSpeed, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiFriendBandwidthPercentage, bCheck);
			}else
			if (m_htiSeparateReleaseBandwidth && pton->hItem == m_htiSeparateReleaseBandwidth){
				m_ctrlTreeOptions.GetCheckBox(m_htiSeparateReleaseBandwidth, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiReleaseSlotSpeed, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiReleaseBandwidthPercentage, bCheck);
			}else
#endif // BW_MOD // NEO: BM END


			// NEO: IPS - [InteligentPartSharing]
			if(pton->hItem != NULL && (pton->hItem == m_htiHideOverAvaliblyModeMultiplicativ) || (pton->hItem == m_htiHideOverAvaliblyModeAdditiv) || (pton->hItem == m_htiHideOverAvaliblyModeDefault) || (pton->hItem == m_htiHideOverAvaliblyModeGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiHideOverAvaliblyValue ,m_HideOverAvaliblyMode.Cut, m_HideOverAvaliblyValue,m_Category == NULL && m_paFiles == NULL, m_htiHideOverAvaliblyMode);
			}else
			if(pton->hItem != NULL && (pton->hItem == m_htiHideOverSharedModeMultiplicativ) || (pton->hItem == m_htiHideOverSharedModeAdditiv) || (pton->hItem == m_htiHideOverSharedModeDefault) || (pton->hItem == m_htiHideOverSharedModeGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiHideOverSharedValue ,m_HideOverSharedMode.Cut, m_HideOverSharedValue,m_Category == NULL && m_paFiles == NULL,m_htiHideOverSharedMode);
			}else
			if(pton->hItem != NULL && (pton->hItem == m_htiDontHideUnderAvaliblyModeMultiplicativ) || (pton->hItem == m_htiDontHideUnderAvaliblyModeAdditiv) || (pton->hItem == m_htiDontHideUnderAvaliblyModeDefault) || (pton->hItem == m_htiDontHideUnderAvaliblyModeGlobal)){
				PrepAuxSet(m_ctrlTreeOptions,m_htiDontHideUnderAvaliblyValue ,m_DontHideUnderAvaliblyMode.Cut, m_DontHideUnderAvaliblyValue,m_Category == NULL && m_paFiles == NULL, m_htiDontHideUnderAvaliblyMode);
			}
			// NEO: IPS END

			// NEO: SRS - [SmartReleaseSharing]
			if (m_htiReleaseSlotLimit && pton->hItem == m_htiReleaseSlotLimit){
				m_ctrlTreeOptions.GetCheckBox(m_htiReleaseSlotLimit, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiReleaseSlotLimit, bCheck, TRUE, TRUE);
			}
			// NEO: SRS END

			SetModified();
		}
	}
	return 0;
}


LRESULT CPPgRelease::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgRelease::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgRelease::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgRelease::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
