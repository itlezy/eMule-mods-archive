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
#include "PPgLancast.h"
#include "Neo/NeoPreferences.h"
#include "Partfile.h"
#include "downloadqueue.h"
#include "knownfilelist.h"
#include "Neo/NeoPreferences.h"
#include "OtherFunctions.h"
#include "KnownFileList.h"
#include "Neo\Defaults.h"
#include "Neo\GUI\CP\TreeFunctions.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#include "Neo/LanCast/Lancast.h"
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#include "eMuleDlg.h"
#include "SearchDlg.h"
#include "SearchParamsWnd.h"
#include "SearchResultsWnd.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->


///////////////////////////////////////////////////////////////////////////////
// CPPgLancast dialog

IMPLEMENT_DYNAMIC(CPPgLancast, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgLancast, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_MESSAGE(WM_TREEITEM_HELP, DrawTreeItemHelp)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgLancast::CPPgLancast()
	: CPropertyPage(CPPgLancast::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	// NEO: FCFG - [FileConfiguration]
	m_paFiles = NULL;
	m_Category = NULL;
	m_bDataChanged = false;
	//m_strCaption = GetResString(IDS_FILEINFORMATION);
	//m_psp.pszTitle = m_strCaption;
	//m_psp.dwFlags |= PSP_USETITLE;
	m_timer = 0;
	// NEO: FCFG END
	ClearAllMembers();
	m_ctrlTreeOptions.SetNeoStyle();
}

CPPgLancast::~CPPgLancast()
{
}

void CPPgLancast::ClearAllMembers()
{
	m_bInitializedTreeOpts = false;

	m_htiLancastUpload = NULL;
		m_htiMaxLanUpload = NULL;
		m_htiLanUploadSlots = NULL;
			m_htiMaxLanUploadSlots = NULL;
		m_htiLanUploadBufferSize = NULL;
	m_htiLancastDownload = NULL;
		m_htiMaxLanDownload = NULL;
		m_htiLanDownloadBufferSize = NULL;
	m_htiCustomLanCastAdapter = NULL;
		m_htiCustomLanCastMask = NULL;
	m_htiLancast = NULL;
		m_htiLancastDisable = NULL;
		m_htiLancastEnable = NULL;
		m_htiLancastDefault = NULL;
		m_htiLancastGlobal = NULL;
		m_htiLancastEnabled = NULL;
		m_htiCustomizedLanCast = NULL;
			m_htiLanCastPort = NULL;
		m_htiLANIntervals = NULL;
		m_htiLanCastReask = NULL;
			m_htiLanReaskIntervals = NULL;
			m_htiNnPLanReaskIntervals = NULL;
		m_htiAutoBroadcastLanFiles = NULL;
		m_htiUseLanMultiTransfer = NULL;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_htiVoodoo = NULL;
		m_htiVoodooTransferDisable = NULL;
		m_htiVoodooTransferEnable = NULL;
		m_htiVoodooTransferDefault = NULL;
		m_htiVoodooTransferGlobal = NULL;
		m_htiUseVoodooTransfer = NULL;
		m_htiSlaveAllowed = NULL;
		m_htiSlaveHosting = NULL;
		m_htiAutoConnectVoodoo = NULL;
		m_htiUseVirtualVoodooFiles = NULL;
		m_htiVoodooSourceExchange = NULL;
			m_htiVoodooSourceExchangeDisable = NULL;
			m_htiVoodooSourceExchangeEnable1 = NULL;
			m_htiVoodooSourceExchangeEnable2 = NULL;
			m_htiVoodooSourceExchangeDefault = NULL;
			m_htiVoodooSourceExchangeGlobal = NULL;
		m_htiVoodooSpell = NULL;
		m_htiVoodooPort = NULL;
		m_htiVoodooCastEnabled = NULL;
			m_htiSearchForSlaves = NULL;
			m_htiSearchForMaster = NULL;

		m_htiHideVoodooFiles = NULL; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END
}

void CPPgLancast::SetTreeRadioEx(HTREEITEM &htiDisable, HTREEITEM &htiEnable, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),Value,0),htiParent,GetResString(IDS_X_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable,MkRdBtnLbl(GetResString(IDS_X_ENABLE),Value,1),htiParent,GetResString(IDS_X_ENABLE_INFO),TRUE,Value.Val == 1);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
void CPPgLancast::SetTreeRadioXs(HTREEITEM &htiDisable, HTREEITEM &htiEnable1, HTREEITEM &htiEnable2, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value)
{
	SetTreeRadio(m_ctrlTreeOptions,htiDisable,MkRdBtnLbl(GetResString(IDS_X_DISABLE),Value,0),htiParent,GetResString(IDS_X_DISABLE_INFO),TRUE,Value.Val == 0);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable1,MkRdBtnLbl(GetResString(IDS_X_VOODOO_XS_1),Value,1),htiParent,GetResString(IDS_X_VOODOO_XS_1_INFO),TRUE,Value.Val == 1);
	SetTreeRadio(m_ctrlTreeOptions,htiEnable2,MkRdBtnLbl(GetResString(IDS_X_VOODOO_XS_2),Value,2),htiParent,GetResString(IDS_X_VOODOO_XS_2_INFO),TRUE,Value.Val == 2);
	if(m_paFiles || m_Category)
		SetTreeRadio(m_ctrlTreeOptions,htiDefault,GetResString(IDS_X_DEFAULT),htiParent,GetResString(IDS_X_DEFAULT_INFO),TRUE,Value.Val == FCFG_DEF);
	if(m_paFiles)
		SetTreeRadio(m_ctrlTreeOptions,htiGlobal,GetResString(IDS_X_GLOBAL),htiParent,GetResString(IDS_X_GLOBAL_INFO),TRUE,Value.Val == FCFG_GLB);
}
#endif // VOODOO // NEO: VOODOO END

void CPPgLancast::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int	iImgLanSupport = 8;
		int iImgUploadControl = 8;
		int iImgUpload = 8;
		int iImgDownloadControl = 8;
		int iImgLancast = 8;
		int iImgReaskLan = 8;
		int iImgVoodoo = 8;
		int iImgVoodooXs = 8;

        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgLanSupport = piml->Add(CTempIconLoader(_T("LANSUPPORT")));
			iImgUploadControl = piml->Add(CTempIconLoader(_T("UPLOADCONTROL")));
			iImgUpload = piml->Add(CTempIconLoader(_T("UPLOADTWEAKS")));
			iImgDownloadControl = piml->Add(CTempIconLoader(_T("DOWNLOADCONTROL")));
			iImgLancast = piml->Add(CTempIconLoader(_T("LANCAST")));
			iImgReaskLan = piml->Add(CTempIconLoader(_T("REASKLAN")));
			iImgVoodoo = piml->Add(CTempIconLoader(_T("VOODOO")));
			iImgVoodooXs = piml->Add(CTempIconLoader(_T("VOODOOXS")));
		}

	if(m_Category == NULL && m_paFiles == NULL){
		SetTreeGroup(m_ctrlTreeOptions,m_htiLancastUpload,GetResString(IDS_X_LAN_UPLOAD),iImgUploadControl, TVI_ROOT, GetResString(IDS_X_LAN_UPLOAD_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxLanUpload,GetResString(IDS_X_MAX_LAN_UPLOAD), m_htiLancastUpload,GetResString(IDS_X_MAX_LAN_UPLOAD_INFO));
			SetTreeGroup(m_ctrlTreeOptions,m_htiLanUploadSlots,GetResString(IDS_X_LAN_UPLOAD_SLOTS),iImgUpload,m_htiLancastUpload, GetResString(IDS_X_LAN_UPLOAD_SLOTS_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxLanUploadSlots,GetResString(IDS_X_MAX_LAN_SLOTS), m_htiLanUploadSlots,GetResString(IDS_X_MAX_LAN_SLOTS_INFO));
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiLanUploadBufferSize,GetResString(IDS_X_LAN_UPLOAD_BUFFER), m_htiLancastUpload,GetResString(IDS_X_LAN_UPLOAD_BUFFER_INFO),FALSE, m_bSetLanUploadBuffer);
		SetTreeGroup(m_ctrlTreeOptions,m_htiLancastDownload,GetResString(IDS_X_LAN_DOWNLOAD),iImgDownloadControl, TVI_ROOT, GetResString(IDS_X_LAN_DOWNLOAD_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiMaxLanDownload,GetResString(IDS_X_MAX_LAN_DOWNLOAD), m_htiLancastDownload,GetResString(IDS_X_MAX_LAN_DOWNLOAD_INFO));
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiLanDownloadBufferSize,GetResString(IDS_X_LAN_DOWNLOAD_BUFFER), m_htiLancastDownload,GetResString(IDS_X_LAN_DOWNLOAD_BUFFER_INFO),FALSE, m_bSetLanDownloadBuffer);
		SetTreeAdapterIndex(m_ctrlTreeOptions,m_htiCustomLanCastAdapter,GetResString(IDS_X_LAN_ADAPTER),TVI_ROOT,GetResString(IDS_X_LAN_ADAPTER_INFO),FALSE,TRUE,FALSE, m_bCustomLanCastAdapter);
			SetTreeAdapterIndex(m_ctrlTreeOptions,m_htiCustomLanCastMask,GetResString(IDS_X_LAN_MASK),m_htiCustomLanCastAdapter,GetResString(IDS_X_LAN_MASK_INFO),TRUE);
	}
		SetTreeGroup(m_ctrlTreeOptions,m_htiLancast,GetResString(IDS_X_LANCAST_MAIN),iImgLancast, TVI_ROOT, GetResString(IDS_X_LANCAST_MAIN_INFO));
		if(m_Category == NULL && m_paFiles == NULL){
			SetTreeCheck(m_ctrlTreeOptions,m_htiLancastEnabled,GetResString(IDS_X_LAN_ENABLE),m_htiLancast,GetResString(IDS_X_LAN_ENABLE_INFO),FALSE,m_bLancastEnabled);

			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiCustomizedLanCast,GetResString(IDS_X_LAN_GROUPE), m_htiLancast,GetResString(IDS_X_LAN_GROUPE_INFO),FALSE, m_bCustomizedLanCast);
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiLanCastPort,GetResString(IDS_X_LAN_PORT), m_htiCustomizedLanCast,GetResString(IDS_X_LAN_PORT_INFO));
		}else
			SetTreeRadioEx(m_htiLancastDisable,m_htiLancastEnable,m_htiLancastDefault,m_htiLancastGlobal,m_htiLancast,m_EnableLanCast);

			SetTreeNumEdit(m_ctrlTreeOptions,m_htiLANIntervals,GetResString(IDS_X_LAN_GET_SRC_TIME), m_htiLancast,GetResString(IDS_X_LAN_GET_SRC_TIME_INFO));
			SetTreeGroup(m_ctrlTreeOptions,m_htiLanCastReask,GetResString(IDS_X_LAN_REASK),iImgReaskLan, m_htiLancast, GetResString(IDS_X_LAN_REASK_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiLanReaskIntervals,GetResString(IDS_X_LAN_DL_REASK), m_htiLanCastReask,GetResString(IDS_X_LAN_DL_REASK_INFO));
				SetTreeNumEdit(m_ctrlTreeOptions,m_htiNnPLanReaskIntervals,GetResString(IDS_X_LAN_NNP_REASK), m_htiLanCastReask,GetResString(IDS_X_LAN_NNP_REASK_INFO));

		if(m_Category == NULL && m_paFiles == NULL){
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiAutoBroadcastLanFiles,GetResString(IDS_X_LAN_BROADCAST), m_htiLancast,GetResString(IDS_X_LAN_BROADCAST_INFO),FALSE, m_bAutoBroadcastLanFiles);
			SetTreeCheck(m_ctrlTreeOptions,m_htiUseLanMultiTransfer,GetResString(IDS_X_USE_LAN_MULTI_TRANSFER),m_htiLancast,GetResString(IDS_X_USE_LAN_MULTI_TRANSFER_INFO),FALSE,m_bUseLanMultiTransfer);
		}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		SetTreeGroup(m_ctrlTreeOptions,m_htiVoodoo,GetResString(IDS_X_VOODOO_MAIN),iImgVoodoo, TVI_ROOT, GetResString(IDS_X_VOODOO_MAIN_INFO));
		if(m_Category == NULL && m_paFiles == NULL){
			SetTreeCheck(m_ctrlTreeOptions,m_htiUseVoodooTransfer,GetResString(IDS_X_USE_VOODOO_TRANSFER),m_htiVoodoo,GetResString(IDS_X_USE_VOODOO_TRANSFER_INFO),FALSE,m_bUseVoodooTransfer);
			SetTreeCheck(m_ctrlTreeOptions,m_htiSlaveAllowed,GetResString(IDS_X_SLAVE_ALLOWED),m_htiVoodoo,GetResString(IDS_X_SLAVE_ALLOWED_INFO),FALSE,m_bSlaveAllowed);
			SetTreeCheck(m_ctrlTreeOptions,m_htiSlaveHosting,GetResString(IDS_X_SLAVE_HOSTING),m_htiVoodoo,GetResString(IDS_X_SLAVE_HOSTING_INFO),FALSE,m_bSlaveHosting);
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiAutoConnectVoodoo,GetResString(IDS_X_AUTO_CONNECT_VOODOO),m_htiVoodoo,GetResString(IDS_X_AUTO_CONNECT_VOODOO_INFO),TRUE,m_uAutoConnectVoodoo);
			SetTreeCheck(m_ctrlTreeOptions,m_htiUseVirtualVoodooFiles,GetResString(IDS_X_USE_VIRTUAL_VOODOO_FILES),m_htiVoodoo,GetResString(IDS_X_USE_VIRTUAL_VOODOO_FILES_INFO),TRUE,m_uUseVirtualVoodooFiles);
		}else
			SetTreeRadioEx(m_htiVoodooTransferDisable,m_htiVoodooTransferEnable,m_htiVoodooTransferDefault,m_htiVoodooTransferGlobal,m_htiVoodoo,m_EnableVoodoo);
		
			SetTreeGroup(m_ctrlTreeOptions,m_htiVoodooSourceExchange,GetResString(IDS_X_USE_VOODOO_SOURCE_EXCHANGE),iImgVoodooXs, m_htiVoodoo, GetResString(IDS_X_USE_VOODOO_SOURCE_EXCHANGE_INFO));
				SetTreeRadioXs(m_htiVoodooSourceExchangeDisable,m_htiVoodooSourceExchangeEnable1,m_htiVoodooSourceExchangeEnable2,m_htiVoodooSourceExchangeDefault,m_htiVoodooSourceExchangeGlobal,m_htiVoodooSourceExchange,m_VoodooXS);

		if(m_Category == NULL && m_paFiles == NULL){
			SetTreeEdit(m_ctrlTreeOptions,m_htiVoodooSpell,GetResString(IDS_X_VOODOO_SPELL), m_htiVoodoo,GetResString(IDS_X_VOODOO_SPELL_INFO));
			SetTreeNumEdit(m_ctrlTreeOptions,m_htiVoodooPort,GetResString(IDS_X_VOODOO_SOCKET_PORT), m_htiVoodoo,GetResString(IDS_X_VOODOO_SOCKET_PORT_INFO));
			SetTreeCheckNumEdit(m_ctrlTreeOptions,m_htiVoodooCastEnabled,GetResString(IDS_X_VOODOOCAST),m_htiVoodoo,GetResString(IDS_X_VOODOOCAST_INFO),TRUE,m_uVoodooCastEnabled);
				SetTreeCheck(m_ctrlTreeOptions,m_htiSearchForSlaves,GetResString(IDS_X_SEARCH_FOR_SLAVES), m_htiVoodooCastEnabled,GetResString(IDS_X_SEARCH_FOR_SLAVES_INFO),TRUE, m_uSearchForSlaves);
				SetTreeCheck(m_ctrlTreeOptions,m_htiSearchForMaster,GetResString(IDS_X_SEARCH_FOR_MASTER), m_htiVoodooCastEnabled,GetResString(IDS_X_SEARCH_FOR_MASTER_INFO),TRUE, m_uSearchForMaster);

			SetTreeCheck(m_ctrlTreeOptions,m_htiHideVoodooFiles,GetResString(IDS_X_HIDE_VOODOO_FILES), m_htiVoodoo,GetResString(IDS_X_HIDE_VOODOO_FILES_INFO),FALSE, m_bHideVoodooFiles); // NEO: PP - [PasswordProtection]
		}
#endif // VOODOO // NEO: VOODOO END

		if(m_Category == NULL && m_paFiles == NULL)
			CheckEnable();
		else
		{
			m_ctrlTreeOptions.SetItemEnable(m_htiLancast, NeoPrefs.IsLancastEnabled());
			m_ctrlTreeOptions.SetItemEnable(m_htiVoodoo, NeoPrefs.UseVoodooTransfer());
		}

		m_ctrlTreeOptions.Expand(TVI_ROOT, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiLancast, TVE_EXPAND);
		m_bInitializedTreeOpts = true;
	}

	if(m_Category == NULL && m_paFiles == NULL){
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxLanDownload, m_iMaxLanDownload);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLanUploadBufferSize, m_bSetLanUploadBuffer);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiLanUploadBufferSize, m_iLanUploadBufferSize);

		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxLanUpload, m_iMaxLanUpload);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLanDownloadBufferSize, m_bSetLanDownloadBuffer);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiLanDownloadBufferSize, m_iLanDownloadBufferSize);

			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiMaxLanUploadSlots, m_iMaxLanUploadSlots);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCustomLanCastAdapter, m_bCustomLanCastAdapter);
		DDX_AdapterIndex(&m_ctrlTreeOptions,pDX, IDC_MOD_OPTS, m_htiCustomLanCastAdapter, m_uLanCastAdapterIPAdress);
			DDX_AdapterIndex(&m_ctrlTreeOptions,pDX, IDC_MOD_OPTS, m_htiCustomLanCastMask, m_uLanCastAdapterSubNet);
	}

	if(m_Category == NULL && m_paFiles == NULL){
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLancastEnabled, m_bLancastEnabled);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCustomizedLanCast, m_bCustomizedLanCast);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiCustomizedLanCast, m_sLanCastGroup);
			DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiLanCastPort, m_uLanCastPort);
	}else
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiLancast, m_EnableLanCast);

		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiLANIntervals, m_LcIntervals);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiLanReaskIntervals, m_LanSourceReaskTime);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiNnPLanReaskIntervals, m_LanNNPSourceReaskTime);

	if(m_Category == NULL && m_paFiles == NULL){
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiAutoBroadcastLanFiles, m_bAutoBroadcastLanFiles);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAutoBroadcastLanFiles, m_iAutoBroadcastLanFiles);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseLanMultiTransfer, m_bUseLanMultiTransfer);
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(m_Category == NULL && m_paFiles == NULL){
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseVoodooTransfer, m_bUseVoodooTransfer);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSlaveAllowed, m_bSlaveAllowed);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSlaveHosting, m_bSlaveHosting);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiAutoConnectVoodoo, m_uAutoConnectVoodoo);
		DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiAutoConnectVoodoo, m_iVoodooReconectTime);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseVirtualVoodooFiles, m_uUseVirtualVoodooFiles);
	}else
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiVoodoo, m_EnableVoodoo);
		DDX_TreeRadioEx(pDX, IDC_MOD_OPTS, m_htiVoodooSourceExchange, m_VoodooXS);
	if(m_Category == NULL && m_paFiles == NULL){
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiVoodooSpell, m_sVoodooSpell);
		DDX_TreeEdit(pDX, IDC_MOD_OPTS, m_htiVoodooPort, m_nVoodooPort);
		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiVoodooCastEnabled, m_uVoodooCastEnabled);
			DDX_TreeEditEx(pDX, IDC_MOD_OPTS, m_htiVoodooCastEnabled, m_iVoodooSearchIntervals);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSearchForSlaves, m_uSearchForSlaves);
			DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiSearchForMaster, m_uSearchForMaster);

		DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiHideVoodooFiles, m_bHideVoodooFiles); // NEO: PP - [PasswordProtection]
	}
#endif // VOODOO // NEO: VOODOO END
}

BOOL CPPgLancast::OnInitDialog()
{
	// NEO: FCFG - [FileConfiguration]
	if(m_paFiles)
		RefreshData();
	else
	// NEO: FCFG END
		LoadSettings();
		

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	// NEO: FCFG - [FileConfiguration]
	// no need to explicitly call 'RefreshData' here, 'OnSetActive' will be called right after 'OnInitDialog'

	// start time for calling 'RefreshData'
	if(m_paFiles)
		VERIFY( (m_timer = SetTimer(301, 5000, 0)) != NULL );
	// NEO: FCFG END

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgLancast::SetLimits(){
	// General
	m_LcIntervals.MSMG(MIN_LC_INTERVALS,DEF_LC_INTERVALS,MAX_LC_INTERVALS,NeoPrefs.PartPrefs.m_LcIntervals,'t');
	m_LanSourceReaskTime.MSMG(MIN_LAN_SOURCE_REASK_TIME,DEF_LAN_SOURCE_REASK_TIME,MAX_LAN_SOURCE_REASK_TIME,NeoPrefs.PartPrefs.m_LanSourceReaskTime,'t');
	m_LanNNPSourceReaskTime.MSMG(MIN_LAN_NNP_SOURCE_REASK_TIME,DEF_LAN_NNP_SOURCE_REASK_TIME,MAX_LAN_NNP_SOURCE_REASK_TIME,NeoPrefs.PartPrefs.m_LanNNPSourceReaskTime,'t');
	m_iAutoBroadcastLanFiles.MSMG(MIN_AUTO_BROADCAST_LAN_FILES,DEF_AUTO_BROADCAST_LAN_FILES,MAX_AUTO_BROADCAST_LAN_FILES,NeoPrefs.m_iAutoBroadcastLanFiles,'t');

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_iVoodooReconectTime.MSMG(MIN_VOODOO_RECONNECT_TIME,DEF_VOODOO_RECONNECT_TIME,MAX_VOODOO_RECONNECT_TIME,NeoPrefs.m_iVoodooReconectTime,'t');
	m_iVoodooSearchIntervals.MSMG(MIN_VOODOO_SEARCH,DEF_VOODOO_SEARCH,MAX_VOODOO_SEARCH,NeoPrefs.m_iVoodooSearchIntervals,'t');
#endif // VOODOO // NEO: VOODOO END
}

void CPPgLancast::LoadSettings()
{
	/*
	* Einstellungen Laden
	*/

	SetLimits();

	if(m_Category)
	{
		CPartPreferences* PartPrefs = m_Category->PartPrefs;
		CKnownPreferences* KnownPrefs = m_Category->KnownPrefs;

		m_EnableLanCast.CVDC(1, KnownPrefs ? KnownPrefs->m_EnableLanCast : FCFG_DEF, NeoPrefs.KnownPrefs.m_EnableLanCast);
		m_LcIntervals.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_LcIntervals : FCFG_DEF);

		m_LanSourceReaskTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_LanSourceReaskTime : FCFG_DEF);
		m_LanNNPSourceReaskTime.DV(FCFG_GLB, PartPrefs ? PartPrefs->m_LanNNPSourceReaskTime : FCFG_DEF);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		m_EnableVoodoo.CVDC(1, KnownPrefs ? KnownPrefs->m_EnableVoodoo : FCFG_DEF, NeoPrefs.KnownPrefs.m_EnableVoodoo);

		m_VoodooXS.CVDC(2, PartPrefs ? PartPrefs->m_VoodooXS : FCFG_DEF, NeoPrefs.PartPrefs.m_VoodooXS);
#endif // VOODOO // NEO: VOODOO END
	}
	else
	{
		m_bLancastEnabled = NeoPrefs.m_bLancastEnabled;

		m_iMaxLanDownload = NeoPrefs.m_iMaxLanDownload;
		m_bSetLanDownloadBuffer = NeoPrefs.m_bSetLanDownloadBuffer;
		m_iLanDownloadBufferSize = NeoPrefs.m_iLanDownloadBufferSize;

		m_iMaxLanUpload = NeoPrefs.m_iMaxLanUpload;
		m_bSetLanUploadBuffer = NeoPrefs.m_bSetLanUploadBuffer;
		m_iLanUploadBufferSize = NeoPrefs.m_iLanUploadBufferSize;

		m_iMaxLanUploadSlots = NeoPrefs.m_iMaxLanUploadSlots;

		m_bCustomizedLanCast = NeoPrefs.m_bCustomizedLanCast;
		m_sLanCastGroup = NeoPrefs.m_sLanCastGroup;
		m_uLanCastPort = NeoPrefs.m_uLanCastPort;

		m_bCustomLanCastAdapter = NeoPrefs.m_bCustomLanCastAdapter;
		m_uLanCastAdapterIPAdress = NeoPrefs.m_uLanCastAdapterIPAdress;
		m_uLanCastAdapterSubNet = NeoPrefs.m_uLanCastAdapterSubNet;

		//
		m_LcIntervals.DV(FCFG_STD, NeoPrefs.PartPrefs.m_LcIntervals, true);

		m_LanSourceReaskTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_LanSourceReaskTime, true);
		m_LanNNPSourceReaskTime.DV(FCFG_STD, NeoPrefs.PartPrefs.m_LanNNPSourceReaskTime, true);

		m_bAutoBroadcastLanFiles = NeoPrefs.m_bAutoBroadcastLanFiles;
		m_iAutoBroadcastLanFiles.DV(FCFG_STD, NeoPrefs.m_iAutoBroadcastLanFiles, true);

		m_bUseLanMultiTransfer = NeoPrefs.m_bUseLanMultiTransfer;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		m_bUseVoodooTransfer = NeoPrefs.m_bUseVoodooTransfer;
		m_bSlaveAllowed = NeoPrefs.m_bSlaveAllowed;
		m_bSlaveHosting = NeoPrefs.m_bSlaveHosting;

		m_sVoodooSpell = NeoPrefs.m_sVoodooSpell;
		m_nVoodooPort = NeoPrefs.m_nVoodooPort;

		m_uAutoConnectVoodoo = NeoPrefs.m_uAutoConnectVoodoo;
		m_iVoodooReconectTime.DV(FCFG_STD, NeoPrefs.m_iVoodooReconectTime, true);

		m_uUseVirtualVoodooFiles = NeoPrefs.m_uUseVirtualVoodooFiles;

		m_VoodooXS.CVDC(2, NeoPrefs.PartPrefs.m_VoodooXS);

		m_uVoodooCastEnabled = NeoPrefs.m_uVoodooCastEnabled;

		m_uSearchForSlaves = NeoPrefs.m_uSearchForSlaves;
		m_uSearchForMaster = NeoPrefs.m_uSearchForMaster;
		m_iVoodooSearchIntervals.DV(FCFG_STD, NeoPrefs.m_iVoodooSearchIntervals, true);

		m_bHideVoodooFiles = NeoPrefs.m_bHideVoodooFiles; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END
	}

}

// NEO: FCFG - [FileConfiguration]
void CPPgLancast::RefreshData()
{
	/*
	* Datei Einstellungen Laden
	*/

	SetLimits();

	if(!theApp.downloadqueue->IsPartFile((CKnownFile*)(*m_paFiles)[0]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)(*m_paFiles)[0]))
		return;

	const CKnownFile* KnownFile = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[0]);

	const CPartFile* PartFile = KnownFile->IsPartFile() ? (CPartFile*)KnownFile : NULL;

	CKnownPreferences* KnownPrefs = KnownFile->KnownPrefs;
	CPartPreferences* PartPrefs = PartFile ? PartFile->PartPrefs : NULL;
	Category_Struct* Category = thePrefs.GetCategory(KnownFile->GetCategory());
	ASSERT(Category);

	m_EnableLanCast.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_EnableLanCast : FCFG_DEF, NeoPrefs.KnownPrefs.m_EnableLanCast, Category && Category->KnownPrefs ? Category->KnownPrefs->m_EnableLanCast : -1);
	m_LcIntervals.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_LcIntervals : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_LcIntervals : FCFG_DEF);

	m_LanSourceReaskTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_LanSourceReaskTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_LanSourceReaskTime : FCFG_DEF);
	m_LanNNPSourceReaskTime.DV(Category == NULL ? FCFG_DEF : Category->PartPrefs ? Category->PartPrefs->m_LanNNPSourceReaskTime : FCFG_DEF, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_LanNNPSourceReaskTime : FCFG_DEF);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_EnableVoodoo.CVDC(1, (KnownPrefs && KnownPrefs->IsFilePrefs()) ? KnownPrefs->m_EnableVoodoo : FCFG_DEF, NeoPrefs.KnownPrefs.m_EnableVoodoo, Category && Category->KnownPrefs ? Category->KnownPrefs->m_EnableVoodoo : -1);

	m_VoodooXS.CVDC(2, (PartPrefs && PartPrefs->IsFilePrefs()) ? PartPrefs->m_VoodooXS : FCFG_DEF, NeoPrefs.PartPrefs.m_VoodooXS, Category && Category->PartPrefs ? Category->PartPrefs->m_VoodooXS : -1);
#endif // VOODOO // NEO: VOODOO END

	for (int i = 1; i < m_paFiles->GetSize(); i++)
	{
		if(!theApp.downloadqueue->IsPartFile((CKnownFile*)(*m_paFiles)[i]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)(*m_paFiles)[i]))
			continue;

		KnownFile = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
		PartFile = KnownFile->IsPartFile() ? (CPartFile*)KnownFile : NULL;

		KnownPrefs = KnownFile->KnownPrefs;
		PartPrefs = PartFile ? PartFile->PartPrefs : NULL;

		if(Category && Category != thePrefs.GetCategory(KnownFile->GetCategory()))
		{
			m_EnableLanCast.Cat = -2;
			m_LcIntervals.Def = FCFG_UNK;

			m_LanSourceReaskTime.Def = FCFG_UNK;
			m_LanNNPSourceReaskTime.Def = FCFG_UNK;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			m_EnableVoodoo.Cat = -2;

			m_VoodooXS.Cat = -2;
#endif // VOODOO // NEO: VOODOO END

			Category = NULL;
		}

		if(m_EnableLanCast.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_EnableLanCast : FCFG_DEF)) m_EnableLanCast.Val = FCFG_UNK;
		if(m_LcIntervals.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_LcIntervals : FCFG_DEF)) m_LcIntervals.Val = FCFG_UNK;

		if(m_LanSourceReaskTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_LanSourceReaskTime : FCFG_DEF)) m_LanSourceReaskTime.Val = FCFG_UNK;
		if(m_LanNNPSourceReaskTime.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_LanNNPSourceReaskTime : FCFG_DEF)) m_LanNNPSourceReaskTime.Val = FCFG_UNK;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		if(m_EnableVoodoo.Val != (KnownPrefs && KnownPrefs->IsFilePrefs() ? KnownPrefs->m_EnableVoodoo : FCFG_DEF)) m_EnableVoodoo.Val = FCFG_UNK;

		if(m_VoodooXS.Val != (PartPrefs && PartPrefs->IsFilePrefs() ? PartPrefs->m_VoodooXS : FCFG_DEF)) m_VoodooXS.Val = FCFG_UNK;
#endif // VOODOO // NEO: VOODOO END
	}

	UpdateData(FALSE);
}

void CPPgLancast::GetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs, bool OperateData)
{
	if(OperateData)
		UpdateData();
	if(KnownPrefs)
	{
		if(m_EnableLanCast.Val != FCFG_UNK) KnownPrefs->m_EnableLanCast = m_EnableLanCast.Val;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		if(m_EnableVoodoo.Val != FCFG_UNK) KnownPrefs->m_EnableVoodoo = m_EnableVoodoo.Val;
#endif // VOODOO // NEO: VOODOO END
	}
	if(PartPrefs)
	{
		if(m_LcIntervals.Val != FCFG_UNK) PartPrefs->m_LcIntervals = m_LcIntervals.Val;

		if(m_LanSourceReaskTime.Val != FCFG_UNK) PartPrefs->m_LanSourceReaskTime = m_LanSourceReaskTime.Val;
		if(m_LanNNPSourceReaskTime.Val != FCFG_UNK) PartPrefs->m_LanNNPSourceReaskTime = m_LanNNPSourceReaskTime.Val;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		if(m_VoodooXS.Val != FCFG_UNK) PartPrefs->m_VoodooXS = m_VoodooXS.Val;
#endif // VOODOO // NEO: VOODOO END
	}

}

void CPPgLancast::SetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs, bool OperateData)
{
	if(KnownPrefs)
	{
		m_EnableLanCast.Val = KnownPrefs->m_EnableLanCast;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		m_EnableVoodoo.Val = KnownPrefs->m_EnableVoodoo;
#endif // VOODOO // NEO: VOODOO END
	}
	if(PartPrefs)
	{
		m_LcIntervals.Val = PartPrefs->m_LcIntervals;

		m_LanSourceReaskTime.Val = PartPrefs->m_LanSourceReaskTime;
		m_LanNNPSourceReaskTime.Val = PartPrefs->m_LanNNPSourceReaskTime;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		m_VoodooXS.Val = PartPrefs->m_VoodooXS;
#endif // VOODOO // NEO: VOODOO END
	}
	if(OperateData)
	{
		UpdateData(FALSE);
		SetModified();
	}
}
// NEO: FCFG END

BOOL CPPgLancast::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	/*
	* Einstellungen Speichern
	*/

	// NEO: FCFG - [FileConfiguration]
	if(m_paFiles)
	{
		CKnownFile* KnownFile;
		CKnownPreferences* KnownPrefs;
		CPartFile* PartFile;
		CPartPreferences* PartPrefs;
		for (int i = 0; i < m_paFiles->GetSize(); i++)
		{
			// check if the file is still valid
			if(!theApp.downloadqueue->IsPartFile((CKnownFile*)(*m_paFiles)[i]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)(*m_paFiles)[i]))
				continue;

			KnownFile = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);

			PartFile = KnownFile->IsPartFile() ? (CPartFile*)KnownFile : NULL;

			// get ot create Preferences 
			KnownPrefs = KnownFile->KnownPrefs;
			if(!KnownPrefs->IsFilePrefs())
				KnownPrefs = new CKnownPreferencesEx(CFP_FILE);
		
			PartPrefs = PartFile ? PartFile->PartPrefs : NULL;
			if(PartFile && !PartPrefs->IsFilePrefs())
				PartPrefs = new CPartPreferencesEx(CFP_FILE);

			// save preferences
			GetFilePreferences(KnownPrefs, PartPrefs);

			// check validiti of the tweaks
			KnownPrefs->CheckTweaks();
			if(PartFile)
				PartPrefs->CheckTweaks();

			// valiate pointers and update (!)
			KnownFile->UpdateKnownPrefs(KnownPrefs);
			if(PartFile)
				PartFile->UpdatePartPrefs(PartPrefs);
		}

		RefreshData();
	}
	else if(m_Category)
	{
		int cat = thePrefs.FindCategory(m_Category);
		if(cat != -1)
		{
			// get ot create Preferences 
			CKnownPreferences* KnownPrefs = m_Category->KnownPrefs;
			if(KnownPrefs == NULL)
				KnownPrefs = new CKnownPreferencesEx(CFP_CATEGORY);

			CPartPreferences* PartPrefs = m_Category->PartPrefs;
			if(PartPrefs == NULL)
				PartPrefs = new CPartPreferencesEx(CFP_CATEGORY);

			// save preferences
			GetFilePreferences(KnownPrefs, PartPrefs);

			// check validiti of the tweaks
			KnownPrefs->CheckTweaks();
			PartPrefs->CheckTweaks();

			// valiate pointers and update (!)
			theApp.downloadqueue->UpdatePartPrefs(PartPrefs, KnownPrefs, (UINT)cat); // may delete PartPrefs
			theApp.knownfiles->UpdateKnownPrefs(KnownPrefs, (UINT)cat); // may delete KnownPrefs

			thePrefs.SaveCats();

			LoadSettings();
		}
		else
			AfxMessageBox(GetResString(IDS_X_INVALID_CAT),MB_OK | MB_ICONSTOP, NULL);
	}
	else
	// NEO: FCFG END
	{

		bool bUpdateLancast = (NeoPrefs.m_bLancastEnabled != m_bLancastEnabled);
		NeoPrefs.m_bLancastEnabled = m_bLancastEnabled;

		NeoPrefs.m_iMaxLanDownload = (uint16)m_iMaxLanDownload;
		NeoPrefs.m_bSetLanDownloadBuffer = m_bSetLanDownloadBuffer;
		NeoPrefs.m_iLanDownloadBufferSize = m_iLanDownloadBufferSize;

		NeoPrefs.m_iMaxLanUpload = (uint16)m_iMaxLanUpload;
		NeoPrefs.m_bSetLanUploadBuffer = m_bSetLanUploadBuffer;
		NeoPrefs.m_iLanUploadBufferSize = m_iLanUploadBufferSize;

		NeoPrefs.m_iMaxLanUploadSlots = m_iMaxLanUploadSlots;

		if ((NeoPrefs.m_bCustomizedLanCast != m_bCustomizedLanCast)
		||	(NeoPrefs.m_sLanCastGroup != m_sLanCastGroup)
		|| (NeoPrefs.m_uLanCastPort != m_uLanCastPort))
			bUpdateLancast = true;

		NeoPrefs.m_bCustomizedLanCast = m_bCustomizedLanCast;
		NeoPrefs.m_sLanCastGroup = m_sLanCastGroup;
		NeoPrefs.m_uLanCastPort = (uint16)m_uLanCastPort;

		bool bUpdateLanAdapter =  ((NeoPrefs.m_bCustomLanCastAdapter != m_bCustomLanCastAdapter)
		||	(NeoPrefs.m_uLanCastAdapterIPAdress != m_uLanCastAdapterIPAdress)
		|| (NeoPrefs.m_uLanCastAdapterSubNet != m_uLanCastAdapterSubNet));

		if(bUpdateLancast)
			bUpdateLanAdapter = true;

		NeoPrefs.m_bCustomLanCastAdapter = m_bCustomLanCastAdapter;
		NeoPrefs.m_uLanCastAdapterIPAdress = m_uLanCastAdapterIPAdress;
		NeoPrefs.m_uLanCastAdapterSubNet = m_uLanCastAdapterSubNet;

		NeoPrefs.m_bAutoBroadcastLanFiles = m_bAutoBroadcastLanFiles;
		if(m_iAutoBroadcastLanFiles.Val != FCFG_UNK) NeoPrefs.m_iAutoBroadcastLanFiles = m_iAutoBroadcastLanFiles.Val;

		NeoPrefs.m_bUseLanMultiTransfer = m_bUseLanMultiTransfer;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		bool bUpdateVoodoo = ((NeoPrefs.m_bUseVoodooTransfer != m_bUseVoodooTransfer)
						|| (NeoPrefs.m_bSlaveAllowed != m_bSlaveAllowed));

		bool bUpdateVoodooSocket = ((NeoPrefs.m_bSlaveAllowed != m_bSlaveAllowed)
							||(NeoPrefs.m_bSlaveHosting != m_bSlaveHosting));

		if(bUpdateVoodooSocket)
			bUpdateLanAdapter = true;

		NeoPrefs.m_bUseVoodooTransfer = m_bUseVoodooTransfer;
		NeoPrefs.m_bSlaveAllowed = m_bSlaveAllowed;
		NeoPrefs.m_bSlaveHosting = m_bSlaveHosting;

		NeoPrefs.m_sVoodooSpell = m_sVoodooSpell;
		bool bUpdateVoodooPort = (NeoPrefs.m_nVoodooPort != m_nVoodooPort);
		NeoPrefs.m_nVoodooPort = (uint16)m_nVoodooPort;

		NeoPrefs.m_uAutoConnectVoodoo = m_uAutoConnectVoodoo;
		if(m_iVoodooReconectTime.Val != FCFG_UNK) NeoPrefs.m_iVoodooReconectTime = m_iVoodooReconectTime.Val;

		NeoPrefs.m_uUseVirtualVoodooFiles = m_uUseVirtualVoodooFiles;

		theApp.emuledlg->searchwnd->m_pwndResults->m_pwndParams->m_ctlVoodoo.EnableWindow(m_bUseVoodooTransfer);  // NEO: VOODOOs - [VoodooSearchForwarding]

		NeoPrefs.m_uVoodooCastEnabled = m_uVoodooCastEnabled;

		NeoPrefs.m_uSearchForSlaves = m_uSearchForSlaves;
		NeoPrefs.m_uSearchForMaster = m_uSearchForMaster;
		if(m_iVoodooSearchIntervals.Val != FCFG_UNK) NeoPrefs.m_iVoodooSearchIntervals = m_iVoodooSearchIntervals.Val;

		NeoPrefs.m_bHideVoodooFiles = m_bHideVoodooFiles; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

		if(bUpdateLanAdapter)
			theApp.lancast->SelectAdapters();

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
		if(bUpdateVoodoo && (NeoPrefs.m_bUseVoodooTransfer || NeoPrefs.m_bSlaveAllowed)){
			if(!theApp.voodoo->IsStarted())
				theApp.voodoo->Start();
		}else if(bUpdateVoodoo){
			if(theApp.voodoo->IsStarted())
				theApp.voodoo->Stop();
		}
		if(bUpdateVoodooSocket && (NeoPrefs.m_bSlaveAllowed || NeoPrefs.m_bSlaveHosting)){
			if(!theApp.voodoo->IsListening())
				theApp.voodoo->StartListening();
		}else if(bUpdateVoodooSocket){
			if(theApp.voodoo->IsListening())
				theApp.voodoo->Close();
		}else if(bUpdateVoodooPort){
			if(theApp.voodoo->IsListening())
				theApp.voodoo->Rebind();
		}
#endif // VOODOO // NEO: VOODOO END

		if(bUpdateLancast){
			if(theApp.lancast->IsConnected())
				theApp.lancast->Stop();
			if(NeoPrefs.m_bLancastEnabled)
				theApp.lancast->Start();
		}

		GetFilePreferences(&NeoPrefs.KnownPrefs, &NeoPrefs.PartPrefs);

		NeoPrefs.KnownPrefs.CheckTweaks();
		NeoPrefs.PartPrefs.CheckTweaks();

		NeoPrefs.CheckNeoPreferences();
		LoadSettings();
	}

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgLancast::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgLancast::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	ClearAllMembers();
	CPropertyPage::OnDestroy();
}

LRESULT CPPgLancast::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code == EN_KILLFOCUS){			
			if(m_htiLanDownloadBufferSize && pton->hItem == m_htiLanDownloadBufferSize){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLanDownloadBufferSize, VAL_LAN_DOWNLOAD_BUFFER_MIN, VAL_LAN_DOWNLOAD_BUFFER_DEF, VAL_LAN_DOWNLOAD_BUFFER_MAX)) SetModified();
			}else if(m_htiLanUploadBufferSize && pton->hItem == m_htiLanUploadBufferSize){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLanUploadBufferSize, VAL_LAN_UPLOAD_BUFFER_MIN, VAL_LAN_UPLOAD_BUFFER_DEF, VAL_LAN_UPLOAD_BUFFER_MAX)) SetModified();
			}else 
			if(m_htiMaxLanUploadSlots && pton->hItem == m_htiMaxLanUploadSlots){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiMaxLanUploadSlots, VAL_MAX_LAN_UPLOAD_SLOTS_MIN, VAL_MAX_LAN_UPLOAD_SLOTS_DEF, VAL_MAX_LAN_UPLOAD_SLOTS_MAX)) SetModified();
			}else 
			if(m_htiLANIntervals && pton->hItem == m_htiLANIntervals){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLANIntervals, m_LcIntervals)) SetModified();
			}else if(m_htiLanReaskIntervals && pton->hItem == m_htiLanReaskIntervals){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiLanReaskIntervals, m_LanSourceReaskTime)) SetModified();
			}else if(m_htiNnPLanReaskIntervals && pton->hItem == m_htiNnPLanReaskIntervals){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiNnPLanReaskIntervals, m_LanNNPSourceReaskTime)) SetModified();
			}else 

			if(m_htiAutoBroadcastLanFiles && pton->hItem == m_htiAutoBroadcastLanFiles){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoBroadcastLanFiles, m_iAutoBroadcastLanFiles)) SetModified();
			}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			else if(m_htiAutoConnectVoodoo && pton->hItem == m_htiAutoConnectVoodoo){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiAutoConnectVoodoo, m_iVoodooReconectTime)) SetModified();
			}
			else if(m_htiVoodooCastEnabled && pton->hItem == m_htiVoodooCastEnabled){
				if(CheckTreeEditLimit(m_ctrlTreeOptions, m_htiVoodooCastEnabled, m_iVoodooSearchIntervals)) SetModified();
			}
#endif // VOODOO // NEO: VOODOO END
		}else{
			UINT bCheck;
			if(m_htiLancastEnabled && pton->hItem == m_htiLancastEnabled){
				CheckEnable();
			}else
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			if(m_htiUseVoodooTransfer && pton->hItem == m_htiUseVoodooTransfer){
				CheckEnable();
			}else
			if(m_htiSlaveHosting && pton->hItem == m_htiSlaveHosting){
				CheckEnable();
			}else
			if(m_htiSlaveAllowed && pton->hItem == m_htiSlaveAllowed){
				CheckEnable();
			}else
#endif // VOODOO // NEO: VOODOO END
			if(m_htiLanDownloadBufferSize && pton->hItem == m_htiLanDownloadBufferSize){
				m_ctrlTreeOptions.GetCheckBox(m_htiLanDownloadBufferSize, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiLanDownloadBufferSize, bCheck, TRUE, TRUE);
			}
			else if(m_htiLanUploadBufferSize && pton->hItem == m_htiLanUploadBufferSize){
				m_ctrlTreeOptions.GetCheckBox(m_htiLanUploadBufferSize, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiLanUploadBufferSize, bCheck, TRUE, TRUE);
			}else
			if(m_htiAutoBroadcastLanFiles && pton->hItem == m_htiAutoBroadcastLanFiles){
				m_ctrlTreeOptions.GetCheckBox(m_htiAutoBroadcastLanFiles, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiAutoBroadcastLanFiles, bCheck,TRUE,TRUE);
			}else
			if(m_htiCustomLanCastAdapter && pton->hItem == m_htiCustomLanCastAdapter){
				m_ctrlTreeOptions.GetCheckBox(m_htiCustomLanCastAdapter, bCheck);
				m_ctrlTreeOptions.SetGroupEnable(m_htiCustomLanCastAdapter, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiCustomLanCastAdapter, bCheck,TRUE,TRUE);
			}else
			if(m_htiCustomizedLanCast && pton->hItem == m_htiCustomizedLanCast){
				m_ctrlTreeOptions.GetCheckBox(m_htiCustomizedLanCast, bCheck);
				m_ctrlTreeOptions.SetGroupEnable(m_htiCustomizedLanCast, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiCustomizedLanCast, bCheck,TRUE,TRUE);
			}else
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			if(m_htiUseVoodooTransfer && pton->hItem == m_htiUseVoodooTransfer){
				m_ctrlTreeOptions.GetCheckBox(m_htiUseVoodooTransfer, bCheck);
				m_ctrlTreeOptions.SetGroupEnable(m_htiSlaveHosting, bCheck);
			}
			if(m_htiAutoConnectVoodoo && pton->hItem == m_htiAutoConnectVoodoo){
				m_ctrlTreeOptions.GetCheckBox(m_htiAutoConnectVoodoo, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiAutoConnectVoodoo, bCheck,FALSE,TRUE);
				m_ctrlTreeOptions.SetGroupEnable(m_htiAutoConnectVoodoo, bCheck);
			}
			if(m_htiVoodooCastEnabled && pton->hItem == m_htiVoodooCastEnabled){
				m_ctrlTreeOptions.GetCheckBox(m_htiVoodooCastEnabled, bCheck);
				m_ctrlTreeOptions.SetItemEnable(m_htiVoodooCastEnabled, bCheck,FALSE,TRUE);
				m_ctrlTreeOptions.SetGroupEnable(m_htiVoodooCastEnabled, bCheck);
			}
#endif // VOODOO // NEO: VOODOO END
			SetModified();
		}
	}
	return 0;
}

void CPPgLancast::CheckEnable(){
	UINT bLanSupport;
	UINT bLanCast;
	m_ctrlTreeOptions.GetCheckBox(m_htiLancastEnabled, bLanCast);
	bLanSupport = bLanCast;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	UINT bVoodoo;
	m_ctrlTreeOptions.GetCheckBox(m_htiUseVoodooTransfer, bVoodoo);
	if(!bVoodoo)
		m_ctrlTreeOptions.GetCheckBox(m_htiSlaveAllowed, bVoodoo);
	if(bVoodoo)
		bLanSupport = TRUE;
#endif // VOODOO // NEO: VOODOO END

	m_ctrlTreeOptions.SetItemEnable(m_htiLancastUpload, bLanSupport);
	m_ctrlTreeOptions.SetItemEnable(m_htiLancastDownload, bLanSupport);
	m_ctrlTreeOptions.SetItemEnable(m_htiCustomLanCastAdapter, bLanSupport);

	m_ctrlTreeOptions.SetItemEnable(m_htiCustomizedLanCast, bLanCast);
	m_ctrlTreeOptions.SetItemEnable(m_htiLANIntervals, bLanCast);
	m_ctrlTreeOptions.SetItemEnable(m_htiLanCastReask, bLanCast);
	m_ctrlTreeOptions.SetItemEnable(m_htiAutoBroadcastLanFiles, bLanCast);
	m_ctrlTreeOptions.SetItemEnable(m_htiUseLanMultiTransfer, bLanCast);

	UINT bCheck;

	if(bLanSupport){
		m_ctrlTreeOptions.GetCheckBox(m_htiLanDownloadBufferSize, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiLanDownloadBufferSize, bCheck, TRUE, TRUE);
		m_ctrlTreeOptions.GetCheckBox(m_htiLanUploadBufferSize, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiLanUploadBufferSize, bCheck, TRUE, TRUE);

		m_ctrlTreeOptions.GetCheckBox(m_htiCustomLanCastAdapter, bCheck);
		m_ctrlTreeOptions.SetGroupEnable(m_htiCustomLanCastAdapter, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiCustomLanCastAdapter, bCheck,TRUE,TRUE);
		m_ctrlTreeOptions.SetItemEnable(m_htiCustomLanCastMask, bCheck);

		// Note: Lan Downlaod speed management is _only_ available with Neo Download bandwifdth Throtler
		m_ctrlTreeOptions.SetItemEnable(m_htiMaxLanDownload, bLanCast && NeoPrefs.UseDownloadBandwidthThrottler());
		m_ctrlTreeOptions.SetItemEnable(m_htiMaxLanUpload, bLanCast);
	}

	if(bLanCast){
		m_ctrlTreeOptions.GetCheckBox(m_htiCustomizedLanCast, bCheck);
		m_ctrlTreeOptions.SetGroupEnable(m_htiCustomizedLanCast, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiCustomizedLanCast, bCheck,TRUE,TRUE);

		m_ctrlTreeOptions.GetCheckBox(m_htiAutoBroadcastLanFiles, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiAutoBroadcastLanFiles, bCheck,TRUE,TRUE);
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	UINT bVoodooSocket = FALSE;
	if(bVoodoo){
		m_ctrlTreeOptions.GetCheckBox(m_htiSlaveHosting, bVoodooSocket);
		if(!bVoodooSocket)
			m_ctrlTreeOptions.GetCheckBox(m_htiSlaveAllowed, bVoodooSocket);
	}
	m_ctrlTreeOptions.SetItemEnable(m_htiSlaveHosting, bVoodoo);
	m_ctrlTreeOptions.SetItemEnable(m_htiAutoConnectVoodoo, bVoodoo);
	m_ctrlTreeOptions.SetItemEnable(m_htiUseVirtualVoodooFiles, bVoodoo);
	m_ctrlTreeOptions.SetItemEnable(m_htiVoodooSourceExchange, bVoodoo);
	m_ctrlTreeOptions.SetItemEnable(m_htiVoodooSpell, bVoodoo);
	m_ctrlTreeOptions.SetItemEnable(m_htiVoodooPort, bVoodooSocket);
	m_ctrlTreeOptions.SetItemEnable(m_htiVoodooCastEnabled, bVoodoo && bLanCast);

	if(bVoodoo){
		m_ctrlTreeOptions.GetCheckBox(m_htiUseVoodooTransfer, bCheck);
		m_ctrlTreeOptions.SetItemEnable(m_htiSlaveHosting, bCheck);

		m_ctrlTreeOptions.GetCheckBox(m_htiAutoConnectVoodoo, bCheck);
		m_ctrlTreeOptions.SetGroupEnable(m_htiAutoConnectVoodoo, bCheck && bLanCast);

		m_ctrlTreeOptions.GetCheckBox(m_htiVoodooCastEnabled, bCheck);
		m_ctrlTreeOptions.SetGroupEnable(m_htiVoodooCastEnabled, bCheck && bLanCast);
	}
#endif // VOODOO // NEO: VOODOO END
}

LRESULT CPPgLancast::DrawTreeItemHelp(WPARAM wParam, LPARAM lParam)
{
	if(!IsWindowVisible())
		return 0;

	if (wParam == IDC_MOD_OPTS){
		CString* sInfo = (CString*)lParam;
		SetDlgItemText(IDC_MOD_OPTS_INFO, *sInfo);
	}
	return FALSE;
}

void CPPgLancast::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgLancast::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgLancast::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

#endif //LANCAST // NEO: NLC END <-- Xanatos --