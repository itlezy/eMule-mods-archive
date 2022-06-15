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


#pragma once
#include "Neo\GUI\CP\TreeOptionsCtrl.h" // NEO - [TreeControl] <-- Xanatos --
#include "Neo/GUI/CP/TreeFunctions.h"

class CKnownPreferences;
class CPartPreferences;
struct Category_Struct;

class CPPgRelease : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgRelease)

public:
	CPPgRelease();
	virtual ~CPPgRelease();

	// NEO: FCFG - [FileConfiguration]
	void SetFiles(const CSimpleArray<CObject*>* paFiles) { m_paFiles = paFiles; m_bDataChanged = true;}
	void SetCategory(Category_Struct* Category) {m_Category = Category; m_bDataChanged = true;}
	// NEO: FCFG END

	// NEO: FCFG - [FileConfiguration]
	void	GetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs, bool OperateData = false);
	void	SetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs, bool OperateData = false);
	// NEO: FCFG END

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	// NEO: FCFG - [FileConfiguration]
	//CString m_strCaption;
	const CSimpleArray<CObject*>* m_paFiles;
	Category_Struct* m_Category;
	bool m_bDataChanged;
	uint32 m_timer;
	static LPCTSTR sm_pszNotAvail;
	// NEO: FCFG END

	CTreeOptionsCtrl m_ctrlTreeOptions; // NEO - [TreeControl] <-- Xanatos --
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiPartTraffic; // NEO: NPT - [NeoPartTraffic]
	HTREEITEM m_htiTweakUploadQueue;
		HTREEITEM m_htiSaveUploadQueueWaitTime; // NEO: SQ - [SaveUploadQueue]
		HTREEITEM m_htiUseMultiQueue; // NEO: MQ - [MultiQueue]
		HTREEITEM m_htiNeoScoreSystem; // NEO: NFS - [NeoScoreSystem]
		// NEO: OCS - [OtherCreditSystems]
		HTREEITEM m_htiCreditSystem;
			HTREEITEM m_htiOfficialCreditSystem;
			HTREEITEM m_htiNeoCreditSystem; // NEO: NCS - [NeoCreditSystem]
			HTREEITEM m_htiOtherCreditSystem; 
		// NEO: OCS END
		HTREEITEM m_htiUseRandomQueue; // NEO: RQ - [RandomQueue]
		// NEO: TQ - [TweakUploadQueue]
		HTREEITEM m_htiInfiniteQueue;
		HTREEITEM m_htiUploadQueueOverFlow;
			HTREEITEM m_htiQueueOverFlowDef;
			HTREEITEM m_htiQueueOverFlowEx;
			HTREEITEM m_htiQueueOverFlowRelease;
			HTREEITEM m_htiQueueOverFlowCF;
		// NEO: TQ END
		// NEO: PRSF - [PushSmallRareFiles]
		HTREEITEM m_htiFilePushTweaks;
			HTREEITEM m_htiPushSmallFiles;
			HTREEITEM m_htiPushRareFiles;
			HTREEITEM m_htiPushRatioFiles;
		// NEO: PRSF END
		// NEO: NMFS - [NiceMultiFriendSlots]
		HTREEITEM m_htiFriendUpload;
			HTREEITEM m_htiFriendSlotLimit;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			HTREEITEM m_htiSeparateFriendBandwidth;
				HTREEITEM m_htiFriendSlotSpeed;
				HTREEITEM m_htiFriendBandwidthPercentage;
#endif // BW_MOD // NEO: BM END
		// NEO: NMFS END
	// NEO: IPS - [InteligentPartSharing]
	HTREEITEM m_htiInteligentPartSharing;
		HTREEITEM m_htiInteligentPartSharingDisable;
		HTREEITEM m_htiInteligentPartSharingEnable;
		HTREEITEM m_htiInteligentPartSharingDefault;
		HTREEITEM m_htiInteligentPartSharingGlobal;

		HTREEITEM m_htiInteligentPartSharingTimer;
		HTREEITEM m_htiMaxProzentToHide;

	// OverAvalibly
		HTREEITEM m_htiHideOverAvaliblyParts;
			HTREEITEM m_htiHideOverAvaliblyPartsDisable;
			HTREEITEM m_htiHideOverAvaliblyPartsOnleKF;
			HTREEITEM m_htiHideOverAvaliblyPartsEnable;
			HTREEITEM m_htiHideOverAvaliblyPartsOnlyPF;
			HTREEITEM m_htiHideOverAvaliblyPartsDefault;
			HTREEITEM m_htiHideOverAvaliblyPartsGlobal;
	
			HTREEITEM m_htiHideOverAvaliblyMode;
				HTREEITEM m_htiHideOverAvaliblyModeMultiplicativ;
				HTREEITEM m_htiHideOverAvaliblyModeAdditiv;
				HTREEITEM m_htiHideOverAvaliblyModeDefault;
				HTREEITEM m_htiHideOverAvaliblyModeGlobal;

			HTREEITEM m_htiHideOverAvaliblyValue;

			HTREEITEM m_htiBlockHighOverAvaliblyParts;
				HTREEITEM m_htiBlockHighOverAvaliblyPartsDisable;
				HTREEITEM m_htiBlockHighOverAvaliblyPartsOnleKF;
				HTREEITEM m_htiBlockHighOverAvaliblyPartsEnable;
				HTREEITEM m_htiBlockHighOverAvaliblyPartsOnlyPF;
				HTREEITEM m_htiBlockHighOverAvaliblyPartsDefault;
				HTREEITEM m_htiBlockHighOverAvaliblyPartsGlobal;

				HTREEITEM m_htiBlockHighOverAvaliblyFactor;

	// OverShared
		HTREEITEM m_htiHideOverSharedParts;
			HTREEITEM m_htiHideOverSharedPartsDisable;
			HTREEITEM m_htiHideOverSharedPartsOnleKF;
			HTREEITEM m_htiHideOverSharedPartsEnable;
			HTREEITEM m_htiHideOverSharedPartsOnlyPF;
			HTREEITEM m_htiHideOverSharedPartsDefault;
			HTREEITEM m_htiHideOverSharedPartsGlobal;
	
			HTREEITEM m_htiHideOverSharedMode;
				HTREEITEM m_htiHideOverSharedModeMultiplicativ;
				HTREEITEM m_htiHideOverSharedModeAdditiv;
				HTREEITEM m_htiHideOverSharedModeDefault;
				HTREEITEM m_htiHideOverSharedModeGlobal;

			HTREEITEM m_htiHideOverSharedValue;

			HTREEITEM m_htiHideOverSharedCalc;
				HTREEITEM m_htiHideOverSharedCalcHigh;
				HTREEITEM m_htiHideOverSharedCalcLow;
				HTREEITEM m_htiHideOverSharedCalcDefault;
				HTREEITEM m_htiHideOverSharedCalcGlobal;

			HTREEITEM m_htiBlockHighOverSharedParts;
				HTREEITEM m_htiBlockHighOverSharedPartsDisable;
				HTREEITEM m_htiBlockHighOverSharedPartsOnleKF;
				HTREEITEM m_htiBlockHighOverSharedPartsEnable;
				HTREEITEM m_htiBlockHighOverSharedPartsOnlyPF;
				HTREEITEM m_htiBlockHighOverSharedPartsDefault;
				HTREEITEM m_htiBlockHighOverSharedPartsGlobal;

				HTREEITEM m_htiBlockHighOverSharedFactor;

	// DontHideUnderAvalibly
			HTREEITEM m_htiDontHideUnderAvaliblyParts;
				HTREEITEM m_htiDontHideUnderAvaliblyPartsDisable;
				HTREEITEM m_htiDontHideUnderAvaliblyPartsOnleKF;
				HTREEITEM m_htiDontHideUnderAvaliblyPartsEnable;
				HTREEITEM m_htiDontHideUnderAvaliblyPartsOnlyPF;
				HTREEITEM m_htiDontHideUnderAvaliblyPartsDefault;
				HTREEITEM m_htiDontHideUnderAvaliblyPartsGlobal;
		
				HTREEITEM m_htiDontHideUnderAvaliblyMode;
					HTREEITEM m_htiDontHideUnderAvaliblyModeMultiplicativ;
					HTREEITEM m_htiDontHideUnderAvaliblyModeAdditiv;
					HTREEITEM m_htiDontHideUnderAvaliblyModeDefault;
					HTREEITEM m_htiDontHideUnderAvaliblyModeGlobal;

				HTREEITEM m_htiDontHideUnderAvaliblyValue;

	// Other
			HTREEITEM m_htiShowAlwaysSomeParts;
				HTREEITEM m_htiShowAlwaysSomePartsDisable;
				HTREEITEM m_htiShowAlwaysSomePartsOnleKF;
				HTREEITEM m_htiShowAlwaysSomePartsEnable;
				HTREEITEM m_htiShowAlwaysSomePartsOnlyPF;
				HTREEITEM m_htiShowAlwaysSomePartsDefault;
				HTREEITEM m_htiShowAlwaysSomePartsGlobal;

				HTREEITEM m_htiShowAlwaysSomePartsValue;

			HTREEITEM m_htiShowAlwaysIncompleteParts;
				HTREEITEM m_htiShowAlwaysIncompletePartsDisable;
				HTREEITEM m_htiShowAlwaysIncompletePartsOnleKF;
				HTREEITEM m_htiShowAlwaysIncompletePartsEnable;
				HTREEITEM m_htiShowAlwaysIncompletePartsOnlyPF;
				HTREEITEM m_htiShowAlwaysIncompletePartsDefault;
				HTREEITEM m_htiShowAlwaysIncompletePartsGlobal;
	// NEO: IPS END


	// NEO: SRS - [SmartReleaseSharing]
	HTREEITEM m_htiSmartReleaseSharing;
		HTREEITEM m_htiReleaseModeMixed;
		HTREEITEM m_htiReleaseModeBoost;
		HTREEITEM m_htiReleaseModePower;
		HTREEITEM m_htiReleaseModeDefault;
		HTREEITEM m_htiReleaseModeGlobal;

		HTREEITEM m_htiReleaseLevel;
		HTREEITEM m_htiReleaseUpload;
			HTREEITEM m_htiReleaseSlotLimit;
#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
			HTREEITEM m_htiSeparateReleaseBandwidth;
				HTREEITEM m_htiReleaseSlotSpeed;
				HTREEITEM m_htiReleaseBandwidthPercentage;
#endif // BW_MOD // NEO: BM END
			HTREEITEM m_htiReleaseChunks;
		HTREEITEM m_htiReleaseTimer;

	// release limit
		HTREEITEM m_htiReleaseReleaseLimit;
			HTREEITEM m_htiReleaseLimit;
				HTREEITEM m_htiReleaseLimitDisable;
				HTREEITEM m_htiReleaseLimitSingle;
				HTREEITEM m_htiReleaseLimitBooth;
				HTREEITEM m_htiReleaseLimitDefault;
				HTREEITEM m_htiReleaseLimitGlobal;

				HTREEITEM m_htiReleaseLimitHigh;
				HTREEITEM m_htiReleaseLimitLow;

				HTREEITEM m_htiReleaseLimitMode;
					HTREEITEM m_htiReleaseLimitModeSimple;
					HTREEITEM m_htiReleaseLimitModeLinear;
					HTREEITEM m_htiReleaseLimitModeExplonential;
					HTREEITEM m_htiReleaseLimitModeDefault;
					HTREEITEM m_htiReleaseLimitModeGlobal;

			HTREEITEM m_htiReleaseLimitLink;
				HTREEITEM m_htiReleaseLimitLinkAnd;
				HTREEITEM m_htiReleaseLimitLinkOr;
				HTREEITEM m_htiReleaseLimitLinkDefault;
				HTREEITEM m_htiReleaseLimitLinkGlobal;
				
			HTREEITEM m_htiReleaseLimitComplete;
				HTREEITEM m_htiReleaseLimitCompleteDisable;
				HTREEITEM m_htiReleaseLimitCompleteSingle;
				HTREEITEM m_htiReleaseLimitCompleteBooth;
				HTREEITEM m_htiReleaseLimitCompleteDefault;
				HTREEITEM m_htiReleaseLimitCompleteGlobal;

				HTREEITEM m_htiReleaseLimitCompleteHigh;
				HTREEITEM m_htiReleaseLimitCompleteLow;

				HTREEITEM m_htiReleaseLimitCompleteMode;
					HTREEITEM m_htiReleaseLimitCompleteModeSimple;
					HTREEITEM m_htiReleaseLimitCompleteModeLinear;
					HTREEITEM m_htiReleaseLimitCompleteModeExplonential;
					HTREEITEM m_htiReleaseLimitCompleteModeDefault;
					HTREEITEM m_htiReleaseLimitCompleteModeGlobal;
	// limit
		HTREEITEM m_htiLimitLink;
			HTREEITEM m_htiLimitLinkAnd;
			HTREEITEM m_htiLimitLinkOr;
			HTREEITEM m_htiLimitLinkDefault;
			HTREEITEM m_htiLimitLinkGlobal;

	// source limit
		HTREEITEM m_htiReleaseSourceLimit;
			HTREEITEM m_htiSourceLimit;
				HTREEITEM m_htiSourceLimitDisable;
				HTREEITEM m_htiSourceLimitSingle;
				HTREEITEM m_htiSourceLimitBooth;
				HTREEITEM m_htiSourceLimitDefault;
				HTREEITEM m_htiSourceLimitGlobal;

				HTREEITEM m_htiSourceLimitHigh;
				HTREEITEM m_htiSourceLimitLow;

				HTREEITEM m_htiSourceLimitMode;
					HTREEITEM m_htiSourceLimitModeSimple;
					HTREEITEM m_htiSourceLimitModeLinear;
					HTREEITEM m_htiSourceLimitModeExplonential;
					HTREEITEM m_htiSourceLimitModeDefault;
					HTREEITEM m_htiSourceLimitModeGlobal;

			HTREEITEM m_htiSourceLimitLink;
				HTREEITEM m_htiSourceLimitLinkAnd;
				HTREEITEM m_htiSourceLimitLinkOr;
				HTREEITEM m_htiSourceLimitLinkDefault;
				HTREEITEM m_htiSourceLimitLinkGlobal;
				
			HTREEITEM m_htiSourceLimitComplete;
				HTREEITEM m_htiSourceLimitCompleteDisable;
				HTREEITEM m_htiSourceLimitCompleteSingle;
				HTREEITEM m_htiSourceLimitCompleteBooth;
				HTREEITEM m_htiSourceLimitCompleteDefault;
				HTREEITEM m_htiSourceLimitCompleteGlobal;

				HTREEITEM m_htiSourceLimitCompleteHigh;
				HTREEITEM m_htiSourceLimitCompleteLow;

				HTREEITEM m_htiSourceLimitCompleteMode;
					HTREEITEM m_htiSourceLimitCompleteModeSimple;
					HTREEITEM m_htiSourceLimitCompleteModeLinear;
					HTREEITEM m_htiSourceLimitCompleteModeExplonential;
					HTREEITEM m_htiSourceLimitCompleteModeDefault;
					HTREEITEM m_htiSourceLimitCompleteModeGlobal;
	// NEO: SRS END

	// NEO: NPT - [NeoPartTraffic]
	UINT	m_uUsePartTraffic;
	int		m_iPartTrafficCollors;
	// NEO: NPT END
	bool	m_bUseClassicShareStatusBar; // NEO: MOD - [ClassicShareStatusBar]
	bool	m_bUseShowSharePermissions; // NEO: SSP - [ShowSharePermissions]
	
	// NEO: SRS - [SmartReleaseSharing]
	int		m_iReleaseChunks;
	UINT	m_uReleaseSlotLimit;
	int		m_iReleaseSlotLimit;
	// NEO: SRS END

	// NEO: NMFS - [NiceMultiFriendSlots]
	UINT	m_uFriendSlotLimit;
	int		m_iFriendSlotLimit;
	// NEO: NMFS END

#ifdef BW_MOD // NEO: BM - [BandwidthModeration]
	UINT    m_uSeparateFriendBandwidth;
	float	m_fFriendSlotSpeed;
	float	m_fFriendBandwidthPercentage;

	UINT	m_uSeparateReleaseBandwidth;
	float	m_fReleaseSlotSpeed;
	float	m_fReleaseBandwidthPercentage;
#endif // BW_MOD // NEO: BM END

	bool	m_bSaveUploadQueueWaitTime; // NEO: SQ - [SaveUploadQueue]
	bool	m_bUseMultiQueue; // NEO: MQ - [MultiQueue]
	UINT	m_uUseRandomQueue; // NEO: RQ - [RandomQueue]

	bool	m_bNeoScoreSystem; // NEO: NFS - [NeoScoreSystem]
	// NEO: OCS - [OtherCreditSystems]
	int		m_iCreditSystem;
	int		m_iOtherCreditSystem;
	// NEO: OCS END

	// NEO: TQ - [TweakUploadQueue]
	bool	m_bUseInfiniteQueue;

	UINT	m_uQueueOverFlowDef;
	int		m_iQueueOverFlowDef;
	UINT	m_uQueueOverFlowEx;
	int		m_iQueueOverFlowEx;
	UINT	m_uQueueOverFlowRelease;
	int		m_iQueueOverFlowRelease;
	UINT	m_uQueueOverFlowCF;
	int		m_iQueueOverFlowCF;
	// NEO: TQ END

	// NEO: PRSF - [PushSmallRareFiles]
	bool	m_bPushSmallFiles;
	int		m_iPushSmallFilesSize;
	bool	m_bPushRareFiles;
	int		m_iPushRareFilesValue;
	bool	m_bPushRatioFiles;
	int		m_iPushRatioFilesValue;
	// NEO: PRSF END

	// NEO: IPS - [InteligentPartSharing]
	SrbtC	m_UseInteligentPartSharing;				// Flag
	SintD	m_InteligentPartSharingTimer;			// Value

	SintD	m_MaxProzentToHide;						// Value

	// OverAvalibly
	SrbtC	m_HideOverAvaliblyParts;				// Flag
	SrbtC	m_HideOverAvaliblyMode;					// Flag
	SintD	m_HideOverAvaliblyValue;

	SrbtC	m_BlockHighOverAvaliblyParts;			// Flag
	SintD	m_BlockHighOverAvaliblyFactor;			// Value

	// OverShared
	SrbtC	m_HideOverSharedParts;					// Flag
	SrbtC	m_HideOverSharedMode;					// Flag
	SintD	m_HideOverSharedValue;					// Value
	SrbtC	m_HideOverSharedCalc;					// Flag

	SrbtC	m_BlockHighOverSharedParts;				// Flag
	SintD	m_BlockHighOverSharedFactor;			// Value

	// DontHideUnderAvalibly
	SrbtC	m_DontHideUnderAvaliblyParts;			// Flag
	SrbtC	m_DontHideUnderAvaliblyMode;			// Flag
	SintD	m_DontHideUnderAvaliblyValue;			// Value

	// Other
	SrbtC	m_ShowAlwaysSomeParts;					// Flag
	SintD	m_ShowAlwaysSomePartsValue;				// Value

	SrbtC	m_ShowAlwaysIncompleteParts;			// Flag
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	SrbtC	m_ReleaseMode;							// Flag
	SintD	m_ReleaseLevel;							// Value
	SintD	m_ReleaseTimer;							// Value

	// release limit
	SrbtC	m_ReleaseLimit;							// Flag
	SrbtC	m_ReleaseLimitMode;						// Flag
	SintD	m_ReleaseLimitHigh;						// Value
	SintD	m_ReleaseLimitLow;						// Value

	SrbtC	m_ReleaseLimitLink;						// Flag

	SrbtC	m_ReleaseLimitComplete;					// Flag
	SrbtC	m_ReleaseLimitCompleteMode;				// Flag
	SintD	m_ReleaseLimitCompleteHigh;				// Value
	SintD	m_ReleaseLimitCompleteLow;				// Value

	// limit
	SrbtC	m_LimitLink;							// Flag

	// source limit
	SrbtC	m_SourceLimit;							// Flag
	SrbtC	m_SourceLimitMode;						// Flag
	SintD	m_SourceLimitHigh;						// Value
	SintD	m_SourceLimitLow;						// Value

	SrbtC	m_SourceLimitLink;						// Flag

	SrbtC	m_SourceLimitComplete;					// Flag
	SrbtC	m_SourceLimitCompleteMode;				// Flag
	SintD	m_SourceLimitCompleteHigh;				// Value
	SintD	m_SourceLimitCompleteLow;				// Value
	// NEO: SRS END

	void ClearAllMembers();

	//void Localize();
	void LoadSettings();
	void RefreshData(); // NEO: FCFG - [FileConfiguration]
	

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnSetActive(); // NEO: FCFG - [FileConfiguration]

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT nIDEvent); // NEO: FCFG - [FileConfiguration]
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM); // NEO: FCFG - [FileConfiguration]
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DrawTreeItemHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

private:
	void SetLimits();
	void SetAuxSet(bool bGlobal);

	// NEO: IPS - [InteligentPartSharing]
	void SetTreeRadioForIPS(HTREEITEM &htiDisable, HTREEITEM &htiEnableKF, HTREEITEM &htiEnable, HTREEITEM &htiEnablePF, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	void SetTreeRadioForIPSMode(HTREEITEM &htiGroup, int iImgMode, HTREEITEM &htiMultiplicativ, HTREEITEM &htiAdditiv, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	void SetTreeRadioForSRS(HTREEITEM &htiDisable, HTREEITEM &htiSingle, HTREEITEM &htiBooth, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	void SetTreeRadioForSRSMode(HTREEITEM &htiGroup, int iImgMode, HTREEITEM &htiSimple, HTREEITEM &htiLinear, HTREEITEM &htiExplonential, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	void SetTreeRadioForSRSLink(HTREEITEM &htiGroup, int iImgLink, HTREEITEM &htiAnd, HTREEITEM &htiOr, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	// NEO: SRS END
};
