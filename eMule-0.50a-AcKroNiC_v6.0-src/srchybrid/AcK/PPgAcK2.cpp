//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "UserMsgs.h"
#include "emuleDlg.h"
#include "PPgAcK2.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "Opcodes.h"
//>>> taz::minRQR
#include "TransferDlg.h"
//<<< taz::minRQR

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgAcK2, CPropertyPage)
BEGIN_MESSAGE_MAP(CPPgAcK2, CPropertyPage)
//>>> taz::AcK Links
	ON_BN_CLICKED(IDC_STATIC,OnBnClickedAckHelp)
	ON_BN_CLICKED(IDC_ACKHELP,OnBnClickedAckHelp)
	ON_BN_CLICKED(IDC_ACKHELP2,OnBnClickedAckHelp2) //>>> taz:: fix restore 3button-link now pointing to AcKroNiC WiKi [Mulo da Soma]
	ON_BN_CLICKED(IDC_ACKHELP3,OnBnClickedAckHelp3)
//<<< taz::AcK Links

	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgAcK2::CPPgAcK2()
	: CPropertyPage(CPPgAcK2::IDD)
, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	Nullify();
}

CPPgAcK2::~CPPgAcK2()
{
}

void CPPgAcK2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACK_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgDM = 8;
		int iImgUM = 8;
		int iImgDisplay = 8;
		int iImgNotifications = 8;
		int iImgQuickstart = 8; //>>> taz::Quick start [TPT]
		int iImgDropDefaults = 8; //>>> taz::drop sources

		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml)
		{
			iImgUM = piml->Add(CTempIconLoader(L"UPLOAD"));
			iImgDM = piml->Add(CTempIconLoader(L"DOWNLOAD"));
			iImgDisplay = piml->Add(CTempIconLoader(L"DISPLAY"));
			iImgNotifications = piml->Add(CTempIconLoader(L"MESSAGEPENDING"));
			iImgQuickstart = piml->Add(CTempIconLoader(_T("QUICKSTART"))); // Thx to the eF-Mod team for the icon //>>> taz::Quick start [TPT]
			iImgDropDefaults = piml->Add(CTempIconLoader(L"DROPDEFAULTS")); //>>> taz::drop sources
		}

		/////////////////////////////////////////////////////////////////////////////
		// Upload Management group
		//

		m_htiUM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UM), iImgUM, TVI_ROOT);
		m_htiInformQueuedClientsAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_IQCAOC), m_htiUM, m_bInformQueuedClientsAfterIPChange); //>>> taz::Inform Clients after IP Change [Stulle]

		m_ctrlTreeOptions.SetItemState(m_htiUM, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.Expand(m_htiUM, TVE_EXPAND);

		/////////////////////////////////////////////////////////////////////////////
		// Download Management group
		//

		m_htiDM = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DM), iImgDM, TVI_ROOT);
//>>> taz::Quick start [TPT]
		m_htiQuickStartGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_QUICK_START_GROUP), iImgQuickstart, m_htiDM);
		// ==> Quick start [TPT] - Stulle
		m_htiQuickStart = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICK_START), m_htiQuickStartGroup, m_bQuickStart);
		m_htiQuickStartMaxTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnPerFive = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnPerFive, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConn = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConn, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiQuickStart, TVE_EXPAND);
		m_htiQuickStartAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICK_START_AFTER_IP_CHANGE), m_htiQuickStartGroup, m_bQuickStartAfterIPChange);
		// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
		// ==> drop sources - Stulle
		m_htiDropDefaults = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DROP_DEFAULTS), iImgDropDefaults, m_htiDM);
		m_htiAutoNNS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_NNS), m_htiDropDefaults, m_bEnableAutoDropNNSDefault);
		m_htiAutoNNSTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_NNS_TIMERLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoNNS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoNNSTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoNNSLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVENNSLIMITLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoNNS);
		m_ctrlTreeOptions.Expand(m_htiAutoNNS, TVE_EXPAND);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoNNSLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoFQS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_FQS), m_htiDropDefaults, m_bEnableAutoDropFQSDefault);
		m_htiAutoFQSTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_FQS_TIMERLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoFQS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoFQSTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoFQSLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVEFQSLIMITLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoFQS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoFQSLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoFQS, TVE_EXPAND);
		m_htiAutoQRS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTO_QRS), m_htiDropDefaults, m_bEnableAutoDropQRSDefault);
		m_htiAutoQRSTimer = m_ctrlTreeOptions.InsertItem(GetResString(IDS_HQRS_TIMERLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoQRS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoQRSTimer, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoQRSMax = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVEQRSLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoQRS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoQRSMax, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAutoQRSLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REMOVEQRSLIMITLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoQRS);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoQRSLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoQRS, TVE_EXPAND);
		// <== drop sources - Stulle
		m_htiDontDropCompleteSources = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DONTDROPCOMPLETESOURCES), m_htiDropDefaults, m_bDontDropCompleteSources); //>>> taz::don't drop complete sources
//<<< taz::drop sources
		m_htiIsreaskSourceAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RSAIC), m_htiDM, m_bIsreaskSourceAfterIPChange); //>>> taz::Inform Clients after IP Change [Stulle]
//>>> taz::Variable corrupted blocks ban threshold [Spike2]
		m_htiBanThreshold = m_ctrlTreeOptions.InsertItem(GetResString(IDS_BanThreshold), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDM);
		m_ctrlTreeOptions.AddEditBox(m_htiBanThreshold, RUNTIME_CLASS(CNumTreeOptionsEdit));
//<<< taz::Variable corrupted blocks ban threshold [Spike2]

		m_ctrlTreeOptions.SetItemState(m_htiDM, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.Expand(m_htiDM, TVE_EXPAND);

		/////////////////////////////////////////////////////////////////////////////
		// Display group
		//

		m_htiDisplayGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisplay, TVI_ROOT);

//>>> WiZaRd::minRQR [WiZaRd]
		m_htiMinRQR = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MINRQR_ENABLE), m_htiDisplayGroup, m_bUseMinRQR);
//<<< WiZaRd::minRQR [WiZaRd]
		m_htiTrayComplete = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_TRAY_COMPLETE), m_htiDisplayGroup, m_bTrayComplete); //>>> taz::Completed in Tray [Stulle]

		m_ctrlTreeOptions.SetItemState(m_htiDisplayGroup, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.Expand(m_htiDisplayGroup, TVE_EXPAND);

		/////////////////////////////////////////////////////////////////////////////
		// Notifications
		//

		m_htiNotifications = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_NOTIFICATIONS), iImgNotifications, TVI_ROOT);

//>>> taz::lowid notifier [chamblard]
		m_htiShowLowID = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ACTIVATELOWIDNOTIFIER), m_htiNotifications, m_bShowLowID);
//<<< taz::lowid notifier [chamblard]
//>>> taz::More info about corrupted .met/.part file
		m_htiShowCorrupted = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ACTIVATE_CORRUPTED), m_htiNotifications, m_bShowCorrupted);
//<<< taz::More info about corrupted .met/.part file

		m_ctrlTreeOptions.SetItemState(m_htiNotifications, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.Expand(m_htiNotifications, TVE_EXPAND);

		m_bInitializedTreeOpts = true;
	}
	/////////////////////////////////////////////////////////////////////////////
	// Upload Management group
	//
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiInformQueuedClientsAfterIPChange, m_bInformQueuedClientsAfterIPChange); //>>> taz::Inform Clients after IP Change [Stulle]

	/////////////////////////////////////////////////////////////////////////////
	// Download Management group
	//

//>>> taz::Quick start [TPT]
	// ==> Quick start [TPT] - Stulle
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiQuickStart, m_bQuickStart);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiQuickStartMaxTime, m_iQuickStartMaxTime);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxTime, 8, 18);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiQuickStartMaxConnPerFive, m_iQuickStartMaxConnPerFive);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnPerFive, 5, 200);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiQuickStartMaxConn, m_iQuickStartMaxConn);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConn, 200, 2000);
	if(m_htiQuickStartAfterIPChange) DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiQuickStartAfterIPChange, m_bQuickStartAfterIPChange);
	// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
	// ==> drop sources - Stulle
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiAutoNNS, m_bEnableAutoDropNNSDefault);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoNNSTimer, m_iAutoNNS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoNNS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoNNSLimit, m_iMaxRemoveNNSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveNNSLimitDefault, 50, 100);
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiAutoFQS, m_bEnableAutoDropFQSDefault);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoFQSTimer, m_iAutoFQS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoFQS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoFQSLimit, m_iMaxRemoveFQSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveFQSLimitDefault, 50, 100);
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiAutoQRS, m_bEnableAutoDropQRSDefault);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoQRSTimer, m_iAutoHQRS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoHQRS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoQRSMax, m_iMaxRemoveQRSDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveQRSDefault, 2500, 10000);
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiAutoQRSLimit, m_iMaxRemoveQRSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveQRSLimitDefault, 50, 100);
	// <== drop sources - Stulle
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiDontDropCompleteSources, m_bDontDropCompleteSources); //>>> taz::don't drop complete sources
//<<< taz::drop sources
//>>> taz::Variable corrupted blocks ban threshold [Spike2]
	DDX_TreeEdit(pDX, IDC_ACK_OPTS, m_htiBanThreshold, m_iBanThreshold);
	DDV_MinMaxInt(pDX, m_iBanThreshold, 10, 32);
//<<< taz::Variable corrupted blocks ban threshold [Spike2]
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiIsreaskSourceAfterIPChange, m_bIsreaskSourceAfterIPChange); //>>> taz::Inform Clients after IP Change [Stulle]

	/////////////////////////////////////////////////////////////////////////////
	// Display group
	//

//>>> WiZaRd::minRQR [WiZaRd]
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiMinRQR, m_bUseMinRQR);
//<<< WiZaRd::minRQR [WiZaRd]
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiTrayComplete, m_bTrayComplete); //>>> taz::Completed in Tray [Stulle]

	/////////////////////////////////////////////////////////////////////////////
	// Notifications
	//

//>>> taz::lowid notifier [chamblard]
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiShowLowID, m_bShowLowID);
//<<< taz::lowid notifier [chamblard]
//>>> taz::More info about corrupted .met/.part file
	DDX_TreeCheck(pDX, IDC_ACK_OPTS, m_htiShowCorrupted, m_bShowCorrupted);
//<<< taz::More info about corrupted .met/.part file
//>>> taz::Quick strat [TPT]
	// ==> Quick start [TPT] - Stulle
	if (m_htiQuickStartAfterIPChange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartAfterIPChange, m_bQuickStart);
	// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
}

BOOL CPPgAcK2::OnInitDialog()
{
//>>> taz::Quick start [TPT]
	// ==> Quick start [TPT] - Stulle
	m_bQuickStart = thePrefs.GetQuickStart();
	m_iQuickStartMaxTime = (int)(thePrefs.GetQuickStartMaxTime());
	m_iQuickStartMaxConnPerFive = (int)(thePrefs.GetQuickStartMaxConnPerFive());
	m_iQuickStartMaxConn = (int)(thePrefs.GetQuickStartMaxConn());
	m_bQuickStartAfterIPChange = thePrefs.GetQuickStartAfterIPChange();
	// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
	// ==> drop sources - Stulle
	m_bEnableAutoDropNNSDefault = thePrefs.m_EnableAutoDropNNSDefault;
	m_iAutoNNS_TimerDefault = (thePrefs.m_AutoNNS_TimerDefault/1000);
	m_iMaxRemoveNNSLimitDefault = thePrefs.m_MaxRemoveNNSLimitDefault;
	m_bEnableAutoDropFQSDefault = thePrefs.m_EnableAutoDropFQSDefault;
	m_iAutoFQS_TimerDefault = (thePrefs.m_AutoFQS_TimerDefault/1000);
	m_iMaxRemoveFQSLimitDefault = thePrefs.m_MaxRemoveFQSLimitDefault;
	m_bEnableAutoDropQRSDefault = thePrefs.m_EnableAutoDropQRSDefault;
	m_iAutoHQRS_TimerDefault = (thePrefs.m_AutoHQRS_TimerDefault/1000);
	m_iMaxRemoveQRSDefault = thePrefs.m_MaxRemoveQRSDefault;
	m_iMaxRemoveQRSLimitDefault = thePrefs.m_MaxRemoveQRSLimitDefault;
	// <== drop sources - Stulle
	m_bDontDropCompleteSources = thePrefs.DontDropCompleteSources(); //>>> taz::don't drop complete sources
//<<< taz::drop sources
//>>> taz::Inform Clients after IP Change [Stulle]
	m_bIsreaskSourceAfterIPChange = thePrefs.IsRASAIC();
	m_bInformQueuedClientsAfterIPChange = thePrefs.IsIQCAOC();
//<<< taz::Inform Clients after IP Change [Stulle]

	m_iBanThreshold = thePrefs.GetBANTHRES(); //>>> taz::Variable corrupted blocks ban threshold [Spike2]

//>>> WiZaRd::minRQR [WiZaRd
	m_bUseMinRQR = thePrefs.ShowMinRQR();
//<<< WiZaRd::minRQR [WiZaRd]
	m_bTrayComplete = thePrefs.GetTrayComplete(); //>>> taz::Completed in Tray [Stulle]

//>>> taz::lowid notifier [chamblard]
	m_bShowLowID = thePrefs.m_bShowLowID;
//<<< taz::lowid notifier [chamblard]
//>>> taz::More info about corrupted .met/.part file
	m_bShowCorrupted = thePrefs.m_bShowCorrupted;
//<<< taz::More info about corrupted .met/.part file

	CPropertyPage::OnInitDialog();
//	InitWindowStyles(this);
//	m_ctrlTreeOptions.SetItemHeight(m_ctrlTreeOptions.GetItemHeight() + 2);

	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgAcK2::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgAcK2::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

//>>> taz::Quick start [TPT]
	// ==> Quick start [TPT] - Stulle
	thePrefs.m_bQuickStart = m_bQuickStart;
	thePrefs.m_iQuickStartMaxTime = (uint16)m_iQuickStartMaxTime;
	thePrefs.m_iQuickStartMaxConnPerFive = (uint16)m_iQuickStartMaxConnPerFive;
	thePrefs.m_iQuickStartMaxConn = (UINT)m_iQuickStartMaxConn;
	thePrefs.m_bQuickStartAfterIPChange = m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
	// ==> drop sources - Stulle
	thePrefs.m_EnableAutoDropNNSDefault = m_bEnableAutoDropNNSDefault;
	thePrefs.m_AutoNNS_TimerDefault = (m_iAutoNNS_TimerDefault*1000);
	thePrefs.m_MaxRemoveNNSLimitDefault = (uint16)m_iMaxRemoveNNSLimitDefault;
	thePrefs.m_EnableAutoDropFQSDefault = m_bEnableAutoDropFQSDefault;
	thePrefs.m_AutoFQS_TimerDefault = (m_iAutoFQS_TimerDefault*1000);
	thePrefs.m_MaxRemoveFQSLimitDefault = (uint16)m_iMaxRemoveFQSLimitDefault;
	thePrefs.m_EnableAutoDropQRSDefault = m_bEnableAutoDropQRSDefault;
	thePrefs.m_AutoHQRS_TimerDefault = (m_iAutoHQRS_TimerDefault*1000);
	thePrefs.m_MaxRemoveQRSDefault = (uint16)m_iMaxRemoveQRSDefault;
	thePrefs.m_MaxRemoveQRSLimitDefault = (uint16)m_iMaxRemoveQRSLimitDefault;
	// <== drop sources - Stulle
	thePrefs.m_bDontDropCompleteSources = m_bDontDropCompleteSources;  //>>> taz::don't drop complete sources
//<<< taz::drop sources
//>>> taz::Inform Clients after IP Change [Stulle]
	thePrefs.m_breaskSourceAfterIPChange = m_bIsreaskSourceAfterIPChange;
	thePrefs.m_bInformQueuedClientsAfterIPChange = m_bInformQueuedClientsAfterIPChange;
//<<< taz::Inform Clients after IP Change [Stulle]

	thePrefs.SetBANTHRES((uint8) m_iBanThreshold); //>>> taz::Variable corrupted blocks ban threshold [Spike2]

//>>> WiZaRd::minRQR [WiZaRd]
	if(thePrefs.ShowMinRQR() != m_bUseMinRQR)
	{
		thePrefs.SetMinRQR(m_bUseMinRQR);
		theApp.emuledlg->transferwnd->GetDownloadList()->Localize();
	}
//<<< WiZaRd::minRQR [WiZaRd]
	thePrefs.SetTrayComplete(m_bTrayComplete); //>>> taz::Completed in Tray [Stulle]
//>>> taz::lowid notifier [chamblard]
	thePrefs.m_bShowLowID = m_bShowLowID;
//<<< taz::lowid notifier [chamblard]
//>>> taz::More info about corrupted .met/.part file
	thePrefs.m_bShowCorrupted = m_bShowCorrupted;
//<<< taz::More info about corrupted .met/.part file

	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

void CPPgAcK2::Localize(void)
{
	if (m_hWnd)
	{
//>>> taz::AcK Links
		GetDlgItem(IDC_ACKHELP)->SetWindowText(GetResString(IDS_ACKHELP));
		GetDlgItem(IDC_ACKHELP2)->SetWindowText(GetResString(IDS_ACKHELP2)); //>>> taz:: fix Restore 3 button-link now pointing to AcKroNiC WiKi [Mulo da Soma]
		GetDlgItem(IDC_ACKHELP3)->SetWindowText(GetResString(IDS_ACKHELP3));
//<<< taz::AcK Links

		if (m_htiUM) m_ctrlTreeOptions.SetItemText(m_htiDM, GetResString(IDS_UM));
		if (m_htiInformQueuedClientsAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiInformQueuedClientsAfterIPChange, GetResString(IDS_IQCAOC)); //>>> taz::Inform Clients after IP Change [Stulle]

		if (m_htiDM) m_ctrlTreeOptions.SetItemText(m_htiDM, GetResString(IDS_DM));
//>>> taz::Quick start [TPT]
		// ==> Quick start [TPT] - Stulle
		if (m_htiQuickStart) m_ctrlTreeOptions.SetItemText(m_htiQuickStart, GetResString(IDS_QUICK_START));
		if (m_htiQuickStartMaxTime) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxTime, GetResString(IDS_QUICK_START_MAX_TIME));
		if (m_htiQuickStartMaxConnPerFive) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnPerFive, GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE));
		if (m_htiQuickStartMaxConn) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConn, GetResString(IDS_QUICK_START_MAX_CONN));
		if (m_htiQuickStartAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiQuickStartAfterIPChange, GetResString(IDS_QUICK_START_AFTER_IP_CHANGE));
		// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
		// ==> drop sources - Stulle
		if (m_htiAutoNNS) m_ctrlTreeOptions.SetItemText(m_htiAutoNNS, GetResString(IDS_AUTO_NNS));
		if (m_htiAutoNNSTimer) m_ctrlTreeOptions.SetEditLabel(m_htiAutoNNSTimer, GetResString(IDS_NNS_TIMERLABEL));
		if (m_htiAutoNNSLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAutoNNSLimit, GetResString(IDS_REMOVENNSLIMITLABEL));
		if (m_htiAutoFQS) m_ctrlTreeOptions.SetItemText(m_htiAutoFQS, GetResString(IDS_AUTO_FQS));
		if (m_htiAutoFQSTimer) m_ctrlTreeOptions.SetEditLabel(m_htiAutoFQSTimer, GetResString(IDS_FQS_TIMERLABEL));
		if (m_htiAutoFQSLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAutoFQSLimit, GetResString(IDS_REMOVEFQSLIMITLABEL));
		if (m_htiAutoQRS) m_ctrlTreeOptions.SetItemText(m_htiAutoQRS, GetResString(IDS_AUTO_QRS));
		if (m_htiAutoQRSTimer) m_ctrlTreeOptions.SetEditLabel(m_htiAutoQRSTimer, GetResString(IDS_HQRS_TIMERLABEL));
		if (m_htiAutoQRSMax) m_ctrlTreeOptions.SetEditLabel(m_htiAutoQRSMax, GetResString(IDS_REMOVEQRSLABEL));
		if (m_htiAutoQRSLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAutoQRSLimit, GetResString(IDS_REMOVEQRSLIMITLABEL));
		// <== drop sources - Stulle
		if (m_htiDontDropCompleteSources) m_ctrlTreeOptions.SetEditLabel(m_htiDontDropCompleteSources, GetResString(IDS_DONTDROPCOMPLETESOURCES)); //>>> taz::don't drop complete sources
//<<< taz::drop sources
		if (m_htiIsreaskSourceAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiIsreaskSourceAfterIPChange, GetResString(IDS_RSAIC)); //>>> taz::Inform Clients after IP Change [Stulle]

		if (m_htiBanThreshold) m_ctrlTreeOptions.SetEditLabel(m_htiBanThreshold, GetResString(IDS_BanThreshold)); //>>> taz::Variable corrupted blocks ban threshold [Spike2]

		if(m_htiDisplayGroup) m_ctrlTreeOptions.SetItemText(m_htiDisplayGroup, GetResString(IDS_PW_DISPLAY));
//>>> WiZaRd::minRQR [WiZaRd]
		if(m_htiMinRQR) m_ctrlTreeOptions.SetItemText(m_htiMinRQR, GetResString(IDS_MINRQR_ENABLE));
//<<< WiZaRd::minRQR [WiZaRd]
		if (m_htiTrayComplete) m_ctrlTreeOptions.SetItemText(m_htiTrayComplete, GetResString(IDS_TRAY_COMPLETE)); // Completed in Tray - Stulle

		if (m_htiNotifications) m_ctrlTreeOptions.SetItemText(m_htiNotifications, GetResString(IDS_NOTIFICATIONS));
//>>> taz::lowid notifier [chamblard]
		if(m_htiShowLowID) m_ctrlTreeOptions.SetItemText(m_htiShowLowID, GetResString(IDS_ACTIVATELOWIDNOTIFIER));
//<<< taz::lowid notifier [chamblard]
//>>> taz::More info about corrupted .met/.part file
		if(m_htiShowCorrupted) m_ctrlTreeOptions.SetItemText(m_htiShowCorrupted, GetResString(IDS_ACTIVATE_CORRUPTED));
//<<< taz::More info about corrupted .met/.part file
	}
}

//>>> taz::AcK Links
void CPPgAcK2::OnBnClickedAckHelp()
{
	TCHAR link[128];//Mulo da Soma - add more blanks in text-area
	_tcscpy (link, L"http://www.ackronic.net/ackws/smf/index.php?cat=105");//Mulo da Soma - now link points to new guide link
	ShellExecute(NULL, NULL, link, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
}
void CPPgAcK2::OnBnClickedAckHelp2()
{
	TCHAR link[128];//Add by Mulo da Soma - new text-area about 3 link-button
	_tcscpy (link, L"http://wiki.darkforge.eu/wiki/index.php?title=EMule_AcKroNiC_Wiki_ITA");//added by Mulo da Soma - restore 3 link-button pointing to AcKroNiC WiKi
	ShellExecute(NULL, NULL, link, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
}
void CPPgAcK2::OnBnClickedAckHelp3()
{
	TCHAR link[128];//Mulo da Soma - add more blanks in text-area
	_tcscpy (link, L"http://www.ackronic.net/ackws/smf/index.php?board=40.0");//changed by Mulo da Soma - now link points to AcKroNiC forum
	ShellExecute(NULL, NULL, link, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
}
//<<< taz::AcK Links

void CPPgAcK2::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	Nullify();

	CPropertyPage::OnDestroy();
}

LRESULT CPPgAcK2::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_ACK_OPTS)
	{
		//TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		//(void*) pton;

//>>> taz::Quick start [TPT]
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;

	BOOL bCheck;

		// ==> Quick start [TPT] - Stulle
		if (m_htiQuickStart && pton->hItem == m_htiQuickStart)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiQuickStart, bCheck))
			{
				if (m_htiQuickStartAfterIPChange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartAfterIPChange, bCheck);
			}
		}
		// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]

		SetModified();
	}
	return 0;
}

void CPPgAcK2::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgAcK2::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgAcK2::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void	CPPgAcK2::Nullify()
{
	m_htiDM = NULL;
	m_htiUM = NULL;
	m_htiDisplayGroup = NULL;
	m_htiNotifications = NULL;
//>>> taz::Quick start [TPT]
	// ==> Quick start [TPT] - Stulle
	m_htiQuickStartGroup = NULL;
	m_htiQuickStart = NULL;
	m_htiQuickStartMaxTime = NULL;
	m_htiQuickStartMaxConnPerFive = NULL;
	m_htiQuickStartMaxConn = NULL;
	m_htiQuickStartAfterIPChange = NULL;
	// <== Quick start [TPT] - Stulle
//<<< taz::Quick start [TPT]
//>>> taz::drop sources
	// ==> drop sources - Stulle
	m_htiDropDefaults = NULL;
	m_htiAutoNNS = NULL;
	m_htiAutoNNSTimer = NULL;
	m_htiAutoNNSLimit = NULL;
	m_htiAutoFQS = NULL;
	m_htiAutoFQSTimer = NULL;
	m_htiAutoFQSLimit = NULL;
	m_htiAutoQRS = NULL;
	m_htiAutoQRSTimer = NULL;
	m_htiAutoQRSMax = NULL;
	m_htiAutoQRSLimit = NULL;
	// <== drop sources - Stulle
	m_htiDontDropCompleteSources = NULL; //>>> taz::don't drop complete sources
//<<< taz::drop sources
//>>> taz::Inform Clients after IP Change [Stulle]
	m_htiIsreaskSourceAfterIPChange = NULL;
	m_htiInformQueuedClientsAfterIPChange = NULL;
//<<< taz::Inform Clients after IP Change [Stulle]

	m_htiBanThreshold = NULL; //>>> taz::Variable corrupted blocks ban threshold [Spike2]

//>>> WiZaRd::minRQR [WiZaRd]
	m_htiMinRQR = NULL;
	m_bUseMinRQR = false;
//<<< WiZaRd::minRQR [WiZaRd]

//>>> taz::Completed in Tray [Stulle]
	m_htiTrayComplete = NULL;
//<<< taz::Completed in Tray [Stulle]

//>>> taz::lowid notifier [chamblard]
	m_htiShowLowID = NULL;
	m_bShowLowID = false;
//<<< taz::lowid notifier [chamblard]

//>>> taz::More info about corrupted .met/.part file
	m_htiShowCorrupted = NULL;
	m_bShowCorrupted = false;
//<<< taz::More info about corrupted .met/.part file
}