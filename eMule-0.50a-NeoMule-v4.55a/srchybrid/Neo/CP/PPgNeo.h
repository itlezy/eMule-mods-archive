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

class CPPgNeo : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgNeo)

public:
	CPPgNeo();
	virtual ~CPPgNeo();

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	CTreeOptionsCtrl m_ctrlTreeOptions; // NEO - [TreeControl] <-- Xanatos --
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiAppPriority; // NEO: MOD - [AppPriority]
	HTREEITEM m_htiPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]

	HTREEITEM m_htiProt;
		HTREEITEM m_htiIncompletePartStatus; // NEO: ICS - [InteligentChunkSelection] 
		HTREEITEM m_htiSubChunkTransfer; // NEO: SCT - [SubChunkTransfer]
		HTREEITEM m_htiPartStatusHistory; // NEO: PSH - [PartStatusHistory]
		HTREEITEM m_htiLowID2HighIDAutoCallback; // NEO: L2HAC - [LowID2HighIDAutoCallback]
		HTREEITEM m_htiSaveComments; // NEO: XCs - [SaveComments]
		HTREEITEM m_htiKnownComments; // NEO: XCk - [KnownComments]	
	
	HTREEITEM m_htiPreferShareAll; // NEO: PSA - [PreferShareAll]
	HTREEITEM m_htiLugdunumCredits; // NEO: KLC - [KhaosLugdunumCredits]
	HTREEITEM m_htiDontRemoveStaticServers; // NEO: MOD - [DontRemoveStaticServers]

	HTREEITEM m_htiOnlyCompleteFiles; // NEO: OCF - [OnlyCompleetFiles]
	HTREEITEM m_htiRefreshShared; // NEO: MOD - [RefreshShared]

	DWORD	m_dAppPriority;// NEO: MOD - [AppPriority]
	bool	m_bPauseOnFileComplete; // NEO: POFC - [PauseOnFileComplete]

	UINT	m_uIncompletePartStatus;	// NEO: ICS - [InteligentChunkSelection] 
	UINT	m_uSubChunkTransfer; // NEO: SCT - [SubChunkTransfer]
	bool	m_bPartStatusHistory; // NEO: PSH - [PartStatusHistory]
	bool	m_bLowID2HighIDAutoCallback; // NEO: L2HAC - [LowID2HighIDAutoCallback]
	bool	m_bSaveComments; // NEO: XCs - [SaveComments]
	bool	m_bKnownComments; // NEO: XCk - [KnownComments]	
	
	bool	m_bPreferShareAll; // NEO: PSA - [PreferShareAll]
	UINT	m_uLugdunumCredits; // NEO: KLC - [KhaosLugdunumCredits]
	bool	m_bDontRemoveStaticServers; // NEO: MOD - [DontRemoveStaticServers]

	// NEO: OCF - [OnlyCompleetFiles]
	bool	m_bOnlyCompleteFiles;
	int		m_iToOldComplete;
	// NEO: OCF END

	// NEO: MOD - [RefreshShared]
	bool	m_bRefreshShared;
	int		m_iRefreshSharedIntervals;
	// NEO: MOD END

	void ClearAllMembers();

	//void Localize();
	void LoadSettings();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DrawTreeItemHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

private:
};
