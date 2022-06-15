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

class CPPgSourceStorage : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSourceStorage)

public:
	CPPgSourceStorage();
	virtual ~CPPgSourceStorage();

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
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	HTREEITEM m_htiSourceStorage;
		HTREEITEM m_htiAutoSaveSources;
			HTREEITEM m_htiAutoSaveSourcesDisable;
			HTREEITEM m_htiAutoSaveSourcesEnable1;
			HTREEITEM m_htiAutoSaveSourcesEnable2;
			HTREEITEM m_htiAutoSaveSourcesDefault;
			HTREEITEM m_htiAutoSaveSourcesGlobal;

			HTREEITEM m_htiAutoSaveSourcesIntervals;
			HTREEITEM m_htiSourceStorageLimit;

			HTREEITEM m_htiStoreAlsoA4AFSources;
				HTREEITEM m_htiStoreAlsoA4AFSourcesDisable;
				HTREEITEM m_htiStoreAlsoA4AFSourcesEnable;
				HTREEITEM m_htiStoreAlsoA4AFSourcesDefault;
				HTREEITEM m_htiStoreAlsoA4AFSourcesGlobal;
				
		HTREEITEM m_htiAutoLoadSources;
			HTREEITEM m_htiAutoLoadSourcesDisable;
			HTREEITEM m_htiAutoLoadSourcesEnable1;
			HTREEITEM m_htiAutoLoadSourcesEnable2;
			HTREEITEM m_htiAutoLoadSourcesDefault;
			HTREEITEM m_htiAutoLoadSourcesGlobal;

			HTREEITEM m_htiLoadedSourceCleanUpTime;
			HTREEITEM m_htiSourceStorageReaskLimit;

			HTREEITEM m_htiTotalSourceRestore;
				HTREEITEM m_htiTotalSourceRestoreDisable;
				HTREEITEM m_htiTotalSourceRestoreEnable1;
				HTREEITEM m_htiTotalSourceRestoreEnable2;
				HTREEITEM m_htiTotalSourceRestoreDefault;
				HTREEITEM m_htiTotalSourceRestoreGlobal;

		HTREEITEM m_htiReaskManagement;
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			HTREEITEM m_htiReaskPropability;

			HTREEITEM m_htiUnpredictedPropability;
				HTREEITEM m_htiUnpredictedPropabilityDisable;
				HTREEITEM m_htiUnpredictedPropabilityEnable;
				HTREEITEM m_htiUnpredictedPropabilityDefault;
				HTREEITEM m_htiUnpredictedPropabilityGlobal;
				HTREEITEM m_htiUnpredictedReaskPropability;
 #endif // NEO_SA // NEO: NSA END

			HTREEITEM m_htiAutoReaskStoredSourcesDelay;

			HTREEITEM m_htiGroupStoredSourceReask;
				HTREEITEM m_htiGroupStoredSourceReaskDisable;
				HTREEITEM m_htiGroupStoredSourceReaskEnable;
				HTREEITEM m_htiGroupStoredSourceReaskDefault;
				HTREEITEM m_htiGroupStoredSourceReaskGlobal;

				HTREEITEM m_htiStoredSourceGroupIntervals;
				HTREEITEM m_htiStoredSourceGroupSize;
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	HTREEITEM m_htiClientDatabase;
		HTREEITEM m_htiEnableSourceList;
		// cleanup
			HTREEITEM m_htiSourceListExpirationTime;
				HTREEITEM m_htiSourceListRunTimeCleanUp;
	// NEO: SFL - [SourceFileList]
	// seen files
		HTREEITEM m_htiSaveSourceFileList;
			HTREEITEM m_htiFileListExpirationTime;
	// NEO: SFL END
	// security
		HTREEITEM m_htiIpSecurity;
			HTREEITEM m_htiUseIPZoneCheck;
			HTREEITEM m_htiSourceHashMonitor; // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
		HTREEITEM m_htiIpTables;
			HTREEITEM m_htiTableAmountToStore;
			HTREEITEM m_htiIgnoreUndefinedIntervall;
			HTREEITEM m_htiIgnoreUnreachableInterval;
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	HTREEITEM m_htiSourceAnalyzer;
		HTREEITEM m_htiEnableSourceAnalizer;
			HTREEITEM m_htiAnalyzeIntervals;
			HTREEITEM m_htiTableAmountToAnalyze;
	// gap handling
		HTREEITEM m_htiHandleTableGaps;
			HTREEITEM m_htiPriorityGapRatio;
			HTREEITEM m_htiMaxGapSize;
			HTREEITEM m_htiMaxGapTime;
	// analyze obtions
		HTREEITEM m_htiAdvanced;
			HTREEITEM m_htiMaxMidleDiscrepanceHigh;
			HTREEITEM m_htiMaxMidleDiscrepanceLow;
			HTREEITEM m_htiDualLinkedTableGravity;
	// calculation obtions
		HTREEITEM m_htiCalculation;
			HTREEITEM m_htiEnhancedFactor;
			HTREEITEM m_htiFreshSourceTreshold;
			HTREEITEM m_htiTempralIPBorderLine;
	// additional obtions
			HTREEITEM m_htiAdvancedCalculation;
				HTREEITEM m_htiLastSeenDurationThreshold;
				HTREEITEM m_htiLinkTimeThreshold;
				HTREEITEM m_htiMaxReliableTime;
#endif // NEO_SA // NEO: NSA END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	SrbtC	m_AutoSaveSources;
	SintD	m_AutoSaveSourcesIntervals;
	SintD	m_SourceStorageLimit;

	SrbtC	m_StoreAlsoA4AFSources;

	SrbtC	m_AutoLoadSources;
	SintD	m_LoadedSourceCleanUpTime;
	SintD	m_SourceStorageReaskLimit;

	SrbtC	m_TotalSourceRestore;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	SintD	m_ReaskPropability;
	SrbtC	m_UnpredictedPropability;
	SintD	m_UnpredictedReaskPropability;
 #endif // NEO_SA // NEO: NSA END

	SintD	m_AutoReaskStoredSourcesDelay;

	SrbtC	m_GroupStoredSourceReask;
	SintD	m_StoredSourceGroupIntervals;
	SintD	m_StoredSourceGroupSize;
#endif // NEO_SS // NEO: NSS END
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	bool	m_bEnableSourceList;
	SintD	m_iTableAmountToStore;
	// cleanup
	UINT	m_uSourceListRunTimeCleanUp;
	SintD	m_iSourceListExpirationTime;
	// NEO: SFL - [SourceFileList]
	// seen files
	bool	m_bSaveSourceFileList;
	SintD	m_iFileListExpirationTime;
	// NEO: SFL END
	// security
	bool	m_bUseIPZoneCheck;
	bool	m_bSourceHashMonitor; // NEO: SHM - [SourceHashMonitor]
	// aditional obtions
	SintD	m_iIgnoreUndefinedIntervall;
	SintD	m_iIgnoreUnreachableInterval;
#endif // NEO_CD // NEO: NCD END
#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	bool	m_bEnableSourceAnalizer;
	SintD	m_iAnalyzeIntervals;
	SintD	m_iTableAmountToAnalyze;
	// gap handling
	bool	m_bHandleTableGaps;
	SintD	m_fPriorityGapRatio;
	SintD	m_iMaxGapSize;
	SintD	m_iMaxGapTime;
	// analyze obtions
	SintD	m_fMaxMidleDiscrepanceHigh;
	SintD	m_fMaxMidleDiscrepanceLow;
	bool	m_bDualLinkedTableGravity;
	SintD	m_iDualLinkedTableGravity;
	// calculation obtions
	SintD	m_fEnhancedFactor;
	SintD	m_iFreshSourceTreshold;
	SintD	m_iTempralIPBorderLine;
	// additional obtions
	SintD	m_fLastSeenDurationThreshold;
	bool	m_bLinkTimePropability;
	SintD	m_iLinkTimeThreshold;
	bool	m_bScaleReliableTime;
	SintD	m_iMaxReliableTime;
#endif // NEO_SA // NEO: NSA END

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
	void SetAut();
	void SetAutVal(SintD &Val, int Ref, int Fac, int Set = 0);

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	void SetTreeRadioForNSS(HTREEITEM &htiDisable, HTREEITEM &htiEnable, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	void SetTreeRadioForNSS(HTREEITEM &htiDisable, HTREEITEM &htiEnable1, HTREEITEM &htiEnable2, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value, CString lbl1, CString Info1, CString lbl2, CString Info2);
#endif // NEO_SS // NEO: NSS END
};
