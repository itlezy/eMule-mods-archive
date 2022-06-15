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

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->

class CPPgArgos : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgArgos)

public:
	CPPgArgos();
	virtual ~CPPgArgos();

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	CTreeOptionsCtrl m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiZeroScoreGPLBreaker;
	HTREEITEM m_htiBanTime;
	HTREEITEM m_htiCloseMaellaBackdoor;

	// DLP Groupe
	HTREEITEM m_htiDetectionLevel;
		HTREEITEM m_htiDetectionLevel0;
		HTREEITEM m_htiDetectionLevel1;
		HTREEITEM m_htiDetectionLevel2;
		HTREEITEM m_htiLeecherModDetection;
		HTREEITEM m_htiLeecherNickDetection;
		HTREEITEM m_htiLeecherHashDetection;
	

	// Behavioural groupe
	HTREEITEM m_htiAgressionDetection;
	HTREEITEM m_htiHashChangeDetection;

	HTREEITEM m_htiUploadFakerDetection;
	HTREEITEM m_htiFileFakerDetection;
	HTREEITEM m_htiRankFloodDetection;
	HTREEITEM m_htiXsExploitDetection;
	HTREEITEM m_htiFileScannerDetection;
	HTREEITEM m_htiSpamerDetection;

	HTREEITEM m_htiHashThiefDetection;
	HTREEITEM m_htiNickThiefDetection;
	HTREEITEM m_htiModThiefDetection;

	
	bool	m_bZeroScoreGPLBreaker;
	int		m_iBanTime;
	bool	m_bCloseMaellaBackdoor;

	// DLP Groupe
	bool	m_bLeecherModDetection;
	bool	m_bLeecherNickDetection;
	bool	m_bLeecherHashDetection;
	int		m_iDetectionLevel;

	// Behavioural groupe
	UINT	m_uAgressionDetection;
	UINT	m_uHashChangeDetection;

	bool	m_bUploadFakerDetection;
	bool	m_bFileFakerDetection;
	bool	m_bRankFloodDetection;
	bool	m_bXsExploitDetection;
	bool	m_bFileScannerDetection;
	UINT	m_uSpamerDetection;

	bool	m_bHashThiefDetection;
	UINT	m_uNickThiefDetection;
	bool	m_bModThiefDetection;

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
};

#endif // ARGOS // NEO: NA END <-- Xanatos --