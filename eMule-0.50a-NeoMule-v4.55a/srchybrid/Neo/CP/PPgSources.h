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

class CPPgSources : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSources)

public:
	CPPgSources();
	virtual ~CPPgSources();

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

	// NEO: SRT - [SourceRequestTweaks]
	HTREEITEM m_htiSourceRequest;
		// General
		HTREEITEM m_htiSourceLimit;

		// Management
		HTREEITEM m_htiSourceManagement;
			HTREEITEM m_htiSwapLimit;
			// NEO: NXC - [NewExtendedCategories]
			HTREEITEM m_htiAdvancedA4AFMode;
				HTREEITEM m_htiAdvancedA4AFModeDisabled;
				HTREEITEM m_htiAdvancedA4AFModeBalance;
				HTREEITEM m_htiAdvancedA4AFModeStack;
				HTREEITEM m_htiSmartA4AFSwapping;
			HTREEITEM m_htiA4AFFlags;
				HTREEITEM m_htiA4AFFlagsNone;
				HTREEITEM m_htiA4AFFlagsOn;
				HTREEITEM m_htiA4AFFlagsOff;
			// NEO: NXC END
			// NEO: XSC - [ExtremeSourceCache]
			HTREEITEM m_htiSourceCache;
				HTREEITEM m_htiSourceCacheDisable;
				HTREEITEM m_htiSourceCacheEnable;
				HTREEITEM m_htiSourceCacheDefault;
				HTREEITEM m_htiSourceCacheGlobal;
				HTREEITEM m_htiSourceCacheLimit;
				HTREEITEM m_htiSourceCacheTime;
			// NEO: XSC END
			// NEO: ASL - [AutoSoftLock]
			HTREEITEM m_htiAutoSoftLock;
				HTREEITEM m_htiAutoSoftLockDisable;
				HTREEITEM m_htiAutoSoftLockEnable;
				HTREEITEM m_htiAutoSoftLockDefault;
				HTREEITEM m_htiAutoSoftLockGlobal;
				HTREEITEM m_htiAutoSoftLockLimit;
			// NEO: ASL END
			// NEO: AHL - [AutoHardLimit]
			HTREEITEM m_htiAutoHardLimit;
				HTREEITEM m_htiAutoHardLimitDisable;
				HTREEITEM m_htiAutoHardLimitEnable;
				HTREEITEM m_htiAutoHardLimitDefault;
				HTREEITEM m_htiAutoHardLimitGlobal;
				HTREEITEM m_htiAutoHardLimitTime;
			// NEO: AHL END
			// NEO: CSL - [CategorySourceLimit]
			HTREEITEM m_htiCategorySourceLimit;
				HTREEITEM m_htiCategorySourceLimitDisable;
				HTREEITEM m_htiCategorySourceLimitEnable;
				HTREEITEM m_htiCategorySourceLimitDefault;
				HTREEITEM m_htiCategorySourceLimitGlobal;
				HTREEITEM m_htiCategorySourceLimitLimit;
				HTREEITEM m_htiCategorySourceLimitTime;
			// NEO: CSL END
			// NEO: GSL - [GlobalSourceLimit]
			HTREEITEM m_htiGlobalSourceLimit;
				HTREEITEM m_htiGlobalSourceLimitDisable;
				HTREEITEM m_htiGlobalSourceLimitEnable;
				HTREEITEM m_htiGlobalSourceLimitDefault;
				HTREEITEM m_htiGlobalSourceLimitGlobal;
				HTREEITEM m_htiGlobalSourceLimitLimit;
				HTREEITEM m_htiGlobalSourceLimitTime;
			// NEO: GSL END
			HTREEITEM m_htiMinSourcePerFile;

		//XS
		HTREEITEM m_htiXs;
			HTREEITEM m_htiXsDisable;
			HTREEITEM m_htiXsEnable;
			HTREEITEM m_htiXsDefault;
			HTREEITEM m_htiXsGlobal;
				
			HTREEITEM m_htiXsLimit;
			HTREEITEM m_htiXsIntervals;
			HTREEITEM m_htiXsClientIntervals;
			HTREEITEM m_htiXsCleintDelay;
			HTREEITEM m_htiXsRareLimit;

	// SVR
		HTREEITEM m_htiSvr;
			HTREEITEM m_htiSvrDisable;
			HTREEITEM m_htiSvrEnable;
			HTREEITEM m_htiSvrDefault;
			HTREEITEM m_htiSvrGlobal;

			HTREEITEM m_htiSvrLimit;
			HTREEITEM m_htiSvrIntervals;

	//KAD
		HTREEITEM m_htiKad;
			HTREEITEM m_htiKadDisable;
			HTREEITEM m_htiKadEnable;
			HTREEITEM m_htiKadDefault;
			HTREEITEM m_htiKadGlobal;

			HTREEITEM m_htiKadLimit;
			HTREEITEM m_htiKadIntervals;
			HTREEITEM m_htiKadMaxFiles;
			HTREEITEM m_htiKadRepeatDelay;

	//UDP
		HTREEITEM m_htiUdp;
			HTREEITEM m_htiUdpDisable;
			HTREEITEM m_htiUdpEnable;
			HTREEITEM m_htiUdpDefault;
			HTREEITEM m_htiUdpGlobal;

			HTREEITEM m_htiUdpLimit;
			HTREEITEM m_htiUdpIntervals;
			HTREEITEM m_htiUdpGlobalIntervals;
			HTREEITEM m_htiUdpFilesPerServer;
	// NEO: SRT END
		HTREEITEM m_htiTCPConnectionRetry; // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	HTREEITEM m_htiDownloadReask;
		HTREEITEM m_htiSpreadReask;
		HTREEITEM m_htiSourceReaskTime;
		HTREEITEM m_htiFullQSourceReaskTime;
		HTREEITEM m_htiNNPSourceReaskTime;
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	HTREEITEM m_htiSourcesDrop;
		HTREEITEM m_htiDropTime;

	//Bad
		HTREEITEM m_htiBad;
			HTREEITEM m_htiBadSourceDontDrop;
			HTREEITEM m_htiBadSourceDrop;
			HTREEITEM m_htiBadSourceTrySwap;
			HTREEITEM m_htiBadSourceDefault;
			HTREEITEM m_htiBadSourceGlobal;

			HTREEITEM m_htiBadSourceLimit;
				HTREEITEM m_htiBadSourceLimitTotal;
				HTREEITEM m_htiBadSourceLimitRelativ;
				HTREEITEM m_htiBadSourceLimitSpecyfic;
				HTREEITEM m_htiBadSourceLimitDefault;
				HTREEITEM m_htiBadSourceLimitGlobal;
			HTREEITEM m_htiBadSourceDropTime;
				HTREEITEM m_htiBadSourceDropProgressiv;
				HTREEITEM m_htiBadSourceDropDistributiv;
				HTREEITEM m_htiBadSourceDropCummulativ;
				HTREEITEM m_htiBadSourceDropDefault;
				HTREEITEM m_htiBadSourceDropGlobal;

	//NNP
		HTREEITEM m_htiNNP;
			HTREEITEM m_htiNNPSourceDontDrop;
			HTREEITEM m_htiNNPSourceDrop;
			HTREEITEM m_htiNNPSourceTrySwap;
			HTREEITEM m_htiNNPSourceDefault;
			HTREEITEM m_htiNNPSourceGlobal;

			HTREEITEM m_htiNNPSourceLimit;
				HTREEITEM m_htiNNPSourceLimitTotal;
				HTREEITEM m_htiNNPSourceLimitRelativ;
				HTREEITEM m_htiNNPSourceLimitSpecyfic;
				HTREEITEM m_htiNNPSourceLimitDefault;
				HTREEITEM m_htiNNPSourceLimitGlobal;
			HTREEITEM m_htiNNPSourceDropTime;
				HTREEITEM m_htiNNPSourceDropProgressiv;
				HTREEITEM m_htiNNPSourceDropDistributiv;
				HTREEITEM m_htiNNPSourceDropCummulativ;
				HTREEITEM m_htiNNPSourceDropDefault;
				HTREEITEM m_htiNNPSourceDropGlobal;
	//FullQ
		HTREEITEM m_htiFullQ;
			HTREEITEM m_htiFullQSourceDontDrop;
			HTREEITEM m_htiFullQSourceDrop;
			HTREEITEM m_htiFullQSourceTrySwap;
			HTREEITEM m_htiFullQSourceDefault;
			HTREEITEM m_htiFullQSourceGlobal;

			HTREEITEM m_htiFullQSourceLimit;
				HTREEITEM m_htiFullQSourceLimitTotal;
				HTREEITEM m_htiFullQSourceLimitRelativ;
				HTREEITEM m_htiFullQSourceLimitSpecyfic;
				HTREEITEM m_htiFullQSourceLimitDefault;
				HTREEITEM m_htiFullQSourceLimitGlobal;
			HTREEITEM m_htiFullQSourceDropTime;
				HTREEITEM m_htiFullQSourceDropProgressiv;
				HTREEITEM m_htiFullQSourceDropDistributiv;
				HTREEITEM m_htiFullQSourceDropCummulativ;
				HTREEITEM m_htiFullQSourceDropDefault;
				HTREEITEM m_htiFullQSourceDropGlobal;

	//HighQ
		HTREEITEM m_htiHighQ;
			HTREEITEM m_htiHighQSourceDontDrop;
			HTREEITEM m_htiHighQSourceDrop;
			HTREEITEM m_htiHighQSourceTrySwap;
			HTREEITEM m_htiHighQSourceDefault;
			HTREEITEM m_htiHighQSourceGlobal;

			HTREEITEM m_htiHighQSourceLimit;
				HTREEITEM m_htiHighQSourceLimitTotal;
				HTREEITEM m_htiHighQSourceLimitRelativ;
				HTREEITEM m_htiHighQSourceLimitSpecyfic;
				HTREEITEM m_htiHighQSourceLimitDefault;
				HTREEITEM m_htiHighQSourceLimitGlobal;
			HTREEITEM m_htiHighQSourceDropTime;
				HTREEITEM m_htiHighQSourceDropProgressiv;
				HTREEITEM m_htiHighQSourceDropDistributiv;
				HTREEITEM m_htiHighQSourceDropCummulativ;
				HTREEITEM m_htiHighQSourceDropDefault;
				HTREEITEM m_htiHighQSourceDropGlobal;
			HTREEITEM m_htiHighQSourceMaxRank;
				HTREEITEM m_htiHighQSourceRankNormal;
				HTREEITEM m_htiHighQSourceRankAverage;
				HTREEITEM m_htiHighQSourceRankDefault;
				HTREEITEM m_htiHighQSourceRankGlobal;

		HTREEITEM m_htiDeadTime;
			HTREEITEM m_htiDeadTimeFWMulti;
		HTREEITEM m_htiGlobalDeadTime;
			HTREEITEM m_htiGlobalDeadTimeFWMulti;
	// NEO: SDT END


	// NEO: SRT - [SourceRequestTweaks]
	// General
	SintD	m_SourceLimit;				// Value

	// Management
	SintD	m_SwapLimit;				// Value

	//XS
	SrbtC	m_XsEnable;					// Flag
	SintD	m_XsLimit;					// Value
	SintD	m_XsIntervals;				// Value
	SintD	m_XsClientIntervals;		// Value
	SintD	m_XsCleintDelay;			// Value
	SintD	m_XsRareLimit;				// Value

	// SVR
	SrbtC	m_SvrEnable;				// Flag
	SintD	m_SvrLimit;					// Value
	SintD	m_SvrIntervals;				// Value

	//KAD
	SrbtC	m_KadEnable;				// Flag
	SintD	m_KadLimit;					// Value
	SintD	m_KadIntervals;				// Value
	SintD	m_KadMaxFiles;				// Value
	SintD	m_KadRepeatDelay;			// Value

	//UDP
	SrbtC	m_UdpEnable;				// Flag
	SintD	m_UdpLimit;					// Value
	SintD	m_UdpIntervals;				// Value
	SintD	m_UdpGlobalIntervals;		// Value (Global)
	SintD	m_UdpFilesPerServer;		// Value (Global)
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	SrbtC	m_UseSourceCache;			// Flag
	SintD	m_SourceCacheLimit;			// Value
	SintD	m_SourceCacheTime;			// Value
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	SrbtC	m_AutoSoftLock;				// Flag
	SintD	m_AutoSoftLockLimit;		// Value
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	SrbtC	m_AutoHardLimit;			// Flag
	SintD	m_AutoHardLimitTime;		// Value
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	SrbtC	m_CategorySourceLimit;		// Flag
	SintD	m_CategorySourceLimitLimit;	// Value (Category)
	SintD	m_CategorySourceLimitTime;	// Value (Category)
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	SrbtC	m_GlobalSourceLimit;		// Flag
	SintD	m_GlobalSourceLimitLimit;	// Value (Global)
	SintD	m_GlobalSourceLimitTime;	// Value (Global)
	// NEO: GSL END

	SintD	m_MinSourcePerFile;			// Value

	SintD	m_TCPConnectionRetry;		// Value // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	UINT	m_SpreadReaskEnable;		// Flag (Global)
	SintD	m_SpreadReaskTime;			// Value (Global)
	SintD	m_SourceReaskTime;			// Value
	SintD	m_FullQSourceReaskTime;		// Value
	SintD	m_NNPSourceReaskTime;		// Value
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	SintD	m_DropTime;					// Value

	//Bad
	SrbtC	m_BadSourceDrop;			// Flag
	SintD	m_BadSourceLimit;			// Value
	SrbtC	m_BadSourceLimitMode;		// Flag
	SintD	m_BadSourceDropTime;		// Value
	SrbtC	m_BadSourceDropMode;		// Flag

	//NNP
	SrbtC	m_NNPSourceDrop;			// Flag
	SintD	m_NNPSourceLimit;			// Value
	SrbtC	m_NNPSourceLimitMode;		// Flag
	SintD	m_NNPSourceDropTime;		// Value
	SrbtC	m_NNPSourceDropMode;		// Flag

	//FullQ
	SrbtC	m_FullQSourceDrop;			// Flag
	SintD	m_FullQSourceLimit;			// Value
	SrbtC	m_FullQSourceLimitMode;		// Flag
	SintD	m_FullQSourceDropTime;		// Value
	SrbtC	m_FullQSourceDropMode;		// Flag

	//HighQ
	SrbtC	m_HighQSourceDrop;			// Flag
	SintD	m_HighQSourceLimit;			// Value
	SrbtC	m_HighQSourceLimitMode;		// Flag
	SintD	m_HighQSourceDropTime;		// Value
	SrbtC	m_HighQSourceDropMode;		// Flag
	SintD	m_HighQSourceMaxRank;		// Value
	SrbtC	m_HighQSourceRankMode;		// Flag


	SintD	m_DeadTime;					// Value
	SintD	m_DeadTimeFWMulti;			// Value
	SintD	m_GlobalDeadTime;			// Value
	SintD	m_GlobalDeadTimeFWMulti;	// Value
	// NEO: SDT END

	// NEO: NXC - [NewExtendedCategories]
	int		m_ForceA4AF;
	int		m_iAdvancedA4AFMode;
	bool	m_bSmartA4AFSwapping;
	// NEO: NXC END

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

	// NEO: SRT - [SourceRequestTweaks]
	void SetTreeRadioForSRT(HTREEITEM &htiDisable, HTREEITEM &htiEnable, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
	void SetTreeNumEditForSRT(HTREEITEM &htiLimit, HTREEITEM &htiIntervals, HTREEITEM &htiParent);
	// NEO: SRT END

	// NEO: SDT - [SourcesDropTweaks]
	void SetTreeRadioForSDT(HTREEITEM &htiDontDrop, HTREEITEM &htiDrop, HTREEITEM &htiTrySwap, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value, bool bNoSwap = false);
	void SetTreeRadioNumEditForSDT1(HTREEITEM &htiLimit,HTREEITEM &htiLimitTotal,HTREEITEM &htiLimitRelativ,HTREEITEM &htiLimitSpecyfic,HTREEITEM &htiLimitDefault,HTREEITEM &htiLimitGlobal, HTREEITEM &htiParent, SrbtC Value);
	void SetTreeRadioNumEditForSDT2(HTREEITEM &htiTime,HTREEITEM &htiDropProgressiv,HTREEITEM &htiDropDistributiv,HTREEITEM &htiDropCummulativ,HTREEITEM &htiDropDefault,HTREEITEM &htiDropGlobal, HTREEITEM &htiParent, SrbtC Value);
	// NEO: SDT END
};
