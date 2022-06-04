//this file is part of eMule
//Copyright (C)2003 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "TreeOptionsCtrlEx.h"

class CPPgAcK2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAcK2)

public:
	CPPgAcK2();
	virtual ~CPPgAcK2();
	void Localize(void);
	// Dialog Data
	enum { IDD = IDD_PPG_AcK2 };

protected:
	void	Nullify();

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

//>>> taz::AcK Links
	afx_msg void OnBnClickedAckHelp();
	afx_msg void OnBnClickedAckHelp2(); //>>> taz::fix restore 3 link now pointing to AcKroNiC WiKi [Mulo da Soma]
	afx_msg void OnBnClickedAckHelp3();
//<<< taz::AcK Links
//>>> taz::Quick start [TPT]
	// ==> Quick start [TPT] - Stulle
	bool m_bQuickStart;
	int m_iQuickStartMaxTime;
	int m_iQuickStartMaxConnPerFive;
	int m_iQuickStartMaxConn;
	bool m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Stulle
	// ==> Quick start [TPT] - Stulle
	HTREEITEM m_htiQuickStartGroup;
	HTREEITEM m_htiQuickStart;
	HTREEITEM m_htiQuickStartMaxTime;
	HTREEITEM m_htiQuickStartMaxConnPerFive;
	HTREEITEM m_htiQuickStartMaxConn;
	HTREEITEM m_htiQuickStartAfterIPChange;
	// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
	// ==> drop sources - Stulle
	bool m_bEnableAutoDropNNSDefault;
	int m_iAutoNNS_TimerDefault;
	int m_iMaxRemoveNNSLimitDefault;
	bool m_bEnableAutoDropFQSDefault;
	int m_iAutoFQS_TimerDefault;
	int m_iMaxRemoveFQSLimitDefault;
	bool m_bEnableAutoDropQRSDefault;
	int m_iAutoHQRS_TimerDefault;
	int m_iMaxRemoveQRSDefault;
	int m_iMaxRemoveQRSLimitDefault;
	// <== drop sources - Stulle
	bool m_bDontDropCompleteSources; //>>> taz::don't drop complete sources
	// ==> drop sources - Stulle
	HTREEITEM m_htiDropDefaults;
	HTREEITEM m_htiAutoNNS;
	HTREEITEM m_htiAutoNNSTimer;
	HTREEITEM m_htiAutoNNSLimit;
	HTREEITEM m_htiAutoFQS;
	HTREEITEM m_htiAutoFQSTimer;
	HTREEITEM m_htiAutoFQSLimit;
	HTREEITEM m_htiAutoQRS;
	HTREEITEM m_htiAutoQRSTimer;
	HTREEITEM m_htiAutoQRSMax;
	HTREEITEM m_htiAutoQRSLimit;
	// <== drop sources - Stulle
	HTREEITEM m_htiDontDropCompleteSources; //>>> taz::don't drop complete sources
//<<< taz::drop sources
//>>> taz::Inform Clients after IP Change [Stulle]
	HTREEITEM m_htiIsreaskSourceAfterIPChange;
	HTREEITEM m_htiInformQueuedClientsAfterIPChange;
	bool m_bIsreaskSourceAfterIPChange;
	bool m_bInformQueuedClientsAfterIPChange;
//<<< taz::Inform Clients after IP Change [Stulle]
//>>> taz::Completed in Tray [Stulle]
	HTREEITEM m_htiTrayComplete;
	bool m_bTrayComplete;
//<< taz::Completed in Tray [Stulle]
//>>> taz::Variable corrupted blocks ban threshold [Spike2]
	HTREEITEM	m_htiBanThreshold;
	int		m_iBanThreshold;
//<<< taz::Variable corrupted blocks ban threshold [Spike2]
//>>> WiZaRd::minRQR [WiZaRd]
	HTREEITEM	m_htiMinRQR;
	bool		m_bUseMinRQR;
//<<< WiZaRd::minRQR [WiZaRd]
//>>> taz::lowid notifier [chamblard]
	HTREEITEM	m_htiShowLowID;
	bool		m_bShowLowID;
//<<< taz::lowid notifier [chamblard]
//>>> taz::More info about corrupted .met/.part file
	HTREEITEM	m_htiShowCorrupted;
	bool		m_bShowCorrupted;
//<<< taz::More info about corrupted .met/.part file

	HTREEITEM	m_htiUM;
	HTREEITEM	m_htiDM;
	HTREEITEM	m_htiNotifications;
	HTREEITEM	m_htiDisplayGroup;
};
