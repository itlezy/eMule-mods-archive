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

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

class CPPgBandwidth : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgBandwidth)

public:
	CPPgBandwidth();
	virtual ~CPPgBandwidth();

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	CTreeOptionsCtrl m_ctrlTreeOptions; // NEO - [TreeControl] <-- Xanatos --
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiBandwidthControl;
		HTREEITEM m_htiIncludeOverhead;
		HTREEITEM m_htiIncludeTCPAck;
			HTREEITEM m_htiConnectionsOverHead;
		HTREEITEM m_htiUploadManagement;
			HTREEITEM m_htiMinimizeOpenedSlots;
			HTREEITEM m_htiCumulateBandwidth;
			HTREEITEM m_htiBadwolfsUpload;
			HTREEITEM m_htiUpload;
				HTREEITEM m_htiMaxUploadSlots;
				HTREEITEM m_htiMinUploadSlots;
				HTREEITEM m_htiUploadPerSlots;
				HTREEITEM m_htiMaxReservedSlots;
				HTREEITEM m_htiIncreaseTrickleSpeed;
				HTREEITEM m_htiOpenMoreSlotsWhenNeeded;
				HTREEITEM m_htiCheckSlotDatarate;
			HTREEITEM m_htiTrickleBlocking;
				HTREEITEM m_htiDropBlocking;
		HTREEITEM m_htiSessionRatio;
		HTREEITEM m_htiUploadSystem;
			HTREEITEM m_htiUseBlockedQueue;
			HTREEITEM m_htiBCTimeUp;
			HTREEITEM m_htiBCPriorityUp;
			HTREEITEM m_htiUploadBufferSize;

		HTREEITEM m_htiDownloadSystem;
			HTREEITEM m_htiUseDownloadBandwidthThrottler;
			HTREEITEM m_htiUseHyperDownload;
			HTREEITEM m_htiBCTimeDown;
			HTREEITEM m_htiBCPriorityDown;
			HTREEITEM m_htiDownloadBufferSize;

		HTREEITEM m_htiDatarateSamples;

	HTREEITEM m_htiUploadControl;
		HTREEITEM m_htiUploadControlNone;
		HTREEITEM m_htiUploadControlNAFC;
		HTREEITEM m_htiUploadControlUSS;
			HTREEITEM m_htiBasePingUp;
			HTREEITEM m_htiPingUpTolerance;
			HTREEITEM m_htiPingUpProzent;
			HTREEITEM m_htiDynUpGoingDivider;
				HTREEITEM m_htiDynUpGoingUpDivider;
				HTREEITEM m_htiDynUpGoingDownDivider;
		HTREEITEM m_htiMinBCUpload;
		HTREEITEM m_htiMaxBCUpload;
	HTREEITEM m_htiDownloadControl;
		HTREEITEM m_htiDownloadControlNone;
		HTREEITEM m_htiDownloadControlNAFC;
		HTREEITEM m_htiDownloadControlDSS;
			HTREEITEM m_htiBasePingDown;
			HTREEITEM m_htiPingDownTolerance;
			HTREEITEM m_htiPingDownProzent;
			HTREEITEM m_htiDynDownGoingDivider;
				HTREEITEM m_htiDynDownGoingUpDivider;
				HTREEITEM m_htiDynDownGoingDownDivider;
		HTREEITEM m_htiMinBCDownload;
		HTREEITEM m_htiMaxBCDownload;

	// bandwidth system group (warning be carefull when switching, clear lists!)
	bool	m_bUseDownloadBandwidthThrottler;
	UINT	m_uUseHyperDownload;
	int		m_iBCTimeDown;
	int		m_iBCPriorityDown;
	UINT	m_uSetDownloadBuffer;
	int		m_iDownloadBufferSize;

	UINT	m_uUseBlockedQueue;
	int		m_iBCTimeUp;
	int		m_iBCPriorityUp;
	UINT	m_uSetUploadBuffer;
	int		m_iUploadBufferSize;

	int		m_iDatarateSamples;

	// bandwidth General group
	bool	m_bIncludeOverhead;
	bool	m_bIncludeTCPAck;
	bool	m_bConnectionsOverHead;
	bool	m_bSessionRatio;

	// Download Group
	int		m_iDownloadControl;
	float	m_fMinBCDownload;
	float	m_fMaxBCDownload;

	// Upload group
	int		m_iUploadControl;
	float	m_fMinBCUpload;
	float	m_fMaxBCUpload;

	// Nafc
	float	m_fMaxDownStream;
	float	m_fMaxUpStream;

	// Secundary upload group
	bool	m_bMinimizeOpenedSlots;
	UINT	m_uCumulateBandwidth;
	UINT	m_uBadwolfsUpload;

	// Download sub group // DSS
	bool	m_bDynUpGoingDivider;
	int		m_iDynUpGoingUpDivider;
	int		m_iDynUpGoingDownDivider;

	int		m_iUpMaxPingMethod;
	int		m_iBasePingUp;
	int		m_iPingUpTolerance;
	int		m_iPingUpProzent;

	// Upload sub group // USS
	bool	m_bDynDownGoingDivider;
	int		m_iDynDownGoingUpDivider;
	int		m_iDynDownGoingDownDivider;
	
	int		m_iDownMaxPingMethod;
	int		m_iBasePingDown;
	int		m_iPingDownTolerance;
	int		m_iPingDownProzent;

	// uplaod slor group
	int		m_iMaxUploadSlots;
	int		m_iMinUploadSlots;
	float	m_fUploadPerSlots;
	int		m_iMaxReservedSlots;

	bool	m_bIncreaseTrickleSpeed;
	float	m_fIncreaseTrickleSpeed;

	bool	m_bOpenMoreSlotsWhenNeeded;
	bool	m_bCheckSlotDatarate;

	bool	m_bIsTrickleBlocking;
	bool	m_bIsDropBlocking;

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

	void	CheckEnableUSS();
	void	CheckEnableDSS();
};

#endif // NEO_BC // NEO: NBC END <-- Xanatos --