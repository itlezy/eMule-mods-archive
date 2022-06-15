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
#include "PPgNetwork.h"
#include "Preferences.h"
#include "Neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "Neo\Defaults.h"
#include "Neo\GUI\CP\TreeFunctions.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CPPgNetwork dialog

IMPLEMENT_DYNAMIC(CPPgNetwork, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgNetwork, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgNetwork::CPPgNetwork()
	: CPropertyPage(CPPgNetwork::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgNetwork::~CPPgNetwork()
{
}

void CPPgNetwork::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
	m_htiManageConnections = NULL; // NEO: SCM - [SmartConnectionManagement]
	m_htiConnectionControl = NULL; // NEO: OCC - [ObelixConnectionControl]
	m_htiAutoConnectionChecker = NULL; // NEO: NCC - [NeoConnectionChecker]
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	m_htiNAFC = NULL;
		m_htiNAFCEnabled = NULL;
		m_htiAdapterIP = NULL;
			m_htiISPMask = NULL;
			m_htiBindToAdapter = NULL;
		m_htiMSS = NULL;
			m_htiUseDoubleSendSize = NULL;
	m_htiPinger = NULL;
		m_htiCheckConnection = NULL;
		m_htiPingMode = NULL;
			m_htiPingICMP = NULL;
			m_htiPingRAW = NULL;
			m_htiPingUDP = NULL;
			m_htiNoTTL = NULL;
		m_htiHostToPing = NULL;
		m_htiStaticLowestPing = NULL;
#endif // NEO_BC // NEO: NBC END
	m_htiTCPDisableNagle = NULL;
	// NEO: QS - [QuickStart]
	m_htiQuickStart = NULL;
		m_htiQuickStartEnable = NULL;
		m_htiQuickStartTime = NULL;
		m_htiQuickStartTimePerFile = NULL;
		m_htiQuickMaxConperFive = NULL;
		m_htiQuickMaxHalfOpen = NULL;
		m_htiQuickMaxConnections = NULL;
	// NEO: QS END
	// NEO: RIC - [ReaskOnIDChange]
	m_htiOnIDChange = NULL;
		m_htiCheckIPChange = NULL;
		m_htiInformOnBuddyChange = NULL;
		m_htiInformOnIPChange = NULL;
		m_htiReAskOnIPChange = NULL;
		m_htiQuickStartOnIPChange = NULL; // NEO: QS - [QuickStart]
		m_htiCheckL2HIDChange = NULL;
		m_htiReconnectKadOnIPChange = NULL;
		m_htiRebindSocketsOnIPChange = NULL;
	// NEO: RIC END
	m_htiRecheckKadFirewalled = NULL; // NEO: RKF - [RecheckKadFirewalled]
	m_htiReConnectOnLowID = NULL; // NEO: RLD - [ReconnectOnLowID]

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_htiNAT = NULL;
		m_htiNatTraversalEnabled = NULL;
		m_htiLowIDUploadCallBack = NULL; // NEO: LUC - [LowIDUploadCallBack]
		m_htiReuseTCPPort = NULL; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END
}

void CPPgNetwork::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgNetwork = 8;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		int iImgNAFC = 8;
		int iImgPinger = 8;
		int iImgPing = 8;
#endif // NEO_BC // NEO: NBC END 
		int iImgQuickStart = 8; // NEO: QS - [QuickStart]
		int iImgReaskOnIDChange = 8; // NEO: RIC - [ReaskOnIDChange]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		int iImgNAT = 8; 
#endif //NATTUNNELING // NEO: NATT END
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgNetwork = piml->Add(CTempIconLoader(_T("NETWORKTWEAKS")));
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			iImgNAFC = piml->Add(CTempIconLoader(_T("NAFC")));
			iImgPinger = piml->Add(CTempIconLoader(_T("PINGER")));
			iImgPing = piml->Add(CTempIconLoader(_T("PING")));
#endif // NEO_BC // NEO: NBC END
			iImgQuickStart = piml->Add(CTempIconLoader(_T("QUICKSTART"))); // NEO: QS - [QuickStart]
			iImgReaskOnIDChange = piml->Add(CTempIconLoader(_T("REASKONIDCHANGE"))); // NEO: RIC - [ReaskOnIDChange]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
			iImgNAT = piml->Add(CTempIconLoader(_T("NAT"))); 
#endif //NATTUNNELING // NEO: NATT END
		}

		SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiManageConnections,GetResString(IDS_X_MANAGE_CONNECTIONS),TVI_ROOT,GetResString(IDS_X_MANAGE_CONNECTIONS_INFO),FALSE,m_bManageConnections); // NEO: SCM - [SmartConnectionManagement]
		SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiConnectionControl,GetResString(IDS_X_CONTROL_CONNECTIONS),TVI_ROOT,GetResString(IDS_X_CONTROL_CONNECTIONS_INFO),FALSE,m_bConnectionControl); // NEO: OCC - [ObelixConnectionControl]
		SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiAutoConnectionChecker,GetResString(IDS_X_CONNECTION_CHECK),TVI_ROOT,GetResString(IDS_X_CONNECTION_CHECK_INFO),FALSE,m_bAutoConnectionChecker); // NEO: NCC - [NeoConnectionChecker]
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		SetTreeGroup(m_ctrlTreeOptions,m_htiNAFC,GetResString(IDS_X_NAFC),iImgNAFC, TVI_ROOT, GetResString(IDS_X_NAFC_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiNAFCEnabled,GetResString(IDS_X_NAFC_ENABLE),m_htiNAFC,GetResString(IDS_X_NAFC_ENABLE_INFO),TRUE,m_uNAFCEnabled);
			SetTreeAdapterIndex(m_ctrlTreeOptions,m_htiAdapterIP,GetResString(IDS_X_NAFC_ADAPTER),m_htiNAFC,GetResString(IDS_X_NAFC_ADAPTER_INFO),FALSE,TRUE,FALSE, m_bISPCustomIP);
				SetTreeAdapterIndex(m_ctrlTreeOptions,m_htiISPMask,GetResString(IDS_X_NAFC_ADAPTER_MASK),m_htiAdapterIP,GetResString(IDS_X_NAFC_ADAPTER_MASK_INFO),TRUE);
				SetTreeCheck(m_ctrlTreeOptions,m_htiBindToAdapter,GetResString(IDS_X_BIND),m_htiAdapterIP,GetResString(IDS_X_BIND_INFO),FALSE,m_bBindToAdapter);
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiMSS,GetResString(IDS_X_MSS), m_htiNAFC,GetResString(IDS_X_MSS_INFO),FALSE,m_bAutoMSS);
				SetTreeCheck(m_ctrlTreeOptions,m_htiUseDoubleSendSize,GetResString(IDS_X_USE_DOUBLE_SEND_SIZE),m_htiMSS,GetResString(IDS_X_USE_DOUBLE_SEND_SIZE_INFO),FALSE,m_bUseDoubleSendSize);
		SetTreeGroup(m_ctrlTreeOptions,m_htiPinger,GetResString(IDS_X_PINGER),iImgPinger, TVI_ROOT, GetResString(IDS_X_PINGER_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiCheckConnection,GetResString(IDS_X_CHECK_CONNECTION),m_htiPinger,GetResString(IDS_X_CHECK_CONNECTION_INFO),FALSE,m_bCheckConnection);
			SetTreeGroup(m_ctrlTreeOptions,m_htiPingMode,GetResString(IDS_X_PING),iImgPing, m_htiPinger, GetResString(IDS_X_PING_INFO));
				SetTreeRadio(m_ctrlTreeOptions,m_htiPingICMP,GetResString(IDS_X_PING_ICMP),m_htiPingMode,GetResString(IDS_X_PING_ICMP_INFO),FALSE,m_iPingMode == 0);
				SetTreeRadio(m_ctrlTreeOptions,m_htiPingRAW,GetResString(IDS_X_PING_RAW),m_htiPingMode,GetResString(IDS_X_PING_RAW_INFO),FALSE,m_iPingMode == 1);
				SetTreeRadio(m_ctrlTreeOptions,m_htiPingUDP,GetResString(IDS_X_PING_UDP),m_htiPingMode,GetResString(IDS_X_PING_UDP_INFO),FALSE,m_iPingMode == 2);
				SetTreeCheck(m_ctrlTreeOptions,m_htiNoTTL,GetResString(IDS_X_NOTTL),m_htiPingMode,GetResString(IDS_X_NOTTL_INFO),FALSE,m_bNoTTL);
			SetTreeCheckEdit(m_ctrlTreeOptions,m_htiHostToPing,GetResString(IDS_X_HOST_TO_PING), m_htiPinger,GetResString(IDS_X_HOST_TO_PING_INFO),FALSE, m_bManualHostToPing);
			SetTreeCheck(m_ctrlTreeOptions,m_htiStaticLowestPing,GetResString(IDS_X_STATIC_LOWEST_PING),m_htiPinger,GetResString(IDS_X_STATIC_LOWEST_PING_INFO),FALSE,m_bStaticLowestPing);
#endif // NEO_BC // NEO: NBC END

		SetTreeCheck(m_ctrlTreeOptions,m_htiTCPDisableNagle,GetResString(IDS_X_TCP_DISABLE_NAGLE),TVI_ROOT,GetResString(IDS_X_TCP_DISABLE_NAGLE_INFO),FALSE,m_bTCPDisableNagle);

		// NEO: QS - [QuickStart]
		SetTreeGroup(m_ctrlTreeOptions,m_htiQuickStart,GetResString(IDS_X_QUICK_START),iImgQuickStart, TVI_ROOT, GetResString(IDS_X_QUICK_START_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiQuickStartEnable,GetResString(IDS_X_QUICK_START_ENABLE),m_htiQuickStart,GetResString(IDS_X_QUICK_START_ENABLE_INFO),FALSE,m_uQuickStart);
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiQuickStartTime,GetResString(IDS_X_QUICK_START_TIME), m_htiQuickStart,GetResString(IDS_X_QUICK_START_TIME_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiQuickStartTimePerFile,GetResString(IDS_X_QUICK_START_TIME_PER_FILE), m_htiQuickStart,GetResString(IDS_X_QUICK_START_TIME_PER_FILE_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiQuickMaxConperFive,GetResString(IDS_X_QUICK_START_MAXPER5), m_htiQuickStart,GetResString(IDS_X_QUICK_START_MAXPER5_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiQuickMaxHalfOpen,GetResString(IDS_X_QUICK_START_MAXHALF), m_htiQuickStart,GetResString(IDS_X_QUICK_START_MAXHALF_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiQuickMaxConnections,GetResString(IDS_X_QUICK_START_MAXCON), m_htiQuickStart,GetResString(IDS_X_QUICK_START_MAXCON_INFO));
		// NEO: QS END

		// NEO: RIC - [ReaskOnIDChange]
		SetTreeGroup(m_ctrlTreeOptions,m_htiOnIDChange,GetResString(IDS_X_ON_ID_CHANGE),iImgReaskOnIDChange, TVI_ROOT, GetResString(IDS_X_ON_ID_CHANGE_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiCheckIPChange,GetResString(IDS_X_CHECK_ID_CHANGES),m_htiOnIDChange,GetResString(IDS_X_CHECK_ID_CHANGES_INFO),TRUE,m_uCheckIPChange);
				SetTreeCheck(m_ctrlTreeOptions,m_htiInformOnBuddyChange,GetResString(IDS_X_CHECK_BUDDY_CHANGES),m_htiCheckIPChange,GetResString(IDS_X_CHECK_BUDDY_CHANGES_INFO),FALSE,m_bInformOnBuddyChange);
				SetTreeCheck(m_ctrlTreeOptions,m_htiCheckL2HIDChange,GetResString(IDS_X_CHECK_L2H_CHANGES),m_htiCheckIPChange,GetResString(IDS_X_CHECK_L2H_CHANGES_INFO),FALSE,m_bCheckL2HIDChange);

			SetTreeCheck(m_ctrlTreeOptions,m_htiInformOnIPChange,GetResString(IDS_X_INFORM_ON_ID_CHANGE),m_htiOnIDChange,GetResString(IDS_X_INFORM_ON_ID_CHANGE_INFO),FALSE,m_bInformOnIPChange);
			SetTreeCheck(m_ctrlTreeOptions,m_htiReAskOnIPChange,GetResString(IDS_X_REASK_ON_ID_CHANGE),m_htiOnIDChange,GetResString(IDS_X_REASK_ON_ID_CHANGE_INFO),FALSE,m_bReAskOnIPChange);
				SetTreeCheck(m_ctrlTreeOptions,m_htiQuickStartOnIPChange,GetResString(IDS_X_QUICK_START_ON_ID_CHANGE),m_htiReAskOnIPChange,GetResString(IDS_X_QUICK_START_ON_ID_CHANGE_INFO),FALSE,m_bQuickStartOnIPChange); // NEO: QS - [QuickStart]
			SetTreeCheck(m_ctrlTreeOptions,m_htiReconnectKadOnIPChange,GetResString(IDS_X_RECONNECTKAD_ON_ID_CHANGE),m_htiOnIDChange,GetResString(IDS_X_RECONNECTKAD_ON_ID_CHANGE_INFO),FALSE,m_bReconnectKadOnIPChange);
			SetTreeCheck(m_ctrlTreeOptions,m_htiRebindSocketsOnIPChange,GetResString(IDS_X_REBIND_SOCKETS_ON_IP_CHANGE),m_htiOnIDChange,GetResString(IDS_X_REBIND_SOCKETS_ON_IP_CHANGE_INFO),FALSE,m_bRebindSocketsOnIPChange);
		// NEO: RIC END

		SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiRecheckKadFirewalled,GetResString(IDS_FIREWALLED_CHECK), TVI_ROOT,GetResString(IDS_FIREWALLED_CHECK_INFO),FALSE, m_bRecheckKadFirewalled); // NEO: RKF - [RecheckKadFirewalled]

		SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiReConnectOnLowID,GetResString(IDS_LOW_ID_RETRY), TVI_ROOT,GetResString(IDS_LOW_ID_RETRY_INFO),FALSE, m_bReConnectOnLowID); // NEO: RLD - [ReconnectOnLowID]

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		SetTreeGroup(m_ctrlTreeOptions,m_htiNAT,GetResString(IDS_X_NAT),iImgNAT, TVI_ROOT, GetResString(IDS_X_NAT_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiNatTraversalEnabled,GetResString(IDS_X_NAT_SUPPORT_ENABLE),m_htiNAT,GetResString(IDS_X_NAT_SUPPORT_ENABLE_INFO),FALSE,m_bNatTraversalEnabled);
			SetTreeCheck(m_ctrlTreeOptions,m_htiLowIDUploadCallBack,GetResString(IDS_X_LOWID_CALLBACK),m_htiNAT,GetResString(IDS_X_LOWID_CALLBACK_INFO),TRUE,m_uLowIDUploadCallBack);  // NEO: LUC - [LowIDUploadCallBack]
			SetTreeCheck(m_ctrlTreeOptions,m_htiReuseTCPPort,GetResString(IDS_X_REUSE_TCP_PORT),m_htiNAT,GetResString(IDS_X_REUSE_TCP_PORT_INFO),FALSE,m_bReuseTCPPort); // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

		UINT bCheck;

		// NEO: OCC - [ObelixConnectionControl]
		m_ctrlTreeOptions.GetCheckBox(m_htiConnectionControl, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiConnectionControl, bCheck,FALSE,TRUE);
		// NEO: OCC END

		// NEO: SCM - [SmartConnectionManagement]
		m_ctrlTreeOptions.GetCheckBox(m_htiManageConnections, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiManageConnections, bCheck,FALSE,TRUE);
		// NEO: SCM END

		// NEO: NCC - [NeoConnectionChecker]
		m_ctrlTreeOptions.GetCheckBox(m_htiAutoConnectionChecker, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiAutoConnectionChecker, bCheck,FALSE,TRUE);
		// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		CheckNAFCEnable();

		m_ctrlTreeOptions.GetCheckBox(m_htiMSS, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiMSS, !bCheck,FALSE,TRUE);
#endif // NEO_BC // NEO: NBC END

		// NEO: QS - [QuickStart]
		m_ctrlTreeOptions.SetItemEnable(m_htiQuickStartTime, m_uQuickStart); 
		m_ctrlTreeOptions.SetItemEnable(m_htiQuickStartTimePerFile, m_uQuickStart); 
		m_ctrlTreeOptions.SetItemEnable(m_htiQuickMaxConperFive, m_uQuickStart); 
		m_ctrlTreeOptions.SetItemEnable(m_htiQuickMaxHalfOpen, m_uQuickStart); 
		m_ctrlTreeOptions.SetItemEnable(m_htiQuickMaxConnections, m_uQuickStart); 
		// NEO: QS END

		// NEO: RIC - [ReaskOnIDChange]
		
		m_ctrlTreeOptions.SetItemEnable(m_htiInformOnBuddyChange,m_uCheckIPChange); 
		m_ctrlTreeOptions.SetItemEnable(m_htiCheckL2HIDChange,m_uCheckIPChange); 
		m_ctrlTreeOptions.SetItemEnable(m_htiInformOnIPChange,m_uCheckIPChange); 
		m_ctrlTreeOptions.SetItemEnable(m_htiReAskOnIPChange,m_uCheckIPChange); 
		m_ctrlTreeOptions.SetItemEnable(m_htiQuickStartOnIPChange,m_uCheckIPChange && m_uQuickStart); 
		m_ctrlTreeOptions.SetItemEnable(m_htiReconnectKadOnIPChange,m_uCheckIPChange); 
		m_ctrlTreeOptions.SetItemEnable(m_htiRebindSocketsOnIPChange,m_uCheckIPChange); 
		// NEO: RIC END

		m_ctrlTreeOptions.Expand(TVI_ROOT, TVE_EXPAND);
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		m_ctrlTreeOptions.Expand(m_htiNAT, TVE_EXPAND);
#endif //NATTUNNELING // NEO: NATT END
		
		m_bInitializedTreeOpts = true;
	}

	// NEO: OCC - [ObelixConnectionControl]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiConnectionControl, m_bConnectionControl);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiConnectionControl, m_iConnectionControlValue);
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiManageConnections, m_bManageConnections);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiManageConnections, m_fManageConnectionsFactor);
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiAutoConnectionChecker, m_bAutoConnectionChecker);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiAutoConnectionChecker, m_iAutoConnectionCheckerValue);
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiNAFCEnabled, m_uNAFCEnabled);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiAdapterIP, m_bISPCustomIP);
		DDX_AdapterIndex(&m_ctrlTreeOptions,pDX, IDC_MOD_OPTS, m_htiAdapterIP, m_uISPZone);
			DDX_AdapterIndex(&m_ctrlTreeOptions,pDX, IDC_MOD_OPTS, m_htiISPMask, m_uISPMask);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiBindToAdapter, m_bBindToAdapter);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMSS, m_bAutoMSS); 
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMSS, m_iMSS);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseDoubleSendSize, m_bUseDoubleSendSize); 

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckConnection, m_bCheckConnection);
			DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiPingMode, m_iPingMode);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiNoTTL, m_bNoTTL);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiHostToPing, m_bManualHostToPing);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiHostToPing, m_sPingServer);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiStaticLowestPing, m_bStaticLowestPing);
#endif // NEO_BC // NEO: NBC END

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiTCPDisableNagle, m_bTCPDisableNagle);

		// NEO: QS - [QuickStart]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiQuickStartEnable, m_uQuickStart);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQuickStartTime, m_iQuickStartTime);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQuickStartTimePerFile, m_iQuickStartTimePerFile);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQuickMaxConperFive, m_iQuickMaxConperFive);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQuickMaxHalfOpen, m_iQuickMaxHalfOpen);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiQuickMaxConnections, m_iQuickMaxConnections);
		// NEO: QS END

		// NEO: RIC - [ReaskOnIDChange]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckIPChange, m_uCheckIPChange);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiInformOnIPChange, m_bInformOnIPChange);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiInformOnBuddyChange, m_bInformOnBuddyChange);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiReAskOnIPChange, m_bReAskOnIPChange);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiQuickStartOnIPChange, m_bQuickStartOnIPChange); // NEO: QS - [QuickStart]
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckL2HIDChange, m_bCheckL2HIDChange);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiReconnectKadOnIPChange, m_bReconnectKadOnIPChange);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRebindSocketsOnIPChange, m_bRebindSocketsOnIPChange);
		// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRecheckKadFirewalled, m_bRecheckKadFirewalled);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiRecheckKadFirewalled, m_iRecheckKadFirewalled);
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiReConnectOnLowID, m_bReConnectOnLowID);
	DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiReConnectOnLowID, m_iReConnectOnLowID);
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiNatTraversalEnabled, m_bNatTraversalEnabled); 
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLowIDUploadCallBack, m_uLowIDUploadCallBack); // NEO: LUC - [LowIDUploadCallBack]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiReuseTCPPort, m_bReuseTCPPort); // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END
}

BOOL CPPgNetwork::OnInitDialog()
{
	LoadSettings();

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgNetwork::LoadSettings()
{
	/*
	* Globale Einstellungen Laden
	*/

	// NEO: OCC - [ObelixConnectionControl]
	m_bConnectionControl = NeoPrefs.m_bConnectionControl;
	m_iConnectionControlValue = NeoPrefs.m_iConnectionControlValue;
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	m_bManageConnections = NeoPrefs.m_bManageConnections;
	m_fManageConnectionsFactor = NeoPrefs.m_fManageConnectionsFactor;
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	m_bAutoConnectionChecker = NeoPrefs.m_bAutoConnectionChecker;
	m_iAutoConnectionCheckerValue = NeoPrefs.m_iAutoConnectionCheckerValue;
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	m_uNAFCEnabled = NeoPrefs.m_uNAFCEnabled;
	m_bISPCustomIP = NeoPrefs.m_bISPCustomIP;
	m_uISPZone = NeoPrefs.m_uISPZone;
	m_uISPMask = NeoPrefs.m_uISPMask;
	m_bBindToAdapter = NeoPrefs.m_bBindToAdapter;

	m_bCheckConnection = NeoPrefs.m_bCheckConnection;
	m_bStaticLowestPing = NeoPrefs.m_bStaticLowestPing;
	m_iPingMode = NeoPrefs.m_iPingMode;
	m_bNoTTL = NeoPrefs.m_bNoTTL;
	m_bManualHostToPing = NeoPrefs.m_bManualHostToPing;
	m_sPingServer = NeoPrefs.m_sPingServer;

	m_bAutoMSS = NeoPrefs.m_bAutoMSS;
	m_iMSS = NeoPrefs.m_iMSS;

	m_bUseDoubleSendSize = NeoPrefs.m_bUseDoubleSendSize;
#endif // NEO_BC // NEO: NBC END

	m_bTCPDisableNagle = NeoPrefs.m_bTCPDisableNagle;

	// NEO: QS - [QuickStart]
	m_uQuickStart = NeoPrefs.m_uQuickStart;
	m_iQuickStartTime = NeoPrefs.m_iQuickStartTime;
	m_iQuickStartTimePerFile = NeoPrefs.m_iQuickStartTimePerFile;
	m_iQuickMaxConperFive = NeoPrefs.m_iQuickMaxConperFive;
	m_iQuickMaxHalfOpen = NeoPrefs.m_iQuickMaxHalfOpen;
	m_iQuickMaxConnections = NeoPrefs.m_iQuickMaxConnections;
	// NEO: QS END

	// NEO: RIC - [ReaskOnIDChange]
	m_uCheckIPChange = NeoPrefs.m_uCheckIPChange;
	m_bInformOnIPChange = NeoPrefs.m_bInformOnIPChange;
	m_bInformOnBuddyChange = NeoPrefs.m_bInformOnBuddyChange;
	m_bReAskOnIPChange = NeoPrefs.m_bReAskOnIPChange;
	m_bQuickStartOnIPChange = NeoPrefs.m_bQuickStartOnIPChange; // NEO: QS - [QuickStart]
	m_bCheckL2HIDChange = NeoPrefs.m_bCheckL2HIDChange;
	m_bReconnectKadOnIPChange = NeoPrefs.m_bReconnectKadOnIPChange;
	m_bRebindSocketsOnIPChange = NeoPrefs.m_bRebindSocketsOnIPChange;
	// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	m_bRecheckKadFirewalled = NeoPrefs.m_bRecheckKadFirewalled;
	m_iRecheckKadFirewalled = NeoPrefs.m_iRecheckKadFirewalled;
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	m_bReConnectOnLowID = NeoPrefs.m_bReConnectOnLowID;
	m_iReConnectOnLowID = NeoPrefs.m_iReConnectOnLowID;
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_bNatTraversalEnabled = NeoPrefs.m_bNatTraversalEnabled;
	m_uLowIDUploadCallBack = NeoPrefs.m_uLowIDUploadCallBack; // NEO: LUC - [LowIDUploadCallBack]
	m_bReuseTCPPort = NeoPrefs.m_bReuseTCPPort; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END
}

BOOL CPPgNetwork::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	/*
	* Globale Einstellungen Speichern
	*/

	// NEO: OCC - [ObelixConnectionControl]
	NeoPrefs.m_bConnectionControl = m_bConnectionControl;
	NeoPrefs.m_iConnectionControlValue = m_iConnectionControlValue;
	// NEO: OCC END

	// NEO: SCM - [SmartConnectionManagement]
	NeoPrefs.m_bManageConnections = m_bManageConnections;
	NeoPrefs.m_fManageConnectionsFactor = m_fManageConnectionsFactor;
	// NEO: SCM END

	// NEO: NCC - [NeoConnectionChecker]
	NeoPrefs.m_bAutoConnectionChecker = m_bAutoConnectionChecker;
	NeoPrefs.m_iAutoConnectionCheckerValue = m_iAutoConnectionCheckerValue;
	// NEO: NCC END

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	NeoPrefs.m_uNAFCEnabled = m_uNAFCEnabled;
	bool ReConNAFC = false;
	if(NeoPrefs.m_bISPCustomIP != m_bISPCustomIP || NeoPrefs.m_uISPZone != m_uISPZone || NeoPrefs.m_uISPMask != m_uISPMask)
		ReConNAFC = true;
	NeoPrefs.m_bISPCustomIP = m_bISPCustomIP;
	NeoPrefs.m_uISPZone = m_uISPZone;
	NeoPrefs.m_uISPMask = m_uISPMask;
	bool UnBind = false;
	if(NeoPrefs.m_bBindToAdapter != m_bBindToAdapter){
		if(m_bBindToAdapter)
			ReConNAFC = true;
		else
			UnBind = true;
	}
	NeoPrefs.m_bBindToAdapter = m_bBindToAdapter;
	if(ReConNAFC)
		theApp.bandwidthControl->SetAdapterIndex();
	// NEO: MOD - [BindToAdapter]
	if(UnBind)
		theApp.BindToAddress();
	// NEO: MOD END

	NeoPrefs.m_bCheckConnection = m_bCheckConnection;
	NeoPrefs.m_bStaticLowestPing = m_bStaticLowestPing;
	NeoPrefs.m_iPingMode = m_iPingMode;
	NeoPrefs.m_bNoTTL = m_bNoTTL;
	NeoPrefs.m_bManualHostToPing = m_bManualHostToPing;
	if (NeoPrefs.m_sPingServer != m_sPingServer){
		NeoPrefs.m_sPingServer = m_sPingServer;
		if(m_bManualHostToPing){
			if (!theApp.bandwidthControl->SetPingedServer(m_sPingServer))
				AfxMessageBox(GetResString(IDS_X_PING_SVR_ERR),MB_OK | MB_ICONERROR,NULL);
		}
	}

	NeoPrefs.m_bAutoMSS = m_bAutoMSS;
	NeoPrefs.m_iMSS = m_iMSS;

	NeoPrefs.m_bUseDoubleSendSize = m_bUseDoubleSendSize;
#endif // NEO_BC // NEO: NBC END

	NeoPrefs.m_bTCPDisableNagle = m_bTCPDisableNagle;

	// NEO: QS - [QuickStart]
	NeoPrefs.m_uQuickStart = m_uQuickStart;
	NeoPrefs.m_iQuickStartTime = m_iQuickStartTime;
	NeoPrefs.m_iQuickStartTimePerFile = m_iQuickStartTimePerFile;
	NeoPrefs.m_iQuickMaxConperFive = m_iQuickMaxConperFive;
	NeoPrefs.m_iQuickMaxHalfOpen = m_iQuickMaxHalfOpen;
	NeoPrefs.m_iQuickMaxConnections = m_iQuickMaxConnections;
	// NEO: QS END

	// NEO: RIC - [ReaskOnIDChange]
	NeoPrefs.m_uCheckIPChange = m_uCheckIPChange;
	NeoPrefs.m_bInformOnIPChange = m_bInformOnIPChange;
	NeoPrefs.m_bInformOnBuddyChange = m_bInformOnBuddyChange;
	NeoPrefs.m_bReAskOnIPChange = m_bReAskOnIPChange;
	NeoPrefs.m_bQuickStartOnIPChange = m_bQuickStartOnIPChange; // NEO: QS - [QuickStart]
	NeoPrefs.m_bCheckL2HIDChange = m_bCheckL2HIDChange;
	NeoPrefs.m_bReconnectKadOnIPChange = m_bReconnectKadOnIPChange;
	NeoPrefs.m_bRebindSocketsOnIPChange = m_bRebindSocketsOnIPChange;
	// NEO: RIC END

	// NEO: RKF - [RecheckKadFirewalled]
	NeoPrefs.m_bRecheckKadFirewalled = m_bRecheckKadFirewalled;
	NeoPrefs.m_iRecheckKadFirewalled = m_iRecheckKadFirewalled;
	// NEO: RKF END

	// NEO: RLD - [ReconnectOnLowID]
	NeoPrefs.m_bReConnectOnLowID = m_bReConnectOnLowID;
	NeoPrefs.m_iReConnectOnLowID = m_iReConnectOnLowID;
	// NEO: RLD END

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	NeoPrefs.m_bNatTraversalEnabled = m_bNatTraversalEnabled;
	NeoPrefs.m_uLowIDUploadCallBack = m_uLowIDUploadCallBack; // NEO: LUC - [LowIDUploadCallBack]
	NeoPrefs.m_bReuseTCPPort = m_bReuseTCPPort; // NEO: RTP - [ReuseTCPPort]
#endif //NATTUNNELING // NEO: NATT END

	NeoPrefs.CheckNeoPreferences();
	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgNetwork::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgNetwork::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

LRESULT CPPgNetwork::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){		
			// NEO: OCC - [ObelixConnectionControl]
			if (m_htiConnectionControl && pton->hItem == m_htiConnectionControl){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiConnectionControl, VAL_CON_CTR_MIN, VAL_CON_CTR_DEF, VAL_CON_CTR_MAX)) SetModified();
			}
			// NEO: OCC END
			// NEO: SCM - [SmartConnectionManagement]
			else if (m_htiManageConnections && pton->hItem == m_htiManageConnections){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiManageConnections, VAL_CON_MAN_MIN, VAL_CON_MAN_DEF, VAL_CON_MAN_MAX)) SetModified();
			}
			// NEO: SCM END
			// NEO: NCC - [NeoConnectionChecker]
			else if (m_htiAutoConnectionChecker && pton->hItem == m_htiAutoConnectionChecker){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoConnectionChecker, VAL_CON_CHK_MIN, VAL_CON_CHK_DEF, VAL_CON_CHK_MAX)) SetModified();
			}
			// NEO: NCC END
			// NEO: RKF - [RecheckKadFirewalled]
			else if(m_htiRecheckKadFirewalled && pton->hItem == m_htiRecheckKadFirewalled){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiRecheckKadFirewalled, TIM_RECHECK_KAD_FIREWALLED_MIN, TIM_RECHECK_KAD_FIREWALLED_DEF, TIM_RECHECK_KAD_FIREWALLED_MAX)) SetModified();
			}
			// NEO: RKF END
			// NEO: RLD - [ReconnectOnLowID]
			else if(m_htiReConnectOnLowID && pton->hItem == m_htiReConnectOnLowID){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiReConnectOnLowID, VAL_RECONNECT_ON_LOWID_TRYS_MIN, VAL_RECONNECT_ON_LOWID_TRYS_DEF, VAL_RECONNECT_ON_LOWID_TRYS_MAX)) SetModified();
			}
			// NEO: RLD END

		}else{

			UINT bCheck;
			// NEO: OCC - [ObelixConnectionControl]
			if(m_htiConnectionControl && pton->hItem == m_htiConnectionControl){
				m_ctrlTreeOptions.GetCheckBox(m_htiConnectionControl, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiConnectionControl, bCheck,FALSE,TRUE);
			}
			// NEO: OCC END
			// NEO: SCM - [SmartConnectionManagement]
			else if(m_htiManageConnections && pton->hItem == m_htiManageConnections){
				m_ctrlTreeOptions.GetCheckBox(m_htiManageConnections, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiManageConnections, bCheck,FALSE,TRUE);
			}
			// NEO: SCM END
			// NEO: NCC - [NeoConnectionChecker]
			else if(m_htiAutoConnectionChecker && pton->hItem == m_htiAutoConnectionChecker){
				m_ctrlTreeOptions.GetCheckBox(m_htiAutoConnectionChecker, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiAutoConnectionChecker, bCheck,FALSE,TRUE);
			}
			// NEO: NCC END
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			else if (m_htiNAFCEnabled && pton->hItem == m_htiNAFCEnabled){
				CheckNAFCEnable();
			}else if (m_htiAdapterIP && pton->hItem == m_htiAdapterIP){
				CheckNAFCEnable();
			}else if (m_htiMSS && pton->hItem == m_htiMSS){
				m_ctrlTreeOptions.GetCheckBox(m_htiMSS, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiMSS, !bCheck,FALSE,TRUE);
			}
#endif // NEO_BC // NEO: NBC END

			// NEO: QS - [QuickStart]
			else if (m_htiQuickStartEnable && pton->hItem == m_htiQuickStartEnable){
				m_ctrlTreeOptions.GetCheckBox(m_htiQuickStartEnable, bCheck);
				
				m_ctrlTreeOptions.SetItemEnable(m_htiQuickStartTime, bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiQuickStartTimePerFile, bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiQuickMaxConperFive, bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiQuickMaxHalfOpen, bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiQuickMaxConnections, bCheck); 
			}
			// NEO: QS END

			// NEO: RIC - [ReaskOnIDChange]
			else if (m_htiCheckIPChange && pton->hItem == m_htiCheckIPChange){
				m_ctrlTreeOptions.GetCheckBox(m_htiCheckIPChange, bCheck);

				m_ctrlTreeOptions.SetItemEnable(m_htiInformOnBuddyChange,bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiCheckL2HIDChange,bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiInformOnIPChange,bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiReAskOnIPChange,bCheck); 
				UINT bCheck2;
				m_ctrlTreeOptions.GetCheckBox(m_htiQuickStartEnable, bCheck2);
				m_ctrlTreeOptions.SetItemEnable(m_htiQuickStartOnIPChange,bCheck && bCheck2); 
				m_ctrlTreeOptions.SetItemEnable(m_htiReconnectKadOnIPChange,bCheck); 
				m_ctrlTreeOptions.SetItemEnable(m_htiRebindSocketsOnIPChange,bCheck); 	
			}
			// NEO: RIC END

			// NEO: RKF - [RecheckKadFirewalled]
			else if(m_htiRecheckKadFirewalled && pton->hItem == m_htiRecheckKadFirewalled){
				m_ctrlTreeOptions.GetCheckBox(m_htiRecheckKadFirewalled, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiRecheckKadFirewalled, bCheck,FALSE,TRUE);
			}
			// NEO: RKF END

			// NEO: RLD - [ReconnectOnLowID]
			else if(m_htiReConnectOnLowID && pton->hItem == m_htiReConnectOnLowID){
				m_ctrlTreeOptions.GetCheckBox(m_htiReConnectOnLowID, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiReConnectOnLowID, bCheck,FALSE,TRUE);
			}
			// NEO: RLD END

			SetModified();
		}
	}
	return 0;
}

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
void CPPgNetwork::CheckNAFCEnable()
{
	UINT bCheck1;
	m_ctrlTreeOptions.GetCheckBox(m_htiNAFCEnabled, bCheck1);
	m_ctrlTreeOptions.SetItemEnable(m_htiAdapterIP, bCheck1);

	UINT bCheck2;
	m_ctrlTreeOptions.GetCheckBox(m_htiAdapterIP, bCheck2);
	m_ctrlTreeOptions.SetItemEnable(m_htiBindToAdapter, (bCheck1 && bCheck2));
}
#endif // NEO_BC // NEO: NBC END

LRESULT CPPgNetwork::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgNetwork::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgNetwork::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgNetwork::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
