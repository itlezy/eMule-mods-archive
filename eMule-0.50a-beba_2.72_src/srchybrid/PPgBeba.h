// Tux: others: beba prefs [start]
//this file is part of eMule beba
//Copyright (C)2005-2010 Tuxman ( der_tuxman@arcor.de / http://tuxproject.de)
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include "preferences.h"
#include "afxwin.h"
#include "TreeOptionsCtrlEx.h"
#include "OtherFunctions.h"

class CPPgBeba : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgBeba)

public:
	CPPgBeba();
	virtual ~CPPgBeba();

	void SetPrefs(CPreferences* in_prefs) {	app_prefs = in_prefs; }

	enum { IDD = IDD_PPG_BEBA };
protected:
	CPreferences *app_prefs;

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	// General groups
	HTREEITEM m_htiUploadTweaks;
	HTREEITEM m_htiQueue;
	HTREEITEM m_htiDownloadTweaks;
	HTREEITEM m_htiReaskTweaks;
	HTREEITEM m_htiMiscTweaks;

	// Tux: Feature: Emulate others [start]
	HTREEITEM m_htiEmuSettings;
	HTREEITEM m_htiEmuMLDonkey;
	HTREEITEM m_htiEmueDonkey;
	HTREEITEM m_htiEmueDonkeyHybrid;
	HTREEITEM m_htiEmuShareaza;
	HTREEITEM m_htiEmuLphant;
	bool m_bEmuMLDonkey;
	bool m_bEmueDonkey;
	bool m_bEmueDonkeyHybrid;
	bool m_bEmuShareaza;
	bool m_bEmuLphant;
	// Tux: Feature: Emulate others [end]

	HTREEITEM m_htiCreditSystem;
	// Tux: Feature: Analyzer CS [start]
	HTREEITEM m_htiCSOfficial;
	HTREEITEM m_htiCSAnalyzer;
	int	 m_bCreditSystem;
	// Tux: Feature: Analyzer CS [end]
	// Tux: Feature: Payback First [start]
	HTREEITEM m_htiPaybackFirstRadio;
	HTREEITEM m_htiPaybackDisabled;
	HTREEITEM m_htiPaybackConst;
	HTREEITEM m_htiPaybackVar;
	HTREEITEM m_htiPaybackAuto;
	int  m_iPaybackFirst;
	// Tux: Feature: Payback First [end]

	// Tux: Feature: Auto Hard Limit [start]
	HTREEITEM m_htiAutoHL;
	HTREEITEM m_htiUseAutoHLRadio;
	HTREEITEM m_htiUseAutoHLPerFile;
	HTREEITEM m_htiUseAutoHLOn;
	HTREEITEM m_htiUseAutoHLOff;
	HTREEITEM m_htiMaxSources;
	HTREEITEM m_htiMaxPerFileSources;
	HTREEITEM m_htiMinPerFileSources;
	HTREEITEM m_htiAHLUpdate;
	int m_iUseAutoHL;
	int m_iMaxSources;
	int m_iMaxPerFileSources;
	int m_iMinPerFileSources;
	int m_iAHLUpdate;
	// Tux: Feature: Auto Hard Limit [end]

	// Tux: Feature: Probabilistic Queue [start]
	HTREEITEM m_htiRandQueue;
	bool m_bRandQueue;
	// Tux: Feature: Probabilistic Queue [end]

	// Tux: Feature: Infinite Queue [start]
	HTREEITEM m_htiInfiniteQueue;
	bool m_bInfiniteQueue;
	// Tux: Feature: Infinite Queue [end]
	
	// Tux: Feature: Release Bonus [start]
	HTREEITEM m_htiReleaseBonus;
	HTREEITEM m_htiReleaseBonusEnabled;
	HTREEITEM m_htiReleaseBonusFactor;
	bool m_bReleaseBonusEnabled;
	int m_iReleaseBonusFactor;
	// Tux: Feature: Release Bonus [end]

	// Tux: Feature: SLS [start]
	HTREEITEM m_htiSLS;
	HTREEITEM m_htiUseSLS;
	HTREEITEM m_htiSLSlimit;
	HTREEITEM m_htiSLSnumber;
	bool m_bUseSaveLoadSources;
	int m_iSLSlimit;
	int m_iSLSnumber;
	// Tux: Feature: SLS [end]

	// Tux: Feature: Reask sources after IP change [start]
	HTREEITEM m_htiReaskSrcAfterIPChange;
	HTREEITEM m_htiReaskFileSrc;
	bool m_bReaskSrcAfterIPChange;
	int m_iReaskFileSrc;
	// Tux: Feature: Reask sources after IP change [end]
	
	// Tux: Feature: Relative Priority [start]
	HTREEITEM m_htiRelativePriority;
	HTREEITEM m_htiRelativePrioAutoSet;
	HTREEITEM m_htiRelativePrioAutoTime;
	bool m_bRelativePrioAutoSet;
	int m_iRelativePrioAutoTime;
	// Tux: Feature: Relative Priority [end]
	
	// Tux: Feature: Filename disparity check [start]
	HTREEITEM m_htiFDC;
	HTREEITEM m_htiNamecheckenabled;
	bool m_bNamecheckenabled;
	HTREEITEM m_htiFDCMode;
	HTREEITEM m_htiNamecheckIcon;
	HTREEITEM m_htiNamecheckColored;
	int m_iNamecheckmode;
	int fdcspos;
	// Tux: Feature: Filename disparity check [end]
	
	// Tux: Feature: IntelliFlush [start]
	HTREEITEM m_htiSystemGroup;
	HTREEITEM m_htiFileBufferGroup;
	HTREEITEM m_htiFileBufferSize;
	HTREEITEM m_htiFileBufferTime;
	HTREEITEM m_htiIntelliFlushCheck;
	int m_iFileBufferSize;
	int m_iFileBufferTime;
	bool m_bIntelliFlush;
	// Tux: Feature: IntelliFlush [end]

	// Tux: Feature: Automatic shared files updater [start]
	HTREEITEM m_htiDirWatcher;
	bool m_bDirWatcher;
	// Tux: Feature: Automatic shared files updater [end]

	// Tux: Feature: Snarl notifications [start]
	HTREEITEM m_htiSnarlDisabled;
	bool m_bSnarlDisabled;
	// Tux: Feature: Snarl notifications [end]
	
	bool bReopenPrefs;	// Tux: Improvement: repaint prefs wnd if needed :-)

	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);	// Tux: Feature: Slot Control
	afx_msg void OnDestroy();
	afx_msg void OnSlotspeedChange();	// Tux: Feature: Slot Control
	afx_msg void OnFDCChange();		// Tux: Feature: Filename disparity check

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void LoadSettings(void);
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	afx_msg void OnSettingsChange() { SetModified();}
	afx_msg void OnNMCustomdrawSliderfdc(NMHDR *pNMHDR, LRESULT *pResult);	// Tux: Feature: Filename disparity check

	void Localize(void);
private:
	void ShowUpSlotValues();	// Tux: Feature: Slot Control
};
// Tux: others: beba prefs [end]