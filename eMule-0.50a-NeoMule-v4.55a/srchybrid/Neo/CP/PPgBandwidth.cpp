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
#include "emuledlg.h"
#include "PPgBandwidth.h"
#include "Neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "Neo\Defaults.h"
#include "Neo\GUI\CP\TreeFunctions.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#include "Neo/BC/DownloadBandwidthThrottler.h"
#include "Neo/BC/UploadBandwidthThrottler.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#include "Scheduler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CPPgBandwidth dialog

IMPLEMENT_DYNAMIC(CPPgBandwidth, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgBandwidth, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgBandwidth::CPPgBandwidth()
	: CPropertyPage(CPPgBandwidth::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgBandwidth::~CPPgBandwidth()
{
}

void CPPgBandwidth::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;
	
	m_htiBandwidthControl = NULL;
		m_htiIncludeOverhead = NULL;
		m_htiIncludeTCPAck = NULL;
			m_htiConnectionsOverHead = NULL;
		m_htiUploadManagement = NULL;
			m_htiMinimizeOpenedSlots = NULL;
			m_htiCumulateBandwidth = NULL;
			m_htiBadwolfsUpload = NULL;
			m_htiUpload = NULL;
				m_htiMaxUploadSlots = NULL;
				m_htiMinUploadSlots = NULL;
				m_htiUploadPerSlots = NULL;
				m_htiMaxReservedSlots = NULL;
				m_htiIncreaseTrickleSpeed = NULL;
				m_htiOpenMoreSlotsWhenNeeded = NULL;
				m_htiCheckSlotDatarate = NULL;
			m_htiTrickleBlocking = NULL;
				m_htiDropBlocking = NULL;
		m_htiSessionRatio = NULL;
		m_htiUploadSystem = NULL;
			m_htiUseBlockedQueue = NULL;
			m_htiBCTimeUp = NULL;
			m_htiBCPriorityUp = NULL;
			m_htiUploadBufferSize = NULL;
		m_htiDownloadSystem = NULL;
			m_htiUseDownloadBandwidthThrottler = NULL;
			m_htiUseHyperDownload = NULL;
			m_htiBCTimeDown = NULL;
			m_htiBCPriorityDown = NULL;
			m_htiDownloadBufferSize = NULL;
	m_htiDatarateSamples = NULL;
	m_htiUploadControl = NULL;
		m_htiUploadControlNone = NULL;
		m_htiUploadControlNAFC = NULL;
		m_htiUploadControlUSS = NULL;
			m_htiBasePingUp = NULL;
			m_htiPingUpTolerance = NULL;
			m_htiPingUpProzent = NULL;
			m_htiDynUpGoingDivider = NULL;
				m_htiDynUpGoingUpDivider = NULL;
				m_htiDynUpGoingDownDivider = NULL;
		m_htiMinBCUpload = NULL;
		m_htiMaxBCUpload = NULL;
	m_htiDownloadControl = NULL;
		m_htiDownloadControlNone = NULL;
		m_htiDownloadControlNAFC = NULL;
		m_htiDownloadControlDSS = NULL;
			m_htiBasePingDown = NULL;
			m_htiPingDownTolerance = NULL;
			m_htiPingDownProzent = NULL;
			m_htiDynDownGoingDivider = NULL;
				m_htiDynDownGoingUpDivider = NULL;
				m_htiDynDownGoingDownDivider = NULL;
		m_htiMinBCDownload = NULL;
		m_htiMaxBCDownload = NULL;

}

void CPPgBandwidth::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgBandwidth = 8;
		int iImgControl = 8;
		int iImgUploadManagement = 8;
		int iImgUpload = 8;
		int iImgUploadSystem = 8;
		int iImgDownloadSystem = 8;
		int iImgUploadControl = 8;
		int iImgDownloadControl = 8;
        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgBandwidth = piml->Add(CTempIconLoader(_T("BANDWIDTH")));
			iImgControl = piml->Add(CTempIconLoader(_T("BANDWIDTHGENERAL")));
			iImgUploadManagement = piml->Add(CTempIconLoader(_T("UPLOADMANAGEMENT")));
			iImgUpload = piml->Add(CTempIconLoader(_T("UPLOADTWEAKS")));
			iImgUploadSystem = piml->Add(CTempIconLoader(_T("UPLOADSYSTEM")));
			iImgDownloadSystem = piml->Add(CTempIconLoader(_T("DOWNLOADSYSTEM")));
			iImgUploadControl = piml->Add(CTempIconLoader(_T("UPLOADCONTROL")));
			iImgDownloadControl = piml->Add(CTempIconLoader(_T("DOWNLOADCONTROL")));
		}

		SetTreeGroup(m_ctrlTreeOptions,m_htiBandwidthControl,GetResString(IDS_X_BANDWIDTH_GENERAL),iImgControl, TVI_ROOT, GetResString(IDS_X_BANDWIDTH_GENERAL_INFO));
			SetTreeCheck(m_ctrlTreeOptions,m_htiIncludeOverhead,GetResString(IDS_X_INCLUDE_OVERHEAD),m_htiBandwidthControl,GetResString(IDS_X_INCLUDE_OVERHEAD_INFO),FALSE,m_bIncludeOverhead);
			SetTreeCheck(m_ctrlTreeOptions,m_htiIncludeTCPAck,GetResString(IDS_X_INCLUDE_TCP_ACK),m_htiBandwidthControl,GetResString(IDS_X_INCLUDE_TCP_ACK_INFO),FALSE,m_bIncludeTCPAck);
				SetTreeCheck(m_ctrlTreeOptions,m_htiConnectionsOverHead,GetResString(IDS_X_CONNECTIONS_OVERHEAD),m_htiIncludeTCPAck,GetResString(IDS_X_CONNECTIONS_OVERHEAD_INFO),FALSE,m_bConnectionsOverHead);
			SetTreeGroup(m_ctrlTreeOptions,m_htiUploadManagement,GetResString(IDS_X_UPLOAD_MANAGEMENT),iImgUploadManagement, m_htiBandwidthControl, GetResString(IDS_X_UPLOAD_MANAGEMENT_INFO));
				SetTreeCheck(m_ctrlTreeOptions,m_htiMinimizeOpenedSlots,GetResString(IDS_X_MINIMIZE_OPENED_SLOTS),m_htiUploadManagement,GetResString(IDS_X_MINIMIZE_OPENED_SLOTS_INFO),FALSE,m_bMinimizeOpenedSlots);
				SetTreeCheck(m_ctrlTreeOptions,m_htiCumulateBandwidth,GetResString(IDS_X_CUMULATE_BANDWIDTH),m_htiUploadManagement,GetResString(IDS_X_CUMULATE_BANDWIDTH_INFO),TRUE,m_uCumulateBandwidth);
				SetTreeCheck(m_ctrlTreeOptions,m_htiBadwolfsUpload,GetResString(IDS_X_BADWOLFS_UPLOAD),m_htiUploadManagement,GetResString(IDS_X_BADWOLFS_UPLOAD_INFO),TRUE,m_uBadwolfsUpload);
				SetTreeGroup(m_ctrlTreeOptions,m_htiUpload,GetResString(IDS_X_UPLOAD_SLOT_TWEAKS),iImgUpload, m_htiUploadManagement, GetResString(IDS_X_UPLOAD_SLOT_TWEAKS_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxUploadSlots,GetResString(IDS_X_MAX_SLOTS),m_htiUpload,GetResString(IDS_X_MAX_SLOTS_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMinUploadSlots,GetResString(IDS_X_MIN_SLOTS),m_htiUpload,GetResString(IDS_X_MIN_SLOTS_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiUploadPerSlots,GetResString(IDS_X_UL_PER_SLOT),m_htiUpload,GetResString(IDS_X_UL_PER_SLOT_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxReservedSlots,GetResString(IDS_X_UL_RES_SLOT),m_htiUpload,GetResString(IDS_X_UL_RES_SLOT_INFO));
					SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiIncreaseTrickleSpeed,GetResString(IDS_X_INCREASE_TRICKLE_SPEED), m_htiUpload,GetResString(IDS_X_INCREASE_TRICKLE_SPEED_INFO),false, m_bIncreaseTrickleSpeed);
					SetTreeCheck(m_ctrlTreeOptions,m_htiOpenMoreSlotsWhenNeeded,GetResString(IDS_X_OPEN_MORE_SLOTS),m_htiUpload,GetResString(IDS_X_OPEN_MORE_SLOTS_INFO),FALSE,m_bOpenMoreSlotsWhenNeeded);
					SetTreeCheck(m_ctrlTreeOptions,m_htiCheckSlotDatarate,GetResString(IDS_X_CHECK_SLOT_DATARATE_SLOTS),m_htiUpload,GetResString(IDS_X_CHECK_SLOT_DATARATE_SLOTS_INFO),FALSE,m_bCheckSlotDatarate);
				SetTreeCheck(m_ctrlTreeOptions,m_htiTrickleBlocking,GetResString(IDS_X_TRICKLE_BLOCKING),m_htiUploadManagement,GetResString(IDS_X_TRICKLE_BLOCKING_INFO),FALSE,m_bIsTrickleBlocking);
					SetTreeCheck(m_ctrlTreeOptions,m_htiDropBlocking,GetResString(IDS_X_DROP_BLOCKING),m_htiTrickleBlocking,GetResString(IDS_X_DROP_BLOCKING_INFO),FALSE,m_bIsDropBlocking);
			SetTreeCheck(m_ctrlTreeOptions,m_htiSessionRatio,GetResString(IDS_X_ZZ_RATIO),m_htiBandwidthControl,GetResString(IDS_X_ZZ_RATIO_INFO),FALSE,m_bSessionRatio);
			SetTreeGroup(m_ctrlTreeOptions,m_htiUploadSystem,GetResString(IDS_X_UPLOAD_SYSTEM),iImgUploadSystem, m_htiBandwidthControl, GetResString(IDS_X_UPLOAD_SYSTEM_INFO));
				SetTreeCheck(m_ctrlTreeOptions,m_htiUseBlockedQueue,GetResString(IDS_X_USE_BLOCKED_QUEUE),m_htiUploadSystem,GetResString(IDS_X_USE_BLOCKED_QUEUE_INFO),TRUE,m_uUseBlockedQueue);
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiBCTimeUp,GetResString(IDS_X_BC_TIME_UP), m_htiUploadSystem,GetResString(IDS_X_BC_TIME_UP_INFO));
				SetTreePriority(m_ctrlTreeOptions,m_htiBCPriorityUp,GetResString(IDS_X_BC_PRIORITY_UP), m_htiUploadSystem,GetResString(IDS_X_BC_PRIORITY_UP_INFO));
				SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiUploadBufferSize,GetResString(IDS_X_UPLOAD_BUFFER), m_htiUploadSystem,GetResString(IDS_X_UPLOAD_BUFFER_INFO),TRUE, m_uSetUploadBuffer);
			SetTreeGroup(m_ctrlTreeOptions,m_htiDownloadSystem,GetResString(IDS_X_DOWNLOAD_SYSTEM),iImgDownloadSystem, m_htiBandwidthControl, GetResString(IDS_X_DOWNLOAD_SYSTEM_INFO));
				SetTreeCheck(m_ctrlTreeOptions,m_htiUseDownloadBandwidthThrottler,GetResString(IDS_X_USE_DBT),m_htiDownloadSystem,GetResString(IDS_X_USE_DBT_INFO),FALSE,m_bUseDownloadBandwidthThrottler);
				SetTreeCheck(m_ctrlTreeOptions,m_htiUseHyperDownload,GetResString(IDS_X_USE_HYPER_DOWNLOAD),m_htiDownloadSystem,GetResString(IDS_X_USE_HYPER_DOWNLOAD),FALSE,m_uUseHyperDownload);
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiBCTimeDown,GetResString(IDS_X_BC_TIME_DOWN), m_htiDownloadSystem,GetResString(IDS_X_BC_TIME_DOWN_INFO));
				SetTreePriority(m_ctrlTreeOptions,m_htiBCPriorityDown,GetResString(IDS_X_BC_PRIORITY_DOWN), m_htiDownloadSystem,GetResString(IDS_X_BC_PRIORITY_DOWN_INFO));
				SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiDownloadBufferSize,GetResString(IDS_X_DOWNLOAD_BUFFER), m_htiDownloadSystem,GetResString(IDS_X_DOWNLOAD_BUFFER_INFO),TRUE, m_uSetDownloadBuffer);
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiDatarateSamples,GetResString(IDS_X_DATARATE_SAMPLES), m_htiBandwidthControl,GetResString(IDS_X_DATARATE_SAMPLES_INFO));
		SetTreeGroup(m_ctrlTreeOptions,m_htiUploadControl,GetResString(IDS_X_BANDWIDTH_UPLOAD),iImgUploadControl, TVI_ROOT, GetResString(IDS_X_BANDWIDTH_UPLOAD_INFO));
			SetTreeRadio(m_ctrlTreeOptions,m_htiUploadControlNone,GetResString(IDS_X_UPLOAD_CONTROL_NONE),m_htiUploadControl,GetResString(IDS_X_UPLOAD_CONTROL_NONE_INFO),FALSE,m_iUploadControl == 0);
			SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiUploadControlNAFC,GetResString(IDS_X_UPLOAD_CONTROL_NAFC),m_htiUploadControl,GetResString(IDS_X_UPLOAD_CONTROL_NAFC_INFO),FALSE,m_iUploadControl == 1);
			SetTreeRadio(m_ctrlTreeOptions,m_htiUploadControlUSS,GetResString(IDS_X_UPLOAD_CONTROL_USS),m_htiUploadControl,GetResString(IDS_X_UPLOAD_CONTROL_USS_INFO),FALSE,m_iUploadControl == 2);
				SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiBasePingUp,GetResString(IDS_X_BASE_PING_UP), m_htiUploadControlUSS,GetResString(IDS_X_BASE_PING_UP_INFO),FALSE, m_iUpMaxPingMethod == 0);
				SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiPingUpTolerance,GetResString(IDS_X_PING_UP_TOLERANCE), m_htiUploadControlUSS,GetResString(IDS_X_PING_UP_TOLERANCE_INFO),FALSE, m_iUpMaxPingMethod == 1);
				SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiPingUpProzent,GetResString(IDS_X_PING_UP_PROZENT), m_htiUploadControlUSS,GetResString(IDS_X_PING_UP_PROZENT_INFO),FALSE, m_iUpMaxPingMethod == 2);
				SetTreeCheck(m_ctrlTreeOptions,m_htiDynUpGoingDivider,GetResString(IDS_X_DYN_UP_GOING_DIVIDER),m_htiUploadControlUSS,GetResString(IDS_X_DYN_UP_GOING_DIVIDER_INFO),FALSE,m_bDynUpGoingDivider);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiDynUpGoingUpDivider,GetResString(IDS_X_DYN_UP_GOING_UP_DIVIDER), m_htiDynUpGoingDivider,GetResString(IDS_X_DYN_UP_GOING_UP_DIVIDER_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiDynUpGoingDownDivider,GetResString(IDS_X_DYN_UP_GOING_DOWN_DIVIDER), m_htiDynUpGoingDivider,GetResString(IDS_X_DYN_UP_GOING_DOWN_DIVIDER_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMinBCUpload,GetResString(IDS_X_MIN_BC_UPLOAD), m_htiUploadControl,GetResString(IDS_X_MIN_BC_UPLOAD_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxBCUpload,GetResString(IDS_X_MAX_BC_UPLOAD), m_htiUploadControl,GetResString(IDS_X_MAX_BC_UPLOAD_INFO));
		SetTreeGroup(m_ctrlTreeOptions,m_htiDownloadControl,GetResString(IDS_X_BANDWIDTH_DOWNLOAD),iImgDownloadControl, TVI_ROOT, GetResString(IDS_X_BANDWIDTH_DOWNLOAD_INFO));
			SetTreeRadio(m_ctrlTreeOptions,m_htiDownloadControlNone,GetResString(IDS_X_DOWNLOAD_CONTROL_NONE),m_htiDownloadControl,GetResString(IDS_X_DOWNLOAD_CONTROL_NONE_INFO),FALSE,m_iDownloadControl == 0);
			SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiDownloadControlNAFC,GetResString(IDS_X_DOWNLOAD_CONTROL_NAFC),m_htiDownloadControl,GetResString(IDS_X_DOWNLOAD_CONTROL_NAFC_INFO),FALSE,m_iDownloadControl == 1);
			SetTreeRadio(m_ctrlTreeOptions,m_htiDownloadControlDSS,GetResString(IDS_X_DOWNLOAD_CONTROL_DSS),m_htiDownloadControl,GetResString(IDS_X_DOWNLOAD_CONTROL_DSS_INFO),FALSE,m_iDownloadControl == 2);
				SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiBasePingDown,GetResString(IDS_X_BASE_PING_DOWN), m_htiDownloadControlDSS,GetResString(IDS_X_BASE_PING_DOWN_INFO),FALSE, m_iDownMaxPingMethod == 0);
				SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiPingDownTolerance,GetResString(IDS_X_PING_DOWN_TOLERANCE), m_htiDownloadControlDSS,GetResString(IDS_X_PING_DOWN_TOLERANCE_INFO),FALSE, m_iDownMaxPingMethod == 1);
				SetTreeRadioNumEdit(m_ctrlTreeOptions,m_htiPingDownProzent,GetResString(IDS_X_PING_DOWN_PROZENT), m_htiDownloadControlDSS,GetResString(IDS_X_PING_DOWN_PROZENT_INFO),FALSE, m_iDownMaxPingMethod == 2);
				SetTreeCheck(m_ctrlTreeOptions,m_htiDynDownGoingDivider,GetResString(IDS_X_DYN_DOWN_GOING_DIVIDER),m_htiDownloadControlDSS,GetResString(IDS_X_DYN_DOWN_GOING_DIVIDER_INFO),FALSE,m_bDynDownGoingDivider);
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiDynDownGoingUpDivider,GetResString(IDS_X_DYN_DOWN_GOING_UP_DIVIDER), m_htiDynDownGoingDivider,GetResString(IDS_X_DYN_DOWN_GOING_UP_DIVIDER_INFO));
					SetTreeNumEdit(m_ctrlTreeOptions,m_htiDynDownGoingDownDivider,GetResString(IDS_X_DYN_DOWN_GOING_DOWN_DIVIDER), m_htiDynDownGoingDivider,GetResString(IDS_X_DYN_DOWN_GOING_DOWN_DIVIDER_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMinBCDownload,GetResString(IDS_X_MIN_BC_DOWNLOAD), m_htiDownloadControl,GetResString(IDS_X_MIN_BC_DOWNLOAD_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxBCDownload,GetResString(IDS_X_MAX_BC_DOWNLOAD), m_htiDownloadControl,GetResString(IDS_X_MAX_BC_DOWNLOAD_INFO));


		m_ctrlTreeOptions.Expand(m_htiIncludeTCPAck, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiBandwidthControl, TVE_EXPAND);

		UINT bCheck;

		m_ctrlTreeOptions.GetCheckBox(m_htiIncludeTCPAck, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiConnectionsOverHead, bCheck);

		m_ctrlTreeOptions.GetCheckBox(m_htiUseDownloadBandwidthThrottler, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiUseHyperDownload, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiBCTimeDown, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiBCPriorityDown, bCheck);

		m_ctrlTreeOptions.GetCheckBox(m_htiDownloadBufferSize, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiDownloadBufferSize, bCheck /*== TRUE*/, TRUE, TRUE);

		m_ctrlTreeOptions.GetCheckBox(m_htiUploadBufferSize, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiUploadBufferSize, bCheck /*== TRUE*/, TRUE, TRUE);

		CheckEnableUSS();
		CheckEnableDSS();

		m_bInitializedTreeOpts = true;
	}

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIncludeOverhead, m_bIncludeOverhead);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIncludeTCPAck, m_bIncludeTCPAck);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiConnectionsOverHead, m_bConnectionsOverHead);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMinimizeOpenedSlots, m_bMinimizeOpenedSlots);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCumulateBandwidth, m_uCumulateBandwidth);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiBadwolfsUpload, m_uBadwolfsUpload);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxUploadSlots, m_iMaxUploadSlots);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMinUploadSlots, m_iMinUploadSlots);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiUploadPerSlots, m_fUploadPerSlots);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxReservedSlots, m_iMaxReservedSlots);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIncreaseTrickleSpeed, m_bIncreaseTrickleSpeed);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiIncreaseTrickleSpeed, m_fIncreaseTrickleSpeed);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiOpenMoreSlotsWhenNeeded, m_bOpenMoreSlotsWhenNeeded);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckSlotDatarate, m_bCheckSlotDatarate);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiTrickleBlocking, m_bIsTrickleBlocking);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDropBlocking, m_bIsDropBlocking);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSessionRatio, m_bSessionRatio);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseBlockedQueue, m_uUseBlockedQueue);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiBCTimeUp, m_iBCTimeUp);
			DDX_PriorityT(&m_ctrlTreeOptions, pDX, IDC_MOD_OPTS, m_htiBCPriorityUp, m_iBCPriorityUp);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUploadBufferSize, m_uSetUploadBuffer);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiUploadBufferSize, m_iUploadBufferSize);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseDownloadBandwidthThrottler, m_bUseDownloadBandwidthThrottler);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseHyperDownload, m_uUseHyperDownload);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiBCTimeDown, m_iBCTimeDown);
			DDX_PriorityT(&m_ctrlTreeOptions, pDX, IDC_MOD_OPTS, m_htiBCPriorityDown, m_iBCPriorityDown);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDownloadBufferSize, m_uSetDownloadBuffer);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDownloadBufferSize, m_iDownloadBufferSize);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDatarateSamples, m_iDatarateSamples);
		DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiUploadControl, m_iUploadControl);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiUploadControlNAFC, m_fMaxUpStream);
				DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiUploadControlUSS, m_iUpMaxPingMethod);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiBasePingUp, m_iBasePingUp);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPingUpTolerance, m_iPingUpTolerance);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPingUpProzent, m_iPingUpProzent);
				DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDynUpGoingDivider, m_bDynUpGoingDivider);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDynUpGoingUpDivider, m_iDynUpGoingUpDivider);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDynUpGoingDownDivider, m_iDynUpGoingDownDivider);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMinBCUpload, m_fMinBCUpload);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxBCUpload, m_fMaxBCUpload);
		DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiDownloadControl, m_iDownloadControl);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDownloadControlNAFC, m_fMaxDownStream);
				DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiDownloadControlDSS, m_iDownMaxPingMethod);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiBasePingDown, m_iBasePingDown);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPingDownTolerance, m_iPingDownTolerance);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiPingDownProzent, m_iPingDownProzent);
				DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiDynDownGoingDivider, m_bDynDownGoingDivider);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDynDownGoingUpDivider, m_iDynDownGoingUpDivider);
					DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiDynDownGoingDownDivider, m_iDynDownGoingDownDivider);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMinBCDownload, m_fMinBCDownload);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxBCDownload, m_fMaxBCDownload);
}

BOOL CPPgBandwidth::OnInitDialog()
{
	LoadSettings();

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgBandwidth::LoadSettings()
{
	/*
	* Globale Einstellungen Laden
	*/

	m_bUseDownloadBandwidthThrottler = NeoPrefs.m_bUseDownloadBandwidthThrottler;
	m_uUseHyperDownload = NeoPrefs.m_uUseHyperDownload;
	m_iBCTimeDown = NeoPrefs.m_iBCTimeDown;
	m_iBCPriorityDown = NeoPrefs.m_iBCPriorityDown;
	m_uSetDownloadBuffer = NeoPrefs.m_uSetDownloadBuffer;
	m_iDownloadBufferSize = NeoPrefs.m_iDownloadBufferSize;

	m_uUseBlockedQueue = NeoPrefs.m_uUseBlockedQueue;
	m_iBCTimeUp = NeoPrefs.m_iBCTimeUp;
	m_iBCPriorityUp = NeoPrefs.m_iBCPriorityUp;
	m_uSetUploadBuffer = NeoPrefs.m_uSetUploadBuffer;
	m_iUploadBufferSize = NeoPrefs.m_iUploadBufferSize;

	m_iDatarateSamples = NeoPrefs.m_iDatarateSamples;

	m_bIncludeOverhead = NeoPrefs.m_bIncludeOverhead;
	m_bIncludeTCPAck = NeoPrefs.m_bIncludeTCPAck;
	m_bConnectionsOverHead = NeoPrefs.m_bConnectionsOverHead;
	m_bSessionRatio = NeoPrefs.m_bSessionRatio;


	m_iDownloadControl = NeoPrefs.m_iDownloadControl;
	m_fMinBCDownload = NeoPrefs.m_fMinBCDownload;
	m_fMaxBCDownload = NeoPrefs.m_fMaxBCDownload;


	m_iUploadControl = NeoPrefs.m_iUploadControl;
	m_fMinBCUpload = NeoPrefs.m_fMinBCUpload;
	m_fMaxBCUpload = NeoPrefs.m_fMaxBCUpload;


	m_fMaxDownStream = NeoPrefs.m_fMaxDownStream;
	m_fMaxUpStream = NeoPrefs.m_fMaxUpStream;


	m_bMinimizeOpenedSlots = NeoPrefs.m_bMinimizeOpenedSlots;
	m_uCumulateBandwidth = NeoPrefs.m_uCumulateBandwidth;
	m_uBadwolfsUpload = NeoPrefs.m_uBadwolfsUpload;


	m_bDynUpGoingDivider = NeoPrefs.m_bDynUpGoingDivider;
	m_iDynUpGoingUpDivider = NeoPrefs.m_iDynUpGoingUpDivider;
	m_iDynUpGoingDownDivider = NeoPrefs.m_iDynUpGoingDownDivider;

	m_iUpMaxPingMethod = NeoPrefs.m_iUpMaxPingMethod;
	m_iBasePingUp = NeoPrefs.m_iBasePingUp;
	m_iPingUpTolerance = NeoPrefs.m_iPingUpTolerance;
	m_iPingUpProzent = NeoPrefs.m_iPingUpProzent;


	m_bDynDownGoingDivider = NeoPrefs.m_bDynDownGoingDivider;
	m_iDynDownGoingUpDivider = NeoPrefs.m_iDynDownGoingUpDivider;
	m_iDynDownGoingDownDivider = NeoPrefs.m_iDynDownGoingDownDivider;
	
	m_iDownMaxPingMethod = NeoPrefs.m_iDownMaxPingMethod;
	m_iBasePingDown = NeoPrefs.m_iBasePingDown;
	m_iPingDownTolerance = NeoPrefs.m_iPingDownTolerance;
	m_iPingDownProzent = NeoPrefs.m_iPingDownProzent;


	m_iMaxUploadSlots = NeoPrefs.m_iMaxUploadSlots;
	m_iMinUploadSlots = NeoPrefs.m_iMinUploadSlots;
	m_fUploadPerSlots = NeoPrefs.m_fUploadPerSlots;
	m_iMaxReservedSlots = NeoPrefs.m_iMaxReservedSlots;

	m_bIncreaseTrickleSpeed = NeoPrefs.m_bIncreaseTrickleSpeed;
	m_fIncreaseTrickleSpeed = NeoPrefs.m_fIncreaseTrickleSpeed;

	m_bOpenMoreSlotsWhenNeeded = NeoPrefs.m_bOpenMoreSlotsWhenNeeded;
	m_bCheckSlotDatarate = NeoPrefs.m_bCheckSlotDatarate;

	m_bIsTrickleBlocking = NeoPrefs.m_bIsTrickleBlocking;
	m_bIsDropBlocking = NeoPrefs.m_bIsDropBlocking;
}

BOOL CPPgBandwidth::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	/*
	* Globale Einstellungen Speichern
	*/

	bool bResizeStatusbBar = false;

	NeoPrefs.m_bUseDownloadBandwidthThrottler = m_bUseDownloadBandwidthThrottler;
	if(NeoPrefs.m_bUseDownloadBandwidthThrottler == false)
		theApp.downloadBandwidthThrottler->ClearQueues();
	NeoPrefs.m_uUseHyperDownload = m_uUseHyperDownload;
	NeoPrefs.m_iBCTimeDown = m_iBCTimeDown;
	if(NeoPrefs.m_iBCPriorityDown != m_iBCPriorityDown)
		theApp.downloadBandwidthThrottler->pThread->SetThreadPriority(m_iBCPriorityDown);
	NeoPrefs.m_iBCPriorityDown = m_iBCPriorityDown;
	NeoPrefs.m_uSetDownloadBuffer = m_uSetDownloadBuffer;
	NeoPrefs.m_iDownloadBufferSize = m_iDownloadBufferSize;

	NeoPrefs.m_uUseBlockedQueue = m_uUseBlockedQueue;
	NeoPrefs.m_iBCTimeUp = m_iBCTimeUp;
	if(NeoPrefs.m_iBCPriorityUp != m_iBCPriorityUp)
		theApp.uploadBandwidthThrottler->pThread->SetThreadPriority(m_iBCPriorityUp);
	NeoPrefs.m_iBCPriorityUp = m_iBCPriorityUp;
	NeoPrefs.m_uSetUploadBuffer = m_uSetUploadBuffer;
	NeoPrefs.m_iUploadBufferSize = m_iUploadBufferSize;

	NeoPrefs.m_iDatarateSamples = m_iDatarateSamples;

	NeoPrefs.m_bIncludeOverhead = m_bIncludeOverhead;
	NeoPrefs.m_bIncludeTCPAck = m_bIncludeTCPAck;
	NeoPrefs.m_bConnectionsOverHead = m_bConnectionsOverHead;
	NeoPrefs.m_bSessionRatio = m_bSessionRatio;


	bResizeStatusbBar = ((NeoPrefs.m_iDownloadControl == 2) != (m_iDownloadControl == 2)) 
					 || ((NeoPrefs.m_iUploadControl == 2) != (m_iUploadControl == 2));

	NeoPrefs.m_iDownloadControl = m_iDownloadControl;
	NeoPrefs.m_fMinBCDownload = m_fMinBCDownload;
	NeoPrefs.m_fMaxBCDownload = m_fMaxBCDownload;


	NeoPrefs.m_iUploadControl = m_iUploadControl;
	NeoPrefs.m_fMinBCUpload = m_fMinBCUpload;
	NeoPrefs.m_fMaxBCUpload = m_fMaxBCUpload;


	NeoPrefs.m_fMaxDownStream = m_fMaxDownStream;
	NeoPrefs.m_fMaxUpStream = m_fMaxUpStream;


	NeoPrefs.m_bMinimizeOpenedSlots = m_bMinimizeOpenedSlots;
	NeoPrefs.m_uCumulateBandwidth = m_uCumulateBandwidth;
	NeoPrefs.m_uBadwolfsUpload = m_uBadwolfsUpload;


	NeoPrefs.m_bDynUpGoingDivider = m_bDynUpGoingDivider;
	NeoPrefs.m_iDynUpGoingUpDivider = m_iDynUpGoingUpDivider;
	NeoPrefs.m_iDynUpGoingDownDivider = m_iDynUpGoingDownDivider;

	NeoPrefs.m_iUpMaxPingMethod = m_iUpMaxPingMethod;
	NeoPrefs.m_iBasePingUp = m_iBasePingUp;
	NeoPrefs.m_iPingUpTolerance = m_iPingUpTolerance;
	NeoPrefs.m_iPingUpProzent = m_iPingUpProzent;


	NeoPrefs.m_bDynDownGoingDivider = m_bDynDownGoingDivider;
	NeoPrefs.m_iDynDownGoingUpDivider = m_iDynDownGoingUpDivider;
	NeoPrefs.m_iDynDownGoingDownDivider = m_iDynDownGoingDownDivider;
	
	NeoPrefs.m_iDownMaxPingMethod = m_iDownMaxPingMethod;
	NeoPrefs.m_iBasePingDown = m_iBasePingDown;
	NeoPrefs.m_iPingDownTolerance = m_iPingDownTolerance;
	NeoPrefs.m_iPingDownProzent = m_iPingDownProzent;


	NeoPrefs.m_iMaxUploadSlots = m_iMaxUploadSlots;
	NeoPrefs.m_iMinUploadSlots = m_iMinUploadSlots;
	NeoPrefs.m_fUploadPerSlots = m_fUploadPerSlots;
	NeoPrefs.m_iMaxReservedSlots = m_iMaxReservedSlots;

	NeoPrefs.m_bIncreaseTrickleSpeed = m_bIncreaseTrickleSpeed;
	NeoPrefs.m_fIncreaseTrickleSpeed = m_fIncreaseTrickleSpeed;

	NeoPrefs.m_bOpenMoreSlotsWhenNeeded = m_bOpenMoreSlotsWhenNeeded;
	NeoPrefs.m_bCheckSlotDatarate = m_bCheckSlotDatarate;

	NeoPrefs.m_bIsTrickleBlocking = m_bIsTrickleBlocking;
	NeoPrefs.m_bIsDropBlocking = m_bIsDropBlocking;	

	NeoPrefs.CheckNeoPreferences();
	LoadSettings();

	if(bResizeStatusbBar)
		theApp.emuledlg->SetStatusBarPartsSize();

	theApp.scheduler->SaveOriginals();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgBandwidth::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgBandwidth::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

LRESULT CPPgBandwidth::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){

			if(m_htiMaxUploadSlots && pton->hItem == m_htiMaxUploadSlots){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxUploadSlots, VAL_MAX_UL_SLOTS_MIN, VAL_MAX_UL_SLOTS_DEF, VAL_MAX_UL_SLOTS_MAX)) SetModified();
			}else if(m_htiMinUploadSlots && pton->hItem == m_htiMinUploadSlots){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMinUploadSlots, VAL_MIN_UL_SLOTS_MIN, VAL_MIN_UL_SLOTS_DEF, VAL_MIN_UL_SLOTS_MAX)) SetModified();
			}else if(m_htiUploadPerSlots && pton->hItem == m_htiUploadPerSlots){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUploadPerSlots, VAL_UL_PER_SLOTS_MIN, VAL_UL_PER_SLOTS_DEF, VAL_UL_PER_SLOTS_MAX)) SetModified();
			}else if(m_htiMaxReservedSlots && pton->hItem == m_htiMaxReservedSlots){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxReservedSlots, VAL_MAX_RESERVED_SLOTS_MIN, VAL_MAX_RESERVED_SLOTS_DEF, VAL_MAX_RESERVED_SLOTS_MAX)) SetModified();
			}
			else if(m_htiBCTimeDown && pton->hItem == m_htiBCTimeDown){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBCTimeDown, VAL_BC_TIME_DOWN_MIN, VAL_BC_TIME_DOWN_DEF, VAL_BC_TIME_DOWN_MAX)) SetModified();
			}else if(m_htiBCTimeUp && pton->hItem == m_htiBCTimeUp){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBCTimeUp, VAL_BC_TIME_UP_MIN, VAL_BC_TIME_UP_DEF, VAL_BC_TIME_UP_MAX)) SetModified();
			}
			else if(m_htiDownloadBufferSize && pton->hItem == m_htiDownloadBufferSize){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDownloadBufferSize, VAL_DOWNLOAD_BUFFER_MIN, VAL_DOWNLOAD_BUFFER_DEF, VAL_DOWNLOAD_BUFFER_MAX)) SetModified();
			}else if(m_htiUploadBufferSize && pton->hItem == m_htiUploadBufferSize){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiUploadBufferSize, VAL_UPLOAD_BUFFER_MIN, VAL_UPLOAD_BUFFER_DEF, VAL_UPLOAD_BUFFER_MAX)) SetModified();
			}
			else if(m_htiDatarateSamples && pton->hItem == m_htiDatarateSamples){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDatarateSamples, 5, 10, 20)) SetModified();
			}
			else if(m_htiDynUpGoingUpDivider && pton->hItem == m_htiDynUpGoingUpDivider){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDynUpGoingUpDivider, VAL_DYN_UP_GOING_UP_DIVIDER_MIN, VAL_DYN_UP_GOING_UP_DIVIDER_DEF, VAL_DYN_UP_GOING_UP_DIVIDER_MAX)) SetModified();
			}else if(m_htiDynUpGoingDownDivider && pton->hItem == m_htiDynUpGoingDownDivider){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDynUpGoingDownDivider, VAL_DYN_UP_GOING_DOWN_DIVIDER_MIN, VAL_DYN_UP_GOING_DOWN_DIVIDER_DEF, VAL_DYN_UP_GOING_DOWN_DIVIDER_MAX)) SetModified();
			}
			else if(m_htiBasePingUp && pton->hItem == m_htiBasePingUp){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBasePingUp, VAL_BASE_PING_UP_MIN, VAL_BASE_PING_UP_DEF, VAL_BASE_PING_UP_MAX)) SetModified();
			}else if(m_htiPingUpTolerance && pton->hItem == m_htiPingUpTolerance){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiPingUpTolerance, VAL_PING_UP_TOLERANCE_MIN, VAL_PING_UP_TOLERANCE_DEF, VAL_PING_UP_TOLERANCE_MAX)) SetModified();
			}else if(m_htiPingUpProzent && pton->hItem == m_htiPingUpProzent){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiPingUpProzent, VAL_PING_UP_PROZENT_MIN, VAL_PING_UP_PROZENT_DEF, VAL_PING_UP_PROZENT_MAX)) SetModified();
			}
			else if(m_htiDynDownGoingUpDivider && pton->hItem == m_htiDynDownGoingUpDivider){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDynDownGoingUpDivider, VAL_DYN_DOWN_GOING_UP_DIVIDER_MIN, VAL_DYN_DOWN_GOING_UP_DIVIDER_DEF, VAL_DYN_DOWN_GOING_UP_DIVIDER_MAX)) SetModified();
			}else if(m_htiDynDownGoingDownDivider && pton->hItem == m_htiDynDownGoingDownDivider){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiDynDownGoingDownDivider, VAL_DYN_DOWN_GOING_DOWN_DIVIDER_MIN, VAL_DYN_DOWN_GOING_DOWN_DIVIDER_DEF, VAL_DYN_DOWN_GOING_DOWN_DIVIDER_MAX)) SetModified();
			}
			else if(m_htiBasePingDown && pton->hItem == m_htiBasePingDown){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiBasePingDown, VAL_BASE_PING_DOWN_MIN, VAL_BASE_PING_DOWN_DEF, VAL_BASE_PING_DOWN_MAX)) SetModified();
			}else if(m_htiPingDownTolerance && pton->hItem == m_htiPingDownTolerance){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiPingDownTolerance, VAL_PING_DOWN_TOLERANCE_MIN, VAL_PING_DOWN_TOLERANCE_DEF, VAL_PING_DOWN_TOLERANCE_MAX)) SetModified();
			}else if(m_htiPingDownProzent && pton->hItem == m_htiPingDownProzent){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiPingDownProzent, VAL_PING_DOWN_PROZENT_MIN, VAL_PING_DOWN_PROZENT_DEF, VAL_PING_DOWN_PROZENT_MAX)) SetModified();
			}

		}else{
			UINT bCheck;
			if (m_htiIncludeTCPAck && pton->hItem == m_htiIncludeTCPAck){
				m_ctrlTreeOptions.GetCheckBox(m_htiIncludeTCPAck, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiConnectionsOverHead, bCheck);
			}

			else if (m_htiUseDownloadBandwidthThrottler && pton->hItem == m_htiUseDownloadBandwidthThrottler){
				m_ctrlTreeOptions.GetCheckBox(m_htiUseDownloadBandwidthThrottler, bCheck);

				m_ctrlTreeOptions.SetItemEnable(m_htiUseHyperDownload, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiBCTimeDown, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiBCPriorityDown, bCheck);
			}

			else if(m_htiDownloadBufferSize && pton->hItem == m_htiDownloadBufferSize){
				m_ctrlTreeOptions.GetCheckBox(m_htiDownloadBufferSize, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiDownloadBufferSize, bCheck /*== TRUE*/, TRUE, TRUE);
			}

			else if(m_htiUploadBufferSize && pton->hItem == m_htiUploadBufferSize){
				m_ctrlTreeOptions.GetCheckBox(m_htiUploadBufferSize, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiUploadBufferSize, bCheck /*== TRUE*/, TRUE, TRUE);
			}

			else if (m_htiUploadControlUSS && (
			    pton->hItem == m_htiUploadControlNone
			 || pton->hItem == m_htiUploadControlNAFC
			 || pton->hItem == m_htiUploadControlUSS
			 || pton->hItem == m_htiDynUpGoingDivider
			))
				CheckEnableUSS();
			else if (m_htiDownloadControlDSS && (
			    pton->hItem == m_htiDownloadControlNone
			 || pton->hItem == m_htiDownloadControlNAFC
			 || pton->hItem == m_htiDownloadControlDSS
			 || pton->hItem == m_htiDynDownGoingDivider
			))
				CheckEnableDSS();
			SetModified();
		}
	}
	return 0;
}

void CPPgBandwidth::CheckEnableUSS(){
	UINT bCheck;
	int iCheck;

	m_ctrlTreeOptions.GetRadioButton(m_htiUploadControlUSS, iCheck);
	m_ctrlTreeOptions.SetGroupEnable(m_htiUploadControlUSS, iCheck);

	if(iCheck){
		m_ctrlTreeOptions.GetCheckBox(m_htiDynUpGoingDivider, bCheck);
		m_ctrlTreeOptions.SetGroupEnable(m_htiDynUpGoingDivider, bCheck);
	}

	m_ctrlTreeOptions.GetRadioButton(m_htiUploadControlNAFC, iCheck);
	m_ctrlTreeOptions.SetItemEnable(m_htiUploadControlNAFC, iCheck,TRUE,TRUE);

	m_ctrlTreeOptions.GetRadioButton(m_htiUploadControlNone, iCheck);
	m_ctrlTreeOptions.SetItemEnable(m_htiMinBCUpload, !iCheck);
	m_ctrlTreeOptions.SetItemEnable(m_htiMaxBCUpload, !iCheck);
}

void CPPgBandwidth::CheckEnableDSS(){
	UINT bCheck;
	int iCheck;

	m_ctrlTreeOptions.GetRadioButton(m_htiDownloadControlDSS, iCheck);
	m_ctrlTreeOptions.SetGroupEnable(m_htiDownloadControlDSS, iCheck);

	if(iCheck){
		m_ctrlTreeOptions.GetCheckBox(m_htiDynDownGoingDivider, bCheck);
		m_ctrlTreeOptions.SetGroupEnable(m_htiDynDownGoingDivider, bCheck);
	}

	m_ctrlTreeOptions.GetRadioButton(m_htiDownloadControlNAFC, iCheck);
	m_ctrlTreeOptions.SetItemEnable(m_htiDownloadControlNAFC, iCheck,TRUE,TRUE);

	m_ctrlTreeOptions.GetRadioButton(m_htiDownloadControlNone, iCheck);
	m_ctrlTreeOptions.SetItemEnable(m_htiMinBCDownload, !iCheck);
	m_ctrlTreeOptions.SetItemEnable(m_htiMaxBCDownload, !iCheck);
}

LRESULT CPPgBandwidth::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgBandwidth::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgBandwidth::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgBandwidth::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

#endif // NEO_BC // NEO: NBC END <-- Xanatos --