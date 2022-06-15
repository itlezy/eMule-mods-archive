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

class CPPgNetwork : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgNetwork)

public:
	CPPgNetwork();
	virtual ~CPPgNetwork();

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	CTreeOptionsCtrl m_ctrlTreeOptions; // NEO - [TreeControl] <-- Xanatos --
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiManageConnections; // NEO: SCM - [SmartConnectionManagement]
	HTREEITEM m_htiConnectionControl; // NEO: OCC - [ObelixConnectionControl]
	HTREEITEM m_htiAutoConnectionChecker; // NEO: NCC - [NeoConnectionChecker]
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	HTREEITEM m_htiNAFC;
		HTREEITEM m_htiNAFCEnabled;
		HTREEITEM m_htiAdapterIP;
			HTREEITEM m_htiISPMask;
			HTREEITEM m_htiBindToAdapter;
		HTREEITEM m_htiMSS;
			HTREEITEM m_htiUseDoubleSendSize;
	HTREEITEM m_htiPinger;
		HTREEITEM m_htiCheckConnection;
		HTREEITEM m_htiPingMode;
			HTREEITEM m_htiPingICMP;
			HTREEITEM m_htiPingRAW;
			HTREEITEM m_htiPingUDP;
			HTREEITEM m_htiNoTTL;
		HTREEITEM m_htiHostToPing;
		HTREEITEM m_htiStaticLowestPing;
#endif // NEO_BC // NEO: NBC END
	HTREEITEM m_htiTCPDisableNagle;
	// NEO: RIC - [ReaskOnIDChange]
	HTREEITEM m_htiOnIDChange;
		HTREEITEM m_htiCheckIPChange;
		HTREEITEM m_htiInformOnBuddyChange;
		HTREEITEM m_htiInformOnIPChange;
		HTREEITEM m_htiReAskOnIPChange;
		HTREEITEM m_htiQuickStartOnIPChange; // NEO: QS - [QuickStart]
		HTREEITEM m_htiCheckL2HIDChange;
		HTREEITEM m_htiReconnectKadOnIPChange;
		HTREEITEM m_htiRebindSocketsOnIPChange;
	// NEO: RIC END
	// NEO: QS - [QuickStart]
	HTREEITEM m_htiQuickStart;
		HTREEITEM m_htiQuickStartEnable;
		HTREEITEM m_htiQuickStartTime;
		HTREEITEM m_htiQuickStartTimePerFile;
		HTREEITEM m_htiQuickMaxConperFive;
		HTREEITEM m_htiQuickMaxHalfOpen;
		HTREEITEM m_htiQuickMaxConnections;
	// NEO: QS END
	HTREEITEM m_htiRecheckKadFirewalled; // NEO: RKF - [RecheckKadFirewalled]
	HTREEITEM m_htiReConnectOnLowID; // NEO: RLD - [ReconnectOnLowID]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	HTREEITEM m_htiNAT;
		HTREEITEM m_htiNatTraversalEnabled;
		HTREEITEM m_htiLowIDUploadCallBack; // NEO: LUC - [LowIDUploadCallBack]
		HTREEITEM m_htiReuseTCPPort; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

	// NEO: OCC - [ObelixConnectionControl]
	bool	m_bConnectionControl;
	int		m_iConnectionControlValue;
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	bool	m_bManageConnections;
	float	m_fManageConnectionsFactor;
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	bool	m_bAutoConnectionChecker;
	int		m_iAutoConnectionCheckerValue;
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	UINT	m_uNAFCEnabled;
	bool	m_bISPCustomIP;
	DWORD	m_uISPZone;
	DWORD	m_uISPMask;
	bool	m_bBindToAdapter;


	bool	m_bCheckConnection;
	bool	m_bStaticLowestPing;
	int		m_iPingMode;
	bool	m_bNoTTL;
	bool	m_bManualHostToPing;
	CString m_sPingServer;

	bool	m_bAutoMSS;
	int		m_iMSS;

	bool	m_bUseDoubleSendSize;
#endif // NEO_BC // NEO: NBC END

	bool	m_bTCPDisableNagle;

	// NEO: RIC - [ReaskOnIDChange]
	UINT	m_uCheckIPChange;
	bool	m_bInformOnIPChange;
	bool	m_bInformOnBuddyChange;
	bool	m_bReAskOnIPChange;
	bool	m_bQuickStartOnIPChange; // NEO: QS - [QuickStart]
	bool	m_bCheckL2HIDChange;
	bool	m_bReconnectKadOnIPChange;
	bool	m_bRebindSocketsOnIPChange;
	// NEO: RIC END

	// NEO: QS - [QuickStart]
	UINT	m_uQuickStart;
	int		m_iQuickStartTime;
	int		m_iQuickStartTimePerFile;
	int		m_iQuickMaxConperFive;
	int		m_iQuickMaxHalfOpen;
	int		m_iQuickMaxConnections;
	// NEO: QS END

	// NEO: RKF - [RecheckKadFirewalled]
	bool	m_bRecheckKadFirewalled;
	int		m_iRecheckKadFirewalled;
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	bool	m_bReConnectOnLowID;
	int		m_iReConnectOnLowID;
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	bool	m_bNatTraversalEnabled;
	UINT	m_uLowIDUploadCallBack; // NEO: LUC - [LowIDUploadCallBack]
	bool	m_bReuseTCPPort; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

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
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	void CheckNAFCEnable();
#endif // NEO_BC // NEO: NBC END
};
