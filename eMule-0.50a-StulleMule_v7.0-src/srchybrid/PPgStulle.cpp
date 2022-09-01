// PpgStulle.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgStulle.h"
#include "emuledlg.h"
#include "UserMsgs.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "MuleToolbarCtrl.h" // TBH: minimule - Stulle
#include "log.h"
#include "DownloadQueue.h" // Global Source Limit - Stulle
#include "ClientList.h" // Reduce Score for leecher - Stulle
#include "TransferDlg.h" // CPU/MEM usage [$ick$/Stulle] - Stulle
#include "MuleToolbarCtrl.h" // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CPPgStulle dialog

IMPLEMENT_DYNAMIC(CPPgStulle, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgStulle, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgStulle::CPPgStulle()
	: CPropertyPage(CPPgStulle::IDD)
	
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	m_htiSecu = NULL;
	// ==> Sivka-Ban - Stulle
	m_htiSivkaBanGroup = NULL;
	m_htiEnableSivkaBan = NULL;
	m_htiSivkaAskTime = NULL;
	m_htiSivkaAskCounter = NULL;
	m_htiSivkaAskLog = NULL;
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	m_htiAntiLeecherGroup = NULL;
	m_htiEnableAntiLeecher = NULL; //MORPH - Added by IceCream, activate Anti-leecher
	m_htiBadModString = NULL;
	m_htiBadNickBan = NULL;
	m_htiGhostMod = NULL;
	m_htiAntiModIdFaker = NULL;
	m_htiAntiNickThief = NULL; // AntiNickThief - Stulle
	m_htiEmptyNick = NULL;
	m_htiFakeEmule = NULL;
	m_htiLeecherName = NULL;
	m_htiCommunityCheck = NULL;
	m_htiHexCheck = NULL;
	m_htiEmcrypt = NULL;
	m_htiBadInfo = NULL;
	m_htiBadHello = NULL;
	m_htiSnafu = NULL;
	m_htiExtraBytes = NULL;
	m_htiNickChanger = NULL;
	m_htiFileFaker = NULL;
	m_htiVagaa = NULL;
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	m_iReduceScore = (thePrefs.IsReduceScore()) ? 1 : 0;
	m_htiPunishmentGroup = NULL;
	m_htiBanAll = NULL;
	m_htiReduce = NULL;
	m_htiReduceFactor = NULL;
	// <== Reduce Score for leecher - Stulle
	m_htiNoBadPushing = NULL; // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	m_htiEnableAntiCreditHack = NULL; //MORPH - Added by IceCream, activate Anti-CreditHack
	m_htiAntiXsExploiter = NULL; // Anti-XS-Exploit [Xman] - Stulle
	m_htiSpamBan = NULL; // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiFilterClientFailedDown = NULL; 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiClientBanTime = NULL; // adjust ClientBanTime - Stulle

	m_htiPush = NULL; // push files - Stulle
	// ==> push small files [sivka] - Stulle
	m_htiEnablePushSmallFile = NULL;
	m_iPushSmallFiles = 0;
	m_htiPushSmallFileBoost = NULL;
	// <== push small files [sivka] - Stulle
	m_htiEnablePushRareFile = NULL;  // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick Tag - Stulle

	m_htiConTweaks = NULL;
	// ==> Quick start [TPT] - Stulle
	m_htiQuickStartGroup = NULL;
	m_htiQuickStart = NULL;
	m_htiQuickStartMaxTime = NULL;
	m_htiQuickStartMaxConnPerFive = NULL;
	m_htiQuickStartMaxConn = NULL;
	m_htiQuickStartMaxConnPerFiveBack = NULL;
	m_htiQuickStartMaxConnBack = NULL;
	m_htiQuickStartAfterIPChange = NULL;
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	m_htiCheckConGroup = NULL;
	m_htiCheckCon = NULL;
	m_htiICMP = NULL;
	m_htiPingTimeOut = NULL;
	m_htiPingTTL = NULL;
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	m_htiRatioGroup = NULL;
	m_htiEnforceRatio = NULL;
	m_htiRatioValue = NULL;
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	m_htiIsreaskSourceAfterIPChange = NULL;
	m_htiInformQueuedClientsAfterIPChange = NULL;
	// <== Inform Clients after IP Change - Stulle
	m_htiReAskFileSrc = NULL; // Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban - Stulle

	m_htiDisplay = NULL;
	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	m_htiSysInfoGroup = NULL;
	m_htiSysInfo = NULL;
	m_htiSysInfoGlobal = NULL;
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	m_htiShowSrcOnTitle = NULL; // Show sources on title - Stulle
	m_htiShowGlobalHL = NULL; // show global HL - Stulle
	m_htiShowFileHLconst = NULL; // show HL per file constantly
	m_htiShowInMSN7 = NULL; // Show in MSN7 [TPT] - Stulle
	m_htiTrayComplete = NULL; // Completed in Tray - Stulle
	m_htiShowSpeedMeter = NULL; // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

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

	// ==> TBH: minimule - Stulle
	m_htiMMGroup = NULL;
	m_htiShowMM = NULL;
	m_htiMMLives = NULL;
	m_htiMMUpdateTime = NULL;
	m_htiMMTrans = NULL;
	m_htiMMCompl = NULL;
	m_htiMMOpen = NULL;
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	m_htiAutoDownPrioGroup = NULL;
	m_htiAutoDownPrioOff = NULL;
	m_htiAutoDownPrioPerc = NULL;
	m_htiAutoDownPrioPercVal = NULL;
	m_htiAutoDownPrioSize = NULL;
	m_htiAutoDownPrioSizeVal = NULL;
	m_htiAutoDownPrioValGroup = NULL;
	m_htiAutoDownPrioLow = NULL;
	m_htiAutoDownPrioNormal = NULL;
	m_htiAutoDownPrioHigh = NULL;
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	m_htiMisc = NULL;
	// ==> Spread Credits Slot - Stulle
	m_htiSpreadCreditsSlotGroup = NULL;
	m_htiSpreadCreditsSlot = NULL;
	m_htiSpreadCreditsSlotCounter = NULL;
	m_htiSpreadCreditsSlotPS = NULL;
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	m_htiGlobalHlGroup = NULL;
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	m_htiGlobalHlAll = NULL;
	m_htiGlobalHlDefault = NULL;
	// <== Global Source Limit - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	m_htiEmulatorGroup = NULL;
	m_htiEmuMLDonkey = NULL;
	m_htiEmueDonkey = NULL;
	m_htiEmueDonkeyHybrid = NULL;
	m_htiEmuShareaza = NULL;
	m_htiEmuLphant = NULL;
	m_htiLogEmulator = NULL;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	m_htiReleaseBonusGroup = NULL;
	m_htiReleaseBonus0 = NULL;
	m_htiReleaseBonus1 = NULL;
	m_htiReleaseBonusDays = NULL;
	m_htiReleaseBonusDaysEdit = NULL;
	// <== Release Bonus [sivka] - Stulle
	m_htiReleaseScoreAssurance = NULL; // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	m_htiAutoSharedGroup = NULL;
	m_htiAutoSharedUpdater = NULL;
	m_htiSingleSharedDirUpdater = NULL;
	m_htiTimeBetweenReloads = NULL;
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle
}

CPPgStulle::~CPPgStulle()
{
}

void CPPgStulle::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STULLE_OPTS, m_ctrlTreeOptions);
	DDX_Control(pDX, IDC_PUSHSMALL_SLIDER, m_ctlPushSmallSize); // push small files [sivka] - Stulle
	if (!m_bInitializedTreeOpts)
	{
		int iImgSecu = 8;
		int iImgPunishment = 8;
		int iImgPush = 8;
		int iImgFunnyNick = 8;
		int iImgConTweaks = 8;
		int iImgQuickstart = 8;
		int iImgPinger = 8;
		int iImgRatio = 8;
		int iImgReconnect = 8;
		int iImgDisplay = 8;
		int iImgSysInfo = 8;
		int iImgDropDefaults = 8;
		int iImgMinimule = 8;
		int iImgAutoDownPrio = 8;
		int iImgPriority = 8;
		int iImgMisc = 8;
		int iImgSpreadCredits = 8;
		int iImgGlobal = 8;
		int iImgEmulate = 8;
		int iImgReleaseBonus = 8;
		int iImgServiceStrGrp = 8;
		int iImgASFU = 8;

		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgSecu = piml->Add(CTempIconLoader(_T("SECURITY")));
			iImgPunishment = piml->Add(CTempIconLoader(_T("RATING_FAKE")));
			iImgPush = piml->Add(CTempIconLoader(_T("SPEED")));
			iImgFunnyNick = piml->Add(CTempIconLoader(_T("FUNNYNICK")));
			iImgConTweaks =  piml->Add(CTempIconLoader(_T("CONNECTION")));
			iImgQuickstart = piml->Add(CTempIconLoader(_T("QUICKSTART"))); // Thx to the eF-Mod team for the icon
			iImgPinger = piml->Add(CTempIconLoader(_T("PINGER")));
			iImgRatio = piml->Add(CTempIconLoader(_T("TRANSFERUPDOWN")));
			iImgReconnect = piml->Add(CTempIconLoader(_T("WIZARD")));
			iImgDisplay = piml->Add(CTempIconLoader(_T("DISPLAY")));
			iImgSysInfo = piml->Add(CTempIconLoader(_T("STATISTICS")));
			iImgDropDefaults = piml->Add(CTempIconLoader(_T("DROPDEFAULTS")));
			iImgMinimule = piml->Add(CTempIconLoader(_T("MINIMULE")));
			iImgAutoDownPrio = piml->Add(CTempIconLoader(_T("DOWNLOAD")));
			iImgPriority = piml->Add(CTempIconLoader(_T("FILEPRIORITY")));
			iImgMisc = piml->Add(CTempIconLoader(_T("SRCUNKNOWN")));
			iImgSpreadCredits = piml->Add(CTempIconLoader(_T("SPREADCREDITS")));
			iImgGlobal = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL")));
			iImgEmulate = piml->Add(CTempIconLoader(_T("EMULATEICON")));
			iImgReleaseBonus = piml->Add(CTempIconLoader(_T("RELEASEBONUS")));
			iImgServiceStrGrp = piml->Add(CTempIconLoader(_T("TWEAK")));
			iImgASFU = piml->Add(CTempIconLoader(_T("SHAREDFILESLIST")));
		}
		
		CString Buffer;
		m_htiSecu = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SECURITY), iImgSecu, TVI_ROOT);
		// ==> Sivka-Ban - Stulle
		m_htiSivkaBanGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SIVKA_BAN_CONTROL), iImgSecu, m_htiSecu);
		m_htiEnableSivkaBan = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SIVKA_BAN),m_htiSivkaBanGroup, m_bEnableSivkaBan);
		m_htiSivkaAskTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SIVKA_ASK_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSivkaBanGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiSivkaAskTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSivkaAskCounter = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SIVKA_ASK_COUNTER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSivkaBanGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiSivkaAskCounter, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSivkaAskLog = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SIVKA_ASK_LOG), m_htiSivkaBanGroup, m_bSivkaAskLog);
		// <== Sivka-Ban - Stulle
		// ==> ban systems optional - Stulle
		m_htiAntiLeecherGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_ANTI_LEECHER_GROUP), iImgSecu, m_htiSecu);
		m_htiEnableAntiLeecher = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiAntiLeecherGroup, m_bEnableAntiLeecher); //MORPH - Added by IceCream, Enable Anti-leecher
		m_htiBadModString = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_BAD_MOD_STRING), m_htiAntiLeecherGroup, m_bBadModString);
		m_htiBadNickBan = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_BAD_NICK_BAN), m_htiAntiLeecherGroup, m_bBadNickBan);
		m_htiGhostMod = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_GHOST_MOD), m_htiAntiLeecherGroup, m_bGhostMod);
		m_htiAntiModIdFaker = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_MOD_FAKER), m_htiAntiLeecherGroup, m_bAntiModIdFaker);
		m_htiAntiNickThief = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_NICK_THIEF), m_htiAntiLeecherGroup, m_bAntiNickThief); // AntiNickThief - Stulle
		m_htiEmptyNick = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMPTY_NICK), m_htiAntiLeecherGroup, m_bEmptyNick);
		m_htiFakeEmule = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FAKE_EMULE), m_htiAntiLeecherGroup, m_bFakeEmule);
		m_htiLeecherName = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LEECHER_NAME), m_htiAntiLeecherGroup, m_bLeecherName);
		m_htiCommunityCheck = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COMMUNITY_CHECK), m_htiAntiLeecherGroup, m_bCommunityCheck);
		m_htiHexCheck = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_HEX_CHECK), m_htiAntiLeecherGroup, m_bHexCheck);
		m_htiEmcrypt = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMCRYPT), m_htiAntiLeecherGroup, m_bEmcrypt);
		m_htiBadInfo = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_BAD_INFO), m_htiAntiLeecherGroup, m_bBadInfo);
		m_htiBadHello = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_BAD_HELLO), m_htiAntiLeecherGroup, m_bBadHello);
		m_htiSnafu = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SNAFU), m_htiAntiLeecherGroup, m_bSnafu);
		m_htiExtraBytes = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EXTRA_BYTES), m_htiAntiLeecherGroup, m_bExtraBytes);
		m_htiNickChanger = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_NICK_CHANGER), m_htiAntiLeecherGroup, m_bNickChanger);
		m_htiFileFaker = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FILEFAKER), m_htiAntiLeecherGroup, m_bFileFaker);
		m_htiVagaa = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_VAGAA), m_htiAntiLeecherGroup, m_bVagaa);
		// ==> Reduce Score for leecher - Stulle
		m_htiPunishmentGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PUNISHMENT), iImgPunishment, m_htiAntiLeecherGroup);
		m_htiBanAll = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PUNISHMENT_BAN), m_htiPunishmentGroup, m_iReduceScore == 0);
		m_htiReduce = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PUNISHMENT_REDUCE), m_htiPunishmentGroup, m_iReduceScore == 1);
		m_htiReduceFactor = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PUNISHMENT_PERCENT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReduce);
		m_ctrlTreeOptions.AddEditBox(m_htiReduceFactor, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiPunishmentGroup, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiReduceFactor, TVE_EXPAND);
		// <== Reduce Score for leecher - Stulle
		m_htiNoBadPushing = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_NO_BAD_PUSHING), m_htiPunishmentGroup, m_bNoBadPushing); // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
		m_ctrlTreeOptions.Expand(m_htiEnableAntiLeecher, TVE_EXPAND);
		// <== ban systems optional - Stulle
		m_htiEnableAntiCreditHack = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_CREDITHACK), m_htiSecu, m_bEnableAntiCreditHack); //MORPH - Added by IceCream, Enable Anti-CreditHack
		m_htiAntiXsExploiter = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_XS_EXPLOITER), m_htiSecu, m_bAntiXsExploiter); // Anti-XS-Exploit [Xman] - Stulle
		m_htiSpamBan = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPAM_BAN), m_htiSecu, m_bSpamBan); // Spam Ban [Xman] - Stulle
		//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
		m_htiFilterClientFailedDown = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FILTER_CLIENTFAILEDDOWN), m_htiSecu, m_bFilterClientFailedDown); 
		//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
		// ==> adjust ClientBanTime - Stulle
		m_htiClientBanTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_CLIENT_BAN_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSecu);
		m_ctrlTreeOptions.AddEditBox(m_htiClientBanTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== adjust ClientBanTime - Stulle
		m_ctrlTreeOptions.Expand(m_htiSecu, TVE_EXPAND);
		
		m_htiPush = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PUSH), iImgPush, TVI_ROOT); // push files - Stulle
		// ==> push small files [sivka] - Stulle
		m_htiEnablePushSmallFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_SMALL), m_htiPush, m_bEnablePushSmallFile);
		m_htiPushSmallFileBoost = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PUSH_SMALL_BOOST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiPush);
		m_ctrlTreeOptions.AddEditBox(m_htiPushSmallFileBoost, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== push small files [sivka] - Stulle
		m_htiEnablePushRareFile = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_RARE), m_htiPush, m_bEnablePushRareFile); // push rare file - Stulle

		// ==> FunnyNick Tag - Stulle
		m_htiFnTag = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FN_TAG), iImgFunnyNick, TVI_ROOT);
		m_htiNoTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_TAG), m_htiFnTag, m_iFnTag == 0);
		m_htiShortTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHORT_TAG), m_htiFnTag, m_iFnTag == 1);
		m_htiFullTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_FULL_TAG), m_htiFnTag, m_iFnTag == 2);
		m_htiCustomTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CUSTOM_TAG),m_htiFnTag,m_iFnTag == 3);
		m_htiFnCustomTag = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SET_CUSTOM_TAG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCustomTag);
		m_ctrlTreeOptions.AddEditBox(m_htiFnCustomTag, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiFnTagAtEnd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FN_TAG_AT_END), m_htiFnTag, m_bFnTagAtEnd);
		m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_EXPAND);
		// <== FunnyNick Tag - Stulle

		m_htiConTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CON_TWEAKS), iImgConTweaks, TVI_ROOT);
		m_htiQuickStartGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_QUICK_START_GROUP), iImgQuickstart, m_htiConTweaks);
		// ==> Quick start [TPT] - Stulle
		m_htiQuickStart = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICK_START), m_htiQuickStartGroup, m_bQuickStart);
		m_htiQuickStartMaxTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnPerFive = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnPerFive, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConn = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConn, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnPerFiveBack = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE_BACK), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnPerFiveBack, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartMaxConnBack = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICK_START_MAX_CONN_BACK), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartMaxConnBack, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiQuickStart, TVE_EXPAND);
		m_htiQuickStartAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICK_START_AFTER_IP_CHANGE), m_htiQuickStartGroup, m_bQuickStartAfterIPChange);
		// <== Quick start [TPT] - Stulle
		// ==> Connection Checker [eWombat/WiZaRd] - Stulle
		m_htiCheckConGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CONCHECK), iImgPinger, m_htiConTweaks);
		m_htiCheckCon = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiCheckConGroup, m_bCheckCon);
		m_htiICMP = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE_ICMP), m_htiCheckConGroup, m_bICMP);
		m_htiPingTimeOut = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PING_TIME_OUT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCheckConGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiPingTimeOut, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPingTTL = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PING_TTL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCheckConGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiPingTTL, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Connection Checker [eWombat/WiZaRd] - Stulle
		// ==> Enforce Ratio - Stulle
		m_htiRatioGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RATIO_GROUP), iImgRatio, m_htiConTweaks);
		m_htiEnforceRatio = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENFORCE_RATIO), m_htiRatioGroup, m_bEnforceRatio);
		m_htiRatioValue = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RATIO_VALUE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiRatioGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiRatioValue, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Enforce Ratio - Stulle
		// ==> Inform Clients after IP Change - Stulle
		m_htiIsreaskSourceAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RSAIC), m_htiConTweaks, m_bIsreaskSourceAfterIPChange);
		m_htiInformQueuedClientsAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_IQCAOC), m_htiConTweaks, m_bInformQueuedClientsAfterIPChange);
		// <== Inform Clients after IP Change - Stulle
		// ==> Timer for ReAsk File Sources - Stulle
		m_htiReAskFileSrc = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REASK_FILE_SRC), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiConTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiReAskFileSrc, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Timer for ReAsk File Sources - Stulle

		// ==> Anti Uploader Ban - Stulle
		m_htiAntiUploaderBanLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UNBAN_UPLOADER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiAntiUploaderBanLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAntiCase1 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_1), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 0);
		m_htiAntiCase2 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_2), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 1);
		m_htiAntiCase3 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_3), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 2);
		// <== Anti Uploader Ban - Stulle

		m_htiDisplay = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PW_DISPLAY), iImgDisplay, TVI_ROOT);
		// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
		m_htiSysInfoGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SYS_INFO_GROUP), iImgSysInfo, m_htiDisplay);
		m_htiSysInfo = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiSysInfoGroup, m_bSysInfo);
		m_htiSysInfoGlobal = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SYS_INFO_GLOBAL), m_htiSysInfoGroup, m_bSysInfoGlobal);
		// <== CPU/MEM usage [$ick$/Stulle] - Stulle
		m_htiShowSrcOnTitle = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWSRCONTITLE), m_htiDisplay, showSrcInTitle); // Show sources on title - Stulle
		m_htiShowGlobalHL = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_GLOBAL_HL), m_htiDisplay, m_bShowGlobalHL); // show global HL - Stulle
		m_htiShowFileHLconst = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_FILE_HL_CONST), m_htiDisplay, m_bShowFileHLconst); // show HL per file constantly
		m_htiShowInMSN7 = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWINMSN7), m_htiDisplay, m_bShowInMSN7); // Show in MSN7 [TPT] - Stulle
		m_htiTrayComplete = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_TRAY_COMPLETE), m_htiDisplay, m_bTrayComplete); // Completed in Tray - Stulle
		m_htiShowSpeedMeter = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOW_SPEED_METER), m_htiDisplay, m_bShowSpeedMeter); // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

		// ==> drop sources - Stulle
		m_htiDropDefaults = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DROP_DEFAULTS), iImgDropDefaults, TVI_ROOT);
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

		// ==> TBH: minimule - Stulle
		m_htiMMGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MM_GROUP), iImgMinimule, TVI_ROOT);
		m_htiShowMM = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_SHOW), m_htiMMGroup, m_bShowMM);
		m_htiMMLives = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_LIVES), m_htiMMGroup, m_bMMLives);
		m_htiMMUpdateTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MM_UPDATE_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMMGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMMUpdateTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMMTrans = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MM_TRANS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiMMGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMMTrans, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMMCompl = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_COMPL), m_htiMMGroup, m_bMMCompl);
		m_htiMMOpen = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MM_OPEN), m_htiMMGroup, m_bMMOpen);
		// <== TBH: minimule - Stulle

		// ==> Control download priority [tommy_gun/iONiX] - Stulle
		m_htiAutoDownPrioGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTO_DOWN_GROUP), iImgAutoDownPrio, TVI_ROOT);
		m_htiAutoDownPrioOff = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiAutoDownPrioGroup, m_iAutoDownPrio == 0);
		m_htiAutoDownPrioPerc = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_AUTO_DOWN_PERC), m_htiAutoDownPrioGroup, m_iAutoDownPrio == 1);
		m_htiAutoDownPrioPercVal = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AUTO_DOWN_PERC_VAL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoDownPrioPerc);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoDownPrioPercVal, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoDownPrioPerc, TVE_EXPAND);
		m_htiAutoDownPrioSize = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_AUTO_DOWN_SIZE), m_htiAutoDownPrioGroup, m_iAutoDownPrio == 2);
		m_htiAutoDownPrioSizeVal = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AUTO_DOWN_SIZE_VAL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoDownPrioSize);
		m_ctrlTreeOptions.AddEditBox(m_htiAutoDownPrioSizeVal, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiAutoDownPrioSize, TVE_EXPAND);
		m_htiAutoDownPrioValGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTO_DOWN_PRIO), iImgPriority, m_htiAutoDownPrioGroup);
		m_htiAutoDownPrioLow = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIOLOW), m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal == 0);
		m_htiAutoDownPrioNormal = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIONORMAL), m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal == 1);
		m_htiAutoDownPrioHigh = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PRIOHIGH), m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal == 2);
		m_ctrlTreeOptions.Expand(m_htiAutoDownPrioValGroup, TVE_EXPAND);
		// <== Control download priority [tommy_gun/iONiX] - Stulle

		m_htiMisc = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MISC), iImgMisc, TVI_ROOT);
		// ==> Spread Credits Slot - Stulle
		m_htiSpreadCreditsSlotGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SPREAD_CREDITS_SLOT), iImgSpreadCredits, m_htiMisc);
		m_htiSpreadCreditsSlot = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiSpreadCreditsSlotGroup, m_bSpreadCreditsSlot);
		m_htiSpreadCreditsSlotCounter = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SPREAD_CREDITS_SLOT_COUNTER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSpreadCreditsSlotGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiSpreadCreditsSlotCounter, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSpreadCreditsSlotPS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPREAD_CREDITS_SLOT_PS), m_htiSpreadCreditsSlotGroup, m_bSpreadCreditsSlotPS);
		// <== Spread Credits Slot - Stulle
		// ==> Global Source Limit - Stulle
		m_htiGlobalHlGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_GLOBAL_HL), iImgGlobal, m_htiMisc);
		m_htiGlobalHL = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiGlobalHlGroup, m_bGlobalHL);
		m_htiGlobalHlLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_GLOBAL_HL_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiGlobalHlGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiGlobalHlLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiGlobalHlAll = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_GLOBAL_HL_ALL), m_htiGlobalHlGroup, m_bGlobalHlAll);
		m_htiGlobalHlDefault = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_GLOBAL_HL_DEFAULT), m_htiGlobalHlGroup, m_bGlobalHlDefault);
		m_ctrlTreeOptions.Expand(m_htiGlobalHlGroup, TVE_EXPAND);
		// <== Global Source Limit - Stulle
		// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		m_htiEmulatorGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EMULATOR_GROUP), iImgEmulate, m_htiMisc);
		m_htiEmuMLDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_ML), m_htiEmulatorGroup, m_bEmuMLDonkey);
		m_htiEmueDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_DONK), m_htiEmulatorGroup, m_bEmueDonkey);
		m_htiEmueDonkeyHybrid = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_DONK_HYB), m_htiEmulatorGroup, m_bEmueDonkeyHybrid);
		m_htiEmuShareaza = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_SHA), m_htiEmulatorGroup, m_bEmuShareaza);
		m_htiEmuLphant = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_PHANT), m_htiEmulatorGroup, m_bEmuLphant);
		m_htiLogEmulator = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_LOG), m_htiEmulatorGroup, m_bLogEmulator);
		// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		// ==> Release Bonus [sivka] - Stulle
		m_htiReleaseBonusGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RELEASE_BONUS_GROUP), iImgReleaseBonus, m_htiMisc);
		m_htiReleaseBonus0 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_POWERSHARE_DISABLED), m_htiReleaseBonusGroup, m_iReleaseBonus == 0);
		m_htiReleaseBonus1 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_RELEASE_BONUS_12), m_htiReleaseBonusGroup, m_iReleaseBonus == 1);
		m_htiReleaseBonusDays = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DAYS2), m_htiReleaseBonusGroup, m_iReleaseBonus == 2);
		m_htiReleaseBonusDaysEdit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RELEASE_BONUS_EDIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReleaseBonusDays);
		m_ctrlTreeOptions.AddEditBox(m_htiReleaseBonusDaysEdit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_ctrlTreeOptions.Expand(m_htiReleaseBonusDays, TVE_EXPAND);
		// <== Release Bonus [sivka] - Stulle
		m_htiReleaseScoreAssurance = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RELEASE_SCORE_ASSURANCE), m_htiReleaseBonusGroup, m_bReleaseScoreAssurance); // Release Score Assurance - Stulle
		// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
		m_htiAutoSharedGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTO_SHARED_UPDATER), iImgASFU, m_htiMisc);
		m_htiAutoSharedUpdater = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SUC_ENABLED), m_htiAutoSharedGroup, m_bAutoSharedUpdater);
		m_htiSingleSharedDirUpdater = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ASFU_SINGLE), m_htiAutoSharedGroup, m_bSingleSharedDirUpdater);
		m_htiTimeBetweenReloads = m_ctrlTreeOptions.InsertItem(GetResString(IDS_ASFU_TIMEBETWEEN), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoSharedGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiTimeBetweenReloads, RUNTIME_CLASS(CNumTreeOptionsEdit));
#endif
		// <== Automatic shared files updater [MoNKi] - Stulle

		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}
	// ==> Sivka-Ban - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEnableSivkaBan, m_bEnableSivkaBan);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiSivkaAskTime, m_iSivkaAskTime);
	DDV_MinMaxInt(pDX, m_iSivkaAskTime, 5, 10);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiSivkaAskCounter, m_iSivkaAskCounter);
	DDV_MinMaxInt(pDX, m_iSivkaAskCounter, 5, 15);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSivkaAskLog, m_bSivkaAskLog);
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEnableAntiLeecher, m_bEnableAntiLeecher); //MORPH - Added by IceCream, enable Anti-leecher
	if(m_htiBadModString)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiBadModString, m_bBadModString);
	if(m_htiBadNickBan)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiBadNickBan, m_bBadNickBan);
	if(m_htiGhostMod)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiGhostMod, m_bGhostMod);
	if(m_htiAntiModIdFaker)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAntiModIdFaker, m_bAntiModIdFaker);
	if(m_htiAntiNickThief)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAntiNickThief, m_bAntiNickThief); // AntiNickThief - Stulle
	if(m_htiEmptyNick)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmptyNick, m_bEmptyNick);
	if(m_htiFakeEmule)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiFakeEmule, m_bFakeEmule);
	if(m_htiLeecherName)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiLeecherName, m_bLeecherName);
	if(m_htiCommunityCheck)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiCommunityCheck, m_bCommunityCheck);
	if(m_htiHexCheck)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiHexCheck, m_bHexCheck);
	if(m_htiEmcrypt)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmcrypt, m_bEmcrypt);
	if(m_htiBadInfo)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiBadInfo, m_bBadInfo);
	if(m_htiBadHello)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiBadHello, m_bBadHello);
	if(m_htiSnafu)			DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSnafu, m_bSnafu);
	if(m_htiExtraBytes)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiExtraBytes, m_bExtraBytes);
	if(m_htiNickChanger)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiNickChanger, m_bNickChanger);
	if(m_htiFileFaker)		DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiFileFaker, m_bFileFaker);
	if(m_htiVagaa)			DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiVagaa, m_bVagaa);
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	if(m_htiPunishmentGroup) DDX_TreeRadio(pDX, IDC_STULLE_OPTS, m_htiPunishmentGroup, (int &)m_iReduceScore);;
	if(m_htiReduceFactor)	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiReduceFactor, m_iReduceFactor);
	DDV_MinMaxInt(pDX, m_iReduceFactor, 10, 100);
	// <== Reduce Score for leecher - Stulle
	if(m_htiNoBadPushing)	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiNoBadPushing, m_bNoBadPushing); // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEnableAntiCreditHack, m_bEnableAntiCreditHack); //MORPH - Added by IceCream, enable Anti-CreditHack
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAntiXsExploiter, m_bAntiXsExploiter); // Anti-XS-Exploit [Xman] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSpamBan, m_bSpamBan); // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiFilterClientFailedDown, m_bFilterClientFailedDown); 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	// ==> adjust ClientBanTime - Stulle
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiClientBanTime, m_iClientBanTime);
	DDV_MinMaxInt(pDX, m_iClientBanTime, 1, 10);
	// <== adjust ClientBanTime - Stulle

	// ==> push small files [sivka] - Stulle
    DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEnablePushSmallFile, m_bEnablePushSmallFile);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiPushSmallFileBoost, m_iPushSmallFileBoost);
	DDV_MinMaxInt(pDX, m_iPushSmallFileBoost, 1, 65536);
	// <== push small files [sivka] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEnablePushRareFile, m_bEnablePushRareFile); // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	DDX_TreeRadio(pDX, IDC_STULLE_OPTS, m_htiFnTag, (int &)m_iFnTag);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiFnCustomTag, m_sFnCustomTag);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiFnTagAtEnd, m_bFnTagAtEnd);
	// <== FunnyNick Tag - Stulle
	
	// ==> Quick start [TPT] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiQuickStart, m_bQuickStart);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiQuickStartMaxTime, m_iQuickStartMaxTime);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxTime, 8, 18);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiQuickStartMaxConnPerFive, m_iQuickStartMaxConnPerFive);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnPerFive, 5, 200);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiQuickStartMaxConn, m_iQuickStartMaxConn);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConn, 200, 2000);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiQuickStartMaxConnPerFiveBack, m_iQuickStartMaxConnPerFiveBack);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnPerFiveBack, 1, INT_MAX);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiQuickStartMaxConnBack, m_iQuickStartMaxConnBack);
	DDV_MinMaxInt(pDX, m_iQuickStartMaxConnBack, 1, INT_MAX);
	if(m_htiQuickStartAfterIPChange) DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiQuickStartAfterIPChange, m_bQuickStartAfterIPChange);
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiCheckCon,m_bCheckCon);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiICMP,m_bICMP);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiPingTimeOut, m_uiPingTimeOut);
	DDV_MinMaxInt(pDX, m_uiPingTimeOut, 1, 20);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiPingTTL, m_uiPingTTL);
	DDV_MinMaxInt(pDX, m_uiPingTTL, 8, 64);
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEnforceRatio, m_bEnforceRatio);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiRatioValue, m_iRatioValue);
	DDV_MinMaxInt(pDX, m_iRatioValue, 1, 10);
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiIsreaskSourceAfterIPChange, m_bIsreaskSourceAfterIPChange);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiInformQueuedClientsAfterIPChange, m_bInformQueuedClientsAfterIPChange);
	// <== Inform Clients after IP Change - Stulle
	// ==> Timer for ReAsk File Sources - Stulle
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiReAskFileSrc, m_iReAskFileSrc);
	DDV_MinMaxInt(pDX, m_iReAskFileSrc, 29, 55);
	// <== Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAntiUploaderBanLimit, m_iAntiUploaderBanLimit);
	DDV_MinMaxInt(pDX, m_iAntiUploaderBanLimit, 0, 20);
	DDX_TreeRadio(pDX, IDC_STULLE_OPTS, m_htiAntiUploaderBanLimit, (int &)m_iAntiUploaderBanCase);
	// <== Anti Uploader Ban - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSysInfo, m_bSysInfo);
	if(m_htiSysInfoGlobal) DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSysInfoGlobal, m_bSysInfoGlobal);
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiShowSrcOnTitle, showSrcInTitle); // Show sources on title - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiShowGlobalHL, m_bShowGlobalHL); // show global HL - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiShowFileHLconst, m_bShowFileHLconst); // show HL per file constantly
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiShowInMSN7, m_bShowInMSN7); // Show in MSN7 [TPT] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiTrayComplete, m_bTrayComplete); // Completed in Tray - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiShowSpeedMeter, m_bShowSpeedMeter); // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

	// ==> drop sources - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAutoNNS, m_bEnableAutoDropNNSDefault);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoNNSTimer, m_iAutoNNS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoNNS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoNNSLimit, m_iMaxRemoveNNSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveNNSLimitDefault, 50, 100);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAutoFQS, m_bEnableAutoDropFQSDefault);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoFQSTimer, m_iAutoFQS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoFQS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoFQSLimit, m_iMaxRemoveFQSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveFQSLimitDefault, 50, 100);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAutoQRS, m_bEnableAutoDropQRSDefault);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoQRSTimer, m_iAutoHQRS_TimerDefault);
	DDV_MinMaxInt(pDX, m_iAutoHQRS_TimerDefault, 0, 60);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoQRSMax, m_iMaxRemoveQRSDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveQRSDefault, 2500, 10000);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoQRSLimit, m_iMaxRemoveQRSLimitDefault);
	DDV_MinMaxInt(pDX, m_iMaxRemoveQRSLimitDefault, 50, 100);
	// <== drop sources - Stulle

	// ==> TBH: minimule - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiShowMM, m_bShowMM);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiMMLives, m_bMMLives);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiMMUpdateTime, m_iMMUpdateTime);
	DDV_MinMaxInt(pDX, m_iMMUpdateTime, 0, MIN2S(60));
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiMMTrans, m_iMMTrans);
	DDV_MinMaxInt(pDX, m_iMMTrans, 1, 255);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiMMCompl, m_bMMCompl);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiMMOpen, m_bMMOpen);
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	DDX_TreeRadio(pDX, IDC_STULLE_OPTS, m_htiAutoDownPrioGroup, m_iAutoDownPrio);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoDownPrioPercVal, m_iAutoDownPrioPerc);
	DDV_MinMaxInt(pDX, m_iAutoDownPrioPerc, 1, 100);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiAutoDownPrioSizeVal, m_iAutoDownPrioSize);
	DDV_MinMaxInt(pDX, m_iAutoDownPrioSize, 1, 1024);
	DDX_TreeRadio(pDX, IDC_STULLE_OPTS, m_htiAutoDownPrioValGroup, m_iAutoDownPrioVal);
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	// ==> Spread Credits Slot - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSpreadCreditsSlot, m_bSpreadCreditsSlot);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiSpreadCreditsSlotCounter, m_iSpreadCreditsSlotCounter);
	DDV_MinMaxInt(pDX, m_iSpreadCreditsSlotCounter, 3, 20);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSpreadCreditsSlotPS, m_bSpreadCreditsSlotPS);
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiGlobalHL, m_bGlobalHL);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiGlobalHlLimit, m_iGlobalHL);
	DDV_MinMaxInt(pDX, m_iGlobalHL, 1000, MAX_GSL);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiGlobalHlAll, m_bGlobalHlAll);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiGlobalHlDefault, m_bGlobalHlDefault);
	// <== Global Source Limit - Stulle

	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmuMLDonkey, m_bEmuMLDonkey);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmueDonkey, m_bEmueDonkey);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmueDonkeyHybrid, m_bEmueDonkeyHybrid);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmuShareaza, m_bEmuShareaza);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiEmuLphant, m_bEmuLphant);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiLogEmulator, m_bLogEmulator);
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	DDX_TreeRadio(pDX, IDC_STULLE_OPTS, m_htiReleaseBonusGroup, m_iReleaseBonus);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiReleaseBonusDaysEdit, m_iReleaseBonusDays);
	DDV_MinMaxInt(pDX, m_iReleaseBonusDays, 1, 16);
	// <== Release Bonus [sivka] - Stulle
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiReleaseScoreAssurance, m_bReleaseScoreAssurance); // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiAutoSharedUpdater, m_bAutoSharedUpdater);
	DDX_TreeCheck(pDX, IDC_STULLE_OPTS, m_htiSingleSharedDirUpdater, m_bSingleSharedDirUpdater);
	DDX_TreeEdit(pDX, IDC_STULLE_OPTS, m_htiTimeBetweenReloads, m_iTimeBetweenReloads);
	DDV_MinMaxInt(pDX, m_iTimeBetweenReloads, 0, 1800);
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle

	// ==> ban systems optional - Stulle
	if (m_htiBadModString)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadModString, m_bEnableAntiLeecher);
	if (m_htiBadNickBan)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadNickBan, m_bEnableAntiLeecher);
	if (m_htiGhostMod)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiGhostMod, m_bEnableAntiLeecher);
	if (m_htiAntiModIdFaker)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiAntiModIdFaker, m_bEnableAntiLeecher);
	if (m_htiAntiNickThief)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiAntiNickThief, m_bEnableAntiLeecher);
	if (m_htiEmptyNick)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiEmptyNick, m_bEnableAntiLeecher);
	if (m_htiFakeEmule)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFakeEmule, m_bEnableAntiLeecher);
	if (m_htiLeecherName)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLeecherName, m_bEnableAntiLeecher);
	if (m_htiCommunityCheck)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiCommunityCheck, m_bEnableAntiLeecher);
	if (m_htiHexCheck)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiHexCheck, m_bEnableAntiLeecher);
	if (m_htiEmcrypt)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiEmcrypt, m_bEnableAntiLeecher);
	if (m_htiBadInfo)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadInfo, m_bEnableAntiLeecher);
	if (m_htiBadHello)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadHello, m_bEnableAntiLeecher);
	if (m_htiSnafu)				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSnafu, m_bEnableAntiLeecher);
	if (m_htiExtraBytes)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiExtraBytes, m_bEnableAntiLeecher);
	if (m_htiNickChanger)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiNickChanger, m_bEnableAntiLeecher);
	if (m_htiFileFaker)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFileFaker, m_bEnableAntiLeecher);
	if (m_htiVagaa)				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiVagaa, m_bEnableAntiLeecher);
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	if(m_htiPunishmentGroup)	m_ctrlTreeOptions.SetGroupEnable(m_htiPunishmentGroup, m_bEnableAntiLeecher);
	if(m_bEnableAntiLeecher) m_ctrlTreeOptions.Expand(m_htiPunishmentGroup, TVE_EXPAND);
	else m_ctrlTreeOptions.Expand(m_htiPunishmentGroup, TVE_COLLAPSE);
	if(m_bEnableAntiLeecher) m_ctrlTreeOptions.Expand(m_htiReduce, TVE_EXPAND);
	else m_ctrlTreeOptions.Expand(m_htiReduce, TVE_COLLAPSE);
	// <== Reduce Score for leecher - Stulle
	if (m_htiNoBadPushing)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiNoBadPushing, m_bEnableAntiLeecher); // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle

	// ==> Quick start [TPT] - Stulle
	if (m_htiQuickStartAfterIPChange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartAfterIPChange, m_bQuickStart);
	// <== Quick start [TPT] - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	if (m_htiSysInfoGlobal)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSysInfoGlobal, m_bSysInfo);
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle

	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	if (m_htiSingleSharedDirUpdater)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSingleSharedDirUpdater, m_bAutoSharedUpdater);
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle
}


BOOL CPPgStulle::OnInitDialog()
{
	// ==> Sivka-Ban - Stulle
	m_bEnableSivkaBan = thePrefs.enableSivkaBan;
	m_iSivkaAskTime = (int)(thePrefs.GetSivkaAskTime());
	m_iSivkaAskCounter = (int)(thePrefs.GetSivkaAskCounter());
	m_bSivkaAskLog = thePrefs.SivkaAskLog;
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	m_bEnableAntiLeecher = thePrefs.enableAntiLeecher; //MORPH - Added by IceCream, enabnle Anti-leecher
	m_bBadModString = thePrefs.IsBadModString();
	m_bBadNickBan = thePrefs.IsBadNickBan();
	m_bGhostMod = thePrefs.IsGhostMod();
	m_bAntiModIdFaker = thePrefs.IsAntiModIdFaker();
	m_bAntiNickThief = thePrefs.IsAntiNickThief(); // AntiNickThief - Stulle
	m_bEmptyNick = thePrefs.IsEmptyNick();
	m_bFakeEmule = thePrefs.IsFakeEmule();
	m_bLeecherName = thePrefs.IsLeecherName();
	m_bCommunityCheck = thePrefs.IsCommunityCheck();
	m_bHexCheck = thePrefs.IsHexCheck();
	m_bEmcrypt = thePrefs.IsEmcrypt();
	m_bBadInfo = thePrefs.IsBadInfo();
	m_bBadHello = thePrefs.IsBadHello();
	m_bSnafu = thePrefs.IsSnafu();
	m_bExtraBytes = thePrefs.IsExtraBytes();
	m_bNickChanger = thePrefs.IsNickChanger();
	m_bFileFaker = thePrefs.IsFileFaker();
	m_bVagaa = thePrefs.IsVagaa();
	// <== ban systems optional - Stulle
	m_iReduceFactor = (int)((thePrefs.GetReduceFactor())*100.0f); // Reduce Score for leecher - Stulle
	m_bNoBadPushing = thePrefs.GetNoBadPushing(); // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	m_bEnableAntiCreditHack = thePrefs.enableAntiCreditHack; //MORPH - Added by IceCream, enabnle Anti-CreditHack
	m_bAntiXsExploiter = thePrefs.GetAntiXSExploiter(); // Anti-XS-Exploit [Xman] - Stulle
	m_bSpamBan = thePrefs.GetSpamBan(); // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_bFilterClientFailedDown = thePrefs.m_bFilterClientFailedDown; 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	m_iClientBanTime = thePrefs.GetClientBanTime()/3600000; // adjust ClientBanTime - Stulle

	// ==> push small files [sivka] - Stulle
	m_bEnablePushSmallFile = thePrefs.GetEnablePushSmallFile();
	m_iPushSmallFileBoost = thePrefs.GetPushSmallFileBoost();
	// <== push small files [sivka] - Stulle
	m_bEnablePushRareFile = thePrefs.enablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	m_iFnTag = thePrefs.GetFnTag();
	m_sFnCustomTag = thePrefs.m_sFnCustomTag;
	m_bFnTagAtEnd = thePrefs.GetFnTagAtEnd();
	// <== FunnyNick Tag - Stulle

	// ==> Quick start [TPT] - Stulle
	m_bQuickStart = thePrefs.GetQuickStart();
	m_iQuickStartMaxTime = (int)(thePrefs.GetQuickStartMaxTime());
	m_iQuickStartMaxConnPerFive = (int)(thePrefs.GetQuickStartMaxConnPerFive());
	m_iQuickStartMaxConn = (int)(thePrefs.GetQuickStartMaxConn());
	m_iQuickStartMaxConnPerFiveBack = (int)(thePrefs.GetQuickStartMaxConnPerFiveBack());
	m_iQuickStartMaxConnBack = (int)(thePrefs.GetQuickStartMaxConnBack());
	m_bQuickStartAfterIPChange = thePrefs.GetQuickStartAfterIPChange();
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	m_bCheckCon = thePrefs.GetCheckCon();
	m_bICMP = thePrefs.GetICMP();
	m_uiPingTimeOut = thePrefs.GetPingTimeout();
	m_uiPingTTL = thePrefs.GetPingTTL();
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	m_bEnforceRatio = thePrefs.GetEnforceRatio();
	m_iRatioValue = (int)thePrefs.GetRatioValue();
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	m_bIsreaskSourceAfterIPChange = thePrefs.IsRASAIC();
	m_bInformQueuedClientsAfterIPChange = thePrefs.IsIQCAOC();
	// <== Inform Clients after IP Change - Stulle
	m_iReAskFileSrc = (thePrefs.GetReAskTimeDif() + FILEREASKTIME)/60000; // Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	m_iAntiUploaderBanLimit = thePrefs.GetAntiUploaderBanLimit();
	m_iAntiUploaderBanCase = thePrefs.GetAntiUploaderBanCase();
	// <== Anti Uploader Ban - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	m_bSysInfo = thePrefs.GetSysInfo();
	m_bSysInfoGlobal = thePrefs.GetSysInfoGlobal();
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	showSrcInTitle = thePrefs.ShowSrcOnTitle(); // Show sources on title - Stulle
	m_bShowGlobalHL = thePrefs.GetShowGlobalHL(); // show global HL - Stulle
	m_bShowFileHLconst = thePrefs.GetShowFileHLconst(); // show HL per file constantly
	m_bShowInMSN7 = thePrefs.GetShowMSN7(); // Show in MSN7 [TPT] - Stulle
	m_bTrayComplete = thePrefs.GetTrayComplete(); // Completed in Tray - Stulle
	m_bShowSpeedMeter = thePrefs.GetShowSpeedMeter(); // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

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

	// ==> TBH: minimule - Stulle
	m_bShowMM = thePrefs.IsMiniMuleEnabled();
	m_bMMLives = thePrefs.GetMiniMuleLives();
	m_iMMUpdateTime = thePrefs.GetMiniMuleUpdate();
	m_iMMTrans = thePrefs.GetMiniMuleTransparency();
	m_bMMCompl = thePrefs.m_bMMCompl;
	m_bMMOpen = thePrefs.m_bMMOpen;
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	m_iAutoDownPrio = thePrefs.GetBowlfishMode();
	m_iAutoDownPrioPerc = thePrefs.GetBowlfishPrioPercentValue();
	m_iAutoDownPrioSize = thePrefs.GetBowlfishPrioSizeValue();
	m_iAutoDownPrioVal = thePrefs.GetBowlfishPrioNewValue();
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	// ==> Spread Credits Slot - Stulle
	m_bSpreadCreditsSlot = thePrefs.SpreadCreditsSlot;
	m_iSpreadCreditsSlotCounter = (int)(thePrefs.SpreadCreditsSlotCounter);
	m_bSpreadCreditsSlotPS = thePrefs.GetSpreadCreditsSlotPS();
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	m_bGlobalHL = thePrefs.IsUseGlobalHL();
	m_iGlobalHL = thePrefs.GetGlobalHL();
	m_bGlobalHlAll = thePrefs.GetGlobalHlAll();
	m_bGlobalHlDefault = thePrefs.GetGlobalHlDefault();
	// <== Global Source Limit - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	m_bEmuMLDonkey = thePrefs.IsEmuMLDonkey();
	m_bEmueDonkey = thePrefs.IsEmueDonkey();
	m_bEmueDonkeyHybrid = thePrefs.IsEmueDonkeyHybrid();
	m_bEmuShareaza = thePrefs.IsEmuShareaza();
	m_bEmuLphant = thePrefs.IsEmuLphant();
	m_bLogEmulator = thePrefs.IsEmuLog();
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	if (thePrefs.GetReleaseBonus() <= 1)
	{
		m_iReleaseBonus = thePrefs.GetReleaseBonus();
		m_iReleaseBonusDays = 1;
	}
	else
	{
		m_iReleaseBonus = 2;
		m_iReleaseBonusDays = (int)(thePrefs.GetReleaseBonus()/2);
	}
	// <== Release Bonus [sivka] - Stulle
	m_bReleaseScoreAssurance = thePrefs.GetReleaseScoreAssurance(); // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	m_bAutoSharedUpdater = thePrefs.GetDirectoryWatcher();
	m_bSingleSharedDirUpdater = thePrefs.GetSingleSharedDirWatcher();
	m_iTimeBetweenReloads = thePrefs.GetTimeBetweenReloads();
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle

	m_ctrlTreeOptions.SetImageListColorFlags(theApp.m_iDfltImageListColorFlags);
    CPropertyPage::OnInitDialog();

	// ==> push small files [sivka] - Stulle
	InitWindowStyles(this);
	m_ctlPushSmallSize.SetRange(1, PARTSIZE>>10, TRUE);
	m_ctlPushSmallSize.SetPos(thePrefs.GetPushSmallFileSize()>>10);
	m_ctlPushSmallSize.SetTicFreq(1024);
	m_ctlPushSmallSize.SetPageSize(1024);
	ShowPushSmallFileValues();
//	LoadSettings();
	// <== push small files [sivka] - Stulle

	Localize();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/*
void CPPgStulle::LoadSettings(void)
{
	if(m_hWnd)
	{
		CString strBuffer;
	
		// ==> push small files [sivka] - Stulle
		m_ctlPushSmallSize.SetRange(1, PARTSIZE, TRUE);
		m_ctlPushSmallSize.SetPos(thePrefs.GetPushSmallFileSize());
		m_ctlPushSmallSize.SetTicFreq(20);
		m_ctlPushSmallSize.SetPageSize(10);
		ShowPushSmallFileValues();
		// <== push small files [sivka] - Stulle
	}
}
*/

BOOL CPPgStulle::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgStulle::OnApply()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	
	if (!UpdateData())
		return FALSE;
	// ==> Sivka-Ban - Stulle
	thePrefs.enableSivkaBan = m_bEnableSivkaBan;
	thePrefs.m_iSivkaAskTime = (uint16)m_iSivkaAskTime;
	thePrefs.m_iSivkaAskCounter = (uint16)m_iSivkaAskCounter;
	thePrefs.SivkaAskLog = m_bSivkaAskLog;
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	thePrefs.enableAntiLeecher = m_bEnableAntiLeecher; //MORPH - Added by IceCream, enable Anti-leecher
	thePrefs.m_bBadModString = m_bBadModString;
	thePrefs.m_bBadNickBan = m_bBadNickBan;
	thePrefs.m_bGhostMod = m_bGhostMod;
	thePrefs.m_bAntiModIdFaker = m_bAntiModIdFaker;
	thePrefs.m_bAntiNickThief = m_bAntiNickThief; // AntiNickThief - Stulle
	thePrefs.m_bEmptyNick = m_bEmptyNick;
	thePrefs.m_bFakeEmule = m_bFakeEmule;
	thePrefs.m_bLeecherName = m_bLeecherName;
	thePrefs.m_bCommunityCheck = m_bCommunityCheck;
	thePrefs.m_bHexCheck = m_bHexCheck;
	thePrefs.m_bEmcrypt = m_bEmcrypt;
	thePrefs.m_bBadInfo = m_bBadInfo;
	thePrefs.m_bBadHello = m_bBadHello;
	thePrefs.m_bSnafu = m_bSnafu;
	thePrefs.m_bExtraBytes = m_bExtraBytes;
	thePrefs.m_bNickChanger = m_bNickChanger;
	thePrefs.m_bFileFaker = m_bFileFaker;
	thePrefs.m_bVagaa = m_bVagaa;
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	if(m_iReduceScore == 0 && thePrefs.IsReduceScore() == true)
		theApp.clientlist->BanReducedLeechers();
	thePrefs.m_bReduceScore = m_iReduceScore == 1;
	if(m_iReduceFactor == 100)
		thePrefs.m_fReduceFactor = 1.0f;
	else
	{
		CString m_strTemp = _T("0.");
		m_strTemp.AppendFormat(_T("%i"),m_iReduceFactor);
	//	AddLogLine(false,_T("%s"),m_strTemp);
		thePrefs.m_fReduceFactor = (float)_tstof(m_strTemp);
	}
	// <== Reduce Score for leecher - Stulle
	thePrefs.m_bNoBadPushing = m_bNoBadPushing; // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	thePrefs.enableAntiCreditHack = m_bEnableAntiCreditHack; //MORPH - Added by IceCream, enable Anti-CreditHack
	thePrefs.m_bAntiXsExploiter = m_bAntiXsExploiter; // Anti-XS-Exploit [Xman] - Stulle
	thePrefs.m_bSpamBan = m_bSpamBan; // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	thePrefs.m_bFilterClientFailedDown = m_bFilterClientFailedDown; 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	thePrefs.m_dwClientBanTime = m_iClientBanTime*3600000; // adjust ClientBanTime - Stulle

	// ==> push small files [sivka] - Stulle
	thePrefs.enablePushSmallFile = m_bEnablePushSmallFile;
	thePrefs.m_iPushSmallBoost = (uint16)m_iPushSmallFileBoost;
//	CString strBuffer;
//	m_ctlPushSmallSize.SetRange(1, PARTSIZE, TRUE);
	thePrefs.m_iPushSmallFiles = m_ctlPushSmallSize.GetPos()<<10;
	// <== push small files [sivka] - Stulle
	thePrefs.enablePushRareFile = m_bEnablePushRareFile; // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	thePrefs.FnTagMode = (uint8)m_iFnTag;
	_stprintf (thePrefs.m_sFnCustomTag,_T("%s"), m_sFnCustomTag);
	thePrefs.m_bFnTagAtEnd = m_bFnTagAtEnd;
	// <== FunnyNick Tag - Stulle

	// ==> Quick start [TPT] - Stulle
	thePrefs.m_bQuickStart = m_bQuickStart;
	thePrefs.m_iQuickStartMaxTime = (uint16)m_iQuickStartMaxTime;
	thePrefs.m_iQuickStartMaxConnPerFive = (uint16)m_iQuickStartMaxConnPerFive;
	thePrefs.m_iQuickStartMaxConn = (UINT)m_iQuickStartMaxConn;
	thePrefs.m_iQuickStartMaxConnPerFiveBack = (uint16)m_iQuickStartMaxConnPerFiveBack;
	thePrefs.m_iQuickStartMaxConnBack = (UINT)m_iQuickStartMaxConnBack;
	thePrefs.m_bQuickStartAfterIPChange = m_bQuickStartAfterIPChange;
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	thePrefs.m_bCheckCon = m_bCheckCon;
	thePrefs.m_bICMP = m_bICMP;
	thePrefs.m_uiPingTimeOut = (uint8)m_uiPingTimeOut;
	thePrefs.m_uiPingTTL = (uint8)m_uiPingTTL;
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	thePrefs.m_bEnforceRatio = m_bEnforceRatio;
	thePrefs.m_uRatioValue = (uint8) m_iRatioValue;
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	thePrefs.m_breaskSourceAfterIPChange = m_bIsreaskSourceAfterIPChange;
	thePrefs.m_bInformQueuedClientsAfterIPChange = m_bInformQueuedClientsAfterIPChange;
	// <== Inform Clients after IP Change - Stulle
	thePrefs.m_uReAskTimeDif = (m_iReAskFileSrc-29)*60000; // Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	thePrefs.m_iAntiUploaderBanLimit = (uint16)m_iAntiUploaderBanLimit;
	thePrefs.AntiUploaderBanCaseMode = (uint8)m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban - Stulle

	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	if(thePrefs.m_bSysInfo != m_bSysInfo)
	{
		thePrefs.m_bSysInfo = m_bSysInfo;
		theApp.emuledlg->transferwnd->EnableSysInfo(m_bSysInfo);
	}
	thePrefs.m_bSysInfoGlobal = m_bSysInfoGlobal;
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	thePrefs.showSrcInTitle = showSrcInTitle; // Show sources on title - Stulle
	thePrefs.ShowGlobalHL = m_bShowGlobalHL; // show global HL - Stulle
	thePrefs.ShowFileHLconst = m_bShowFileHLconst; // show HL per file constantly
	thePrefs.m_bShowInMSN7 = m_bShowInMSN7; // Show in MSN7 [TPT] - Stulle
	thePrefs.m_bTrayComplete = m_bTrayComplete; // Completed in Tray - Stulle
	// ==> High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle
	if(m_bShowSpeedMeter != thePrefs.GetShowSpeedMeter())
	{
		thePrefs.m_bShowSpeedMeter = m_bShowSpeedMeter;
		theApp.emuledlg->toolbar->ShowSpeedMeter(m_bShowSpeedMeter);
	}
	// <== High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

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

	// ==> TBH: minimule - Stulle
	thePrefs.m_bMiniMule = m_bShowMM;
	thePrefs.SetMiniMuleLives(m_bMMLives);
	thePrefs.m_iMiniMuleUpdate = m_iMMUpdateTime;
	thePrefs.SetMiniMuleTransparency((uint8)m_iMMTrans);
	thePrefs.m_bMMCompl = m_bMMCompl;
	thePrefs.m_bMMOpen = m_bMMOpen;
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	thePrefs.m_uiBowlfishMode = (uint8)m_iAutoDownPrio;
	thePrefs.m_nBowlfishPrioPercentValue = (uint8)m_iAutoDownPrioPerc;
	thePrefs.m_nBowlfishPrioSizeValue = (uint16)m_iAutoDownPrioSize;
	thePrefs.m_nBowlfishPrioNewValue = (uint8)m_iAutoDownPrioVal;
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	// ==> Spread Credits Slot - Stulle
	thePrefs.SpreadCreditsSlot = m_bSpreadCreditsSlot;
	thePrefs.SpreadCreditsSlotCounter = (uint16)m_iSpreadCreditsSlotCounter;
	thePrefs.m_bSpreadCreditsSlotPS = m_bSpreadCreditsSlotPS;
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	if (thePrefs.GetGlobalHL() != (UINT)m_iGlobalHL ||
		thePrefs.IsUseGlobalHL() != m_bGlobalHL ||
		thePrefs.m_bGlobalHlAll != m_bGlobalHlAll)
	{
		thePrefs.m_bGlobalHL = m_bGlobalHL;
		thePrefs.m_uGlobalHL = m_iGlobalHL;
		thePrefs.m_bGlobalHlAll = m_bGlobalHlAll;
		if(m_bGlobalHL && theApp.downloadqueue->GetPassiveMode())
		{
			theApp.downloadqueue->SetPassiveMode(false);
			theApp.downloadqueue->SetUpdateHlTime(50000); // 50 sec
			AddDebugLogLine(true,_T("{GSL} Global Source Limit settings have changed! Disabled PassiveMode!"));
		}
	}
	thePrefs.m_bGlobalHlDefault = m_bGlobalHlDefault;
	// <== Global Source Limit - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	thePrefs.m_bEmuMLDonkey = m_bEmuMLDonkey;
	thePrefs.m_bEmueDonkey = m_bEmueDonkey;
	thePrefs.m_bEmueDonkeyHybrid = m_bEmueDonkeyHybrid;
	thePrefs.m_bEmuShareaza = m_bEmuShareaza;
	thePrefs.m_bEmuLphant = m_bEmuLphant;
	thePrefs.m_bLogEmulator = m_bLogEmulator;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	if (m_iReleaseBonus <= 1)
        thePrefs.m_uReleaseBonus = (uint8)m_iReleaseBonus;
	else
		thePrefs.m_uReleaseBonus = (uint8)(m_iReleaseBonusDays*2);
	// <== Release Bonus [sivka] - Stulle
	thePrefs.m_bReleaseScoreAssurance = m_bReleaseScoreAssurance; // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	if(m_bAutoSharedUpdater != thePrefs.GetDirectoryWatcher() ||
		m_bSingleSharedDirUpdater != thePrefs.GetSingleSharedDirWatcher() ||
		m_iTimeBetweenReloads != thePrefs.GetTimeBetweenReloads())
	{
		thePrefs.SetDirectoryWatcher(m_bAutoSharedUpdater);
		thePrefs.SetSingleSharedDirWatcher(m_bSingleSharedDirUpdater);
		thePrefs.SetTimeBetweenReloads((uint32)m_iTimeBetweenReloads);
		theApp.ResetDirectoryWatcher();
	}
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle

//	LoadSettings();

	// ==> Show sources on title - Stulle
	TCHAR buffer[510];

	if (!thePrefs.ShowRatesOnTitle() && !thePrefs.ShowSrcOnTitle()) {
		//MORPH START - Changed by SiRoB, [itsonlyme: -modname-]
		/*
		_stprintf(buffer,_T("eMule v%s"),theApp.m_strCurVersionLong);
		*/
		_stprintf(buffer,_T("eMule v%s [%s]"),theApp.m_strCurVersionLong,theApp.m_strModLongVersion);
		//MORPH END   - Changed by SiRoB, [itsonlyme: -modname-]
		theApp.emuledlg->SetWindowText(buffer);
	}
	// <== Show sources on title - Stulle

	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

void CPPgStulle::Localize(void)
{	
	if(m_hWnd)
	{
		GetDlgItem(IDC_WARNINGMORPH)->SetWindowText(GetResString(IDS_WARNINGMORPH));
		// ==> Sivka-Ban - Stulle
		if (m_htiEnableSivkaBan) m_ctrlTreeOptions.SetItemText(m_htiEnableSivkaBan, GetResString(IDS_SIVKA_BAN));
		if (m_htiSivkaAskTime) m_ctrlTreeOptions.SetEditLabel(m_htiSivkaAskTime, GetResString(IDS_SIVKA_ASK_TIME));
		if (m_htiSivkaAskCounter) m_ctrlTreeOptions.SetEditLabel(m_htiSivkaAskCounter, GetResString(IDS_SIVKA_ASK_COUNTER));
		if (m_htiSivkaAskLog) m_ctrlTreeOptions.SetItemText(m_htiSivkaAskLog, GetResString(IDS_SIVKA_ASK_LOG));
		// <== Sivka-Ban - Stulle
		// ==> ban systems optional - Stulle
		if (m_htiEnableAntiLeecher) m_ctrlTreeOptions.SetItemText(m_htiEnableAntiLeecher, GetResString(IDS_SUC_ENABLED)); //MORPH - Added by IceCream, enable Anti-leecher
		if (m_htiBadModString) m_ctrlTreeOptions.SetItemText(m_htiBadModString, GetResString(IDS_BAD_MOD_STRING));
		if (m_htiBadNickBan) m_ctrlTreeOptions.SetItemText(m_htiBadNickBan, GetResString(IDS_BAD_NICK_BAN));
		if (m_htiGhostMod) m_ctrlTreeOptions.SetItemText(m_htiGhostMod, GetResString(IDS_GHOST_MOD));
		if (m_htiAntiModIdFaker) m_ctrlTreeOptions.SetItemText(m_htiAntiModIdFaker, GetResString(IDS_ANTI_MOD_FAKER));
		if (m_htiAntiNickThief) m_ctrlTreeOptions.SetItemText(m_htiAntiNickThief, GetResString(IDS_ANTI_NICK_THIEF)); // AntiNickThief - Stulle
		if (m_htiEmptyNick) m_ctrlTreeOptions.SetItemText(m_htiEmptyNick, GetResString(IDS_EMPTY_NICK));
		if (m_htiFakeEmule) m_ctrlTreeOptions.SetItemText(m_htiFakeEmule, GetResString(IDS_FAKE_EMULE));
		if (m_htiLeecherName) m_ctrlTreeOptions.SetItemText(m_htiLeecherName, GetResString(IDS_LEECHER_NAME));
		if (m_htiCommunityCheck) m_ctrlTreeOptions.SetItemText(m_htiCommunityCheck, GetResString(IDS_COMMUNITY_CHECK));
		if (m_htiHexCheck) m_ctrlTreeOptions.SetItemText(m_htiHexCheck, GetResString(IDS_HEX_CHECK));
		if (m_htiEmcrypt) m_ctrlTreeOptions.SetItemText(m_htiEmcrypt, GetResString(IDS_EMCRYPT));
		if (m_htiBadInfo) m_ctrlTreeOptions.SetItemText(m_htiBadInfo, GetResString(IDS_BAD_INFO));
		if (m_htiBadHello) m_ctrlTreeOptions.SetItemText(m_htiBadHello, GetResString(IDS_BAD_HELLO));
		if (m_htiSnafu) m_ctrlTreeOptions.SetItemText(m_htiSnafu, GetResString(IDS_SNAFU));
		if (m_htiExtraBytes) m_ctrlTreeOptions.SetItemText(m_htiExtraBytes, GetResString(IDS_EXTRA_BYTES));
		if (m_htiNickChanger) m_ctrlTreeOptions.SetItemText(m_htiNickChanger, GetResString(IDS_NICK_CHANGER));
		if (m_htiFileFaker) m_ctrlTreeOptions.SetItemText(m_htiFileFaker, GetResString(IDS_FILEFAKER));
		if (m_htiVagaa) m_ctrlTreeOptions.SetItemText(m_htiVagaa, GetResString(IDS_VAGAA));
		// <== ban systems optional - Stulle
		if (m_htiReduceFactor) m_ctrlTreeOptions.SetEditLabel(m_htiReduceFactor, GetResString(IDS_PUNISHMENT_PERCENT)); // Reduce Score for leecher - Stulle
		if (m_htiNoBadPushing) m_ctrlTreeOptions.SetItemText(m_htiNoBadPushing, GetResString(IDS_NO_BAD_PUSHING)); // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
		if (m_htiEnableAntiCreditHack) m_ctrlTreeOptions.SetItemText(m_htiEnableAntiCreditHack, GetResString(IDS_ANTI_CREDITHACK)); //MORPH - Added by IceCream, enable Anti-CreditHack
		if (m_htiAntiXsExploiter) m_ctrlTreeOptions.SetItemText(m_htiAntiXsExploiter, GetResString(IDS_ANTI_XS_EXPLOITER)); // Anti-XS-Exploit [Xman] - Stulle
		if (m_bSpamBan) m_ctrlTreeOptions.SetItemText(m_htiSpamBan, GetResString(IDS_SPAM_BAN)); // Spam Ban [Xman] - Stulle
		//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
		if (m_htiFilterClientFailedDown) m_ctrlTreeOptions.SetItemText(m_htiFilterClientFailedDown, GetResString(IDS_FILTER_CLIENTFAILEDDOWN));
		//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
		if (m_htiClientBanTime) m_ctrlTreeOptions.SetEditLabel(m_htiClientBanTime, GetResString(IDS_CLIENT_BAN_TIME)); // adjust ClientBanTime - Stulle

		// ==> push small files [sivka] - Stulle
		if (m_htiEnablePushSmallFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushSmallFile, GetResString(IDS_PUSH_SMALL));
		if (m_htiPushSmallFileBoost) m_ctrlTreeOptions.SetEditLabel(m_htiPushSmallFileBoost, GetResString(IDS_PUSH_SMALL_BOOST));
		GetDlgItem(IDC_PUSHSMALL_LABEL)->SetWindowText(GetResString(IDS_PUSH_SMALL));
		// <== push small files [sivka] - Stulle
		if (m_htiEnablePushRareFile) m_ctrlTreeOptions.SetItemText(m_htiEnablePushRareFile, GetResString(IDS_PUSH_RARE)); // push rare file - Stulle

		// ==> FunnyNick Tag - Stulle
		if (m_htiFnCustomTag) m_ctrlTreeOptions.SetEditLabel(m_htiFnCustomTag, GetResString(IDS_SET_CUSTOM_TAG));
		if (m_htiFnTagAtEnd) m_ctrlTreeOptions.SetItemText(m_htiFnTagAtEnd, GetResString(IDS_FN_TAG_AT_END));
		// <== FunnyNick Tag - Stulle

		// ==> Quick start [TPT] - Stulle
		if (m_htiQuickStart) m_ctrlTreeOptions.SetItemText(m_htiQuickStart, GetResString(IDS_QUICK_START));
		if (m_htiQuickStartMaxTime) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxTime, GetResString(IDS_QUICK_START_MAX_TIME));
		if (m_htiQuickStartMaxConnPerFive) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnPerFive, GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE));
		if (m_htiQuickStartMaxConn) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConn, GetResString(IDS_QUICK_START_MAX_CONN));
		if (m_htiQuickStartMaxConnPerFiveBack) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnPerFiveBack, GetResString(IDS_QUICK_START_MAX_CONN_PER_FIVE_BACK));
		if (m_htiQuickStartMaxConnBack) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartMaxConnBack, GetResString(IDS_QUICK_START_MAX_CONN_BACK));
		if (m_htiQuickStartAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiQuickStartAfterIPChange, GetResString(IDS_QUICK_START_AFTER_IP_CHANGE));
		// <== Quick start [TPT] - Stulle
		// ==> Connection Checker [eWombat/WiZaRd] - Stulle
		if(m_htiCheckCon) m_ctrlTreeOptions.SetItemText(m_htiCheckCon, GetResString(IDS_SUC_ENABLED));
		if(m_htiICMP) m_ctrlTreeOptions.SetItemText(m_htiICMP, GetResString(IDS_ENABLE_ICMP));
		if(m_htiPingTimeOut) m_ctrlTreeOptions.SetEditLabel(m_htiPingTimeOut, GetResString(IDS_PING_TIME_OUT));
		if(m_htiPingTTL) m_ctrlTreeOptions.SetEditLabel(m_htiPingTTL, GetResString(IDS_PING_TTL));
		// <== Connection Checker [eWombat/WiZaRd] - Stulle
		// ==> Enforce Ratio - Stulle
		if (m_htiEnforceRatio) m_ctrlTreeOptions.SetItemText(m_htiEnforceRatio, GetResString(IDS_ENFORCE_RATIO));
		if (m_htiRatioValue) m_ctrlTreeOptions.SetEditLabel(m_htiRatioValue, GetResString(IDS_RATIO_VALUE));
		// <== Enforce Ratio - Stulle
		// ==> Inform Clients after IP Change - Stulle
		if (m_htiIsreaskSourceAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiIsreaskSourceAfterIPChange, GetResString(IDS_RSAIC));
		if (m_htiInformQueuedClientsAfterIPChange) m_ctrlTreeOptions.SetItemText(m_htiInformQueuedClientsAfterIPChange, GetResString(IDS_IQCAOC));
		// <== Inform Clients after IP Change - Stulle
		if (m_htiReAskFileSrc) m_ctrlTreeOptions.SetEditLabel(m_htiReAskFileSrc, GetResString(IDS_REASK_FILE_SRC)); // Timer for ReAsk File Sources - Stulle

		// ==> Anti Uploader Ban - Stulle
		if (m_htiAntiUploaderBanLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAntiUploaderBanLimit, GetResString(IDS_UNBAN_UPLOADER));
		// <== Anti Uploader Ban - Stulle

		// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
		if (m_htiSysInfo) m_ctrlTreeOptions.SetItemText(m_htiSysInfo, GetResString(IDS_SUC_ENABLED));
		if (m_htiSysInfoGlobal) m_ctrlTreeOptions.SetItemText(m_htiSysInfoGlobal, GetResString(IDS_SYS_INFO_GLOBAL));
		// <== CPU/MEM usage [$ick$/Stulle] - Stulle
		if (m_htiShowSrcOnTitle) m_ctrlTreeOptions.SetItemText(m_htiShowSrcOnTitle, GetResString(IDS_SHOWSRCONTITLE)); // Show sources on title - Stulle
		if (m_htiShowGlobalHL) m_ctrlTreeOptions.SetItemText(m_htiShowGlobalHL, GetResString(IDS_SHOW_GLOBAL_HL)); // show global HL - Stulle
		if (m_htiShowFileHLconst) m_ctrlTreeOptions.SetItemText(m_htiShowFileHLconst, GetResString(IDS_SHOW_FILE_HL_CONST)); // show HL per file constantly
		if (m_htiShowInMSN7) m_ctrlTreeOptions.SetItemText(m_htiShowInMSN7, GetResString(IDS_SHOWINMSN7)); // Show in MSN7 [TPT] - Stulle
		if (m_htiTrayComplete) m_ctrlTreeOptions.SetItemText(m_htiTrayComplete, GetResString(IDS_TRAY_COMPLETE)); // Completed in Tray - Stulle
		if (m_htiShowSpeedMeter) m_ctrlTreeOptions.SetItemText(m_htiShowSpeedMeter, GetResString(IDS_SHOW_SPEED_METER)); // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

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

		// ==> TBH: minimule - Stulle
		if (m_htiMMGroup) m_ctrlTreeOptions.SetItemText(m_htiMMGroup, _T("TBH Mini-Mule"));
		if (m_htiShowMM) m_ctrlTreeOptions.SetItemText(m_htiShowMM, GetResString(IDS_MM_SHOW));
		if (m_htiMMLives) m_ctrlTreeOptions.SetItemText(m_htiMMLives, GetResString(IDS_MM_LIVES));
		if (m_htiMMUpdateTime) m_ctrlTreeOptions.SetEditLabel(m_htiMMUpdateTime, GetResString(IDS_MM_UPDATE_TIME));
		if (m_htiMMTrans) m_ctrlTreeOptions.SetEditLabel(m_htiMMTrans, GetResString(IDS_MM_TRANS));
		if (m_htiMMCompl) m_ctrlTreeOptions.SetItemText(m_htiMMCompl, GetResString(IDS_MM_COMPL));
		if (m_htiMMOpen) m_ctrlTreeOptions.SetItemText(m_htiMMOpen, GetResString(IDS_MM_OPEN));
		// <== TBH: minimule - Stulle

		// ==> Spread Credits Slot - Stulle
		if (m_htiSpreadCreditsSlot) m_ctrlTreeOptions.SetItemText(m_htiSpreadCreditsSlot, GetResString(IDS_SUC_ENABLED));
		if (m_htiSpreadCreditsSlotCounter) m_ctrlTreeOptions.SetEditLabel(m_htiSpreadCreditsSlotCounter, GetResString(IDS_SPREAD_CREDITS_SLOT_COUNTER));
		if (m_htiSpreadCreditsSlotPS) m_ctrlTreeOptions.SetItemText(m_htiSpreadCreditsSlotPS, GetResString(IDS_SPREAD_CREDITS_SLOT_PS));
		// <== Spread Credits Slot - Stulle
		// ==> Global Source Limit - Stulle
		if (m_htiGlobalHL) m_ctrlTreeOptions.SetItemText(m_htiGlobalHL, GetResString(IDS_SUC_ENABLED));
		if (m_htiGlobalHlLimit) m_ctrlTreeOptions.SetEditLabel(m_htiGlobalHlLimit, GetResString(IDS_GLOBAL_HL_LIMIT));
		if (m_htiGlobalHlAll) m_ctrlTreeOptions.SetItemText(m_htiGlobalHlAll, GetResString(IDS_GLOBAL_HL_ALL));
		if (m_htiGlobalHlDefault) m_ctrlTreeOptions.SetItemText(m_htiGlobalHlDefault, GetResString(IDS_GLOBAL_HL_DEFAULT));
		// <== Global Source Limit - Stulle
		// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		if (m_htiEmuMLDonkey) m_ctrlTreeOptions.SetItemText(m_htiEmuMLDonkey, GetResString(IDS_EMULATE_ML));
		if (m_htiEmueDonkey) m_ctrlTreeOptions.SetItemText(m_htiEmueDonkey, GetResString(IDS_EMULATE_DONK));
		if (m_htiEmueDonkeyHybrid) m_ctrlTreeOptions.SetItemText(m_htiEmueDonkeyHybrid, GetResString(IDS_EMULATE_DONK_HYB));
		if (m_htiEmuShareaza) m_ctrlTreeOptions.SetItemText(m_htiEmuShareaza, GetResString(IDS_EMULATE_SHA));
		if (m_htiEmuLphant) m_ctrlTreeOptions.SetItemText(m_htiEmuLphant, GetResString(IDS_EMULATE_PHANT));
		if (m_htiLogEmulator) m_ctrlTreeOptions.SetItemText(m_htiLogEmulator, GetResString(IDS_EMULATE_LOG));
		// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
		if (m_htiReleaseBonusDaysEdit) m_ctrlTreeOptions.SetEditLabel(m_htiReleaseBonusDaysEdit, GetResString(IDS_RELEASE_BONUS_EDIT)); // Release Bonus [sivka] - Stulle
		if (m_htiReleaseScoreAssurance) m_ctrlTreeOptions.SetItemText(m_htiReleaseScoreAssurance, GetResString(IDS_RELEASE_SCORE_ASSURANCE)); // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
		if (m_htiAutoSharedUpdater) m_ctrlTreeOptions.SetItemText(m_htiAutoSharedUpdater, GetResString(IDS_SUC_ENABLED));
		if (m_htiSingleSharedDirUpdater) m_ctrlTreeOptions.SetItemText(m_htiSingleSharedDirUpdater,GetResString(IDS_ASFU_SINGLE));
		if (m_htiTimeBetweenReloads) m_ctrlTreeOptions.SetEditLabel(m_htiTimeBetweenReloads, GetResString(IDS_ASFU_TIMEBETWEEN));
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle
	}

}

void CPPgStulle::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;
	m_htiSecu = NULL;
	// ==> Sivka-Ban - Stulle
	m_htiSivkaBanGroup = NULL;
	m_htiEnableSivkaBan = NULL;
	m_htiSivkaAskTime = NULL;
	m_htiSivkaAskCounter = NULL;
	m_htiSivkaAskLog = NULL;
	// <== Sivka-Ban - Stulle
	// ==> ban systems optional - Stulle
	m_htiAntiLeecherGroup = NULL;
	m_htiEnableAntiLeecher = NULL; //MORPH - Added by IceCream, activate Anti-leecher
	m_htiBadModString = NULL;
	m_htiBadNickBan = NULL;
	m_htiGhostMod = NULL;
	m_htiAntiModIdFaker = NULL;
	m_htiAntiNickThief = NULL; // AntiNickThief - Stulle
	m_htiEmptyNick = NULL;
	m_htiFakeEmule = NULL;
	m_htiLeecherName = NULL;
	m_htiCommunityCheck = NULL;
	m_htiHexCheck = NULL;
	m_htiEmcrypt = NULL;
	m_htiBadInfo = NULL;
	m_htiBadHello = NULL;
	m_htiSnafu = NULL;
	m_htiExtraBytes = NULL;
	m_htiNickChanger = NULL;
	m_htiFileFaker = NULL;
	m_htiVagaa = NULL;
	// <== ban systems optional - Stulle
	// ==> Reduce Score for leecher - Stulle
	m_htiPunishmentGroup = NULL;
	m_htiBanAll = NULL;
	m_htiReduce = NULL;
	m_htiReduceFactor = NULL;
	// <== Reduce Score for leecher - Stulle
	m_htiNoBadPushing = NULL; // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
	m_htiEnableAntiCreditHack = NULL; //MORPH - Added by IceCream, activate Anti-CreditHack
	m_htiAntiXsExploiter = NULL; // Anti-XS-Exploit [Xman] - Stulle
	m_htiSpamBan = NULL; // Spam Ban [Xman] - Stulle
	//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiFilterClientFailedDown = NULL; 
	//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]
	m_htiClientBanTime = NULL; // adjust ClientBanTime - Stulle

	m_htiPush = NULL; // push files - Stulle
	// ==> push small files [sivka] - Stulle
	m_htiEnablePushSmallFile = NULL;
	m_htiPushSmallFileBoost = NULL;
	// <== push small files [sivka] - Stulle
	m_htiEnablePushRareFile = NULL; // push rare file - Stulle

	// ==> FunnyNick Tag - Stulle
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick Tag - Stulle

	m_htiConTweaks = NULL;
	// ==> Quick start [TPT] - Stulle
	m_htiQuickStartGroup = NULL;
	m_htiQuickStart = NULL;
	m_htiQuickStartMaxTime = NULL;
	m_htiQuickStartMaxConnPerFive = NULL;
	m_htiQuickStartMaxConn = NULL;
	m_htiQuickStartMaxConnPerFiveBack = NULL;
	m_htiQuickStartMaxConnBack = NULL;
	m_htiQuickStartAfterIPChange = NULL;
	// <== Quick start [TPT] - Stulle
	// ==> Connection Checker [eWombat/WiZaRd] - Stulle
	m_htiCheckConGroup = NULL;
	m_htiCheckCon = NULL;
	m_htiICMP = NULL;
	m_htiPingTimeOut = NULL;
	m_htiPingTTL = NULL;
	// <== Connection Checker [eWombat/WiZaRd] - Stulle
	// ==> Enforce Ratio - Stulle
	m_htiRatioGroup = NULL;
	m_htiEnforceRatio = NULL;
	m_htiRatioValue = NULL;
	// <== Enforce Ratio - Stulle
	// ==> Inform Clients after IP Change - Stulle
	m_htiIsreaskSourceAfterIPChange = NULL;
	m_htiInformQueuedClientsAfterIPChange = NULL;
	// <== Inform Clients after IP Change - Stulle
	m_htiReAskFileSrc = NULL; // Timer for ReAsk File Sources - Stulle

	// ==> Anti Uploader Ban - Stulle
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban - Stulle

	m_htiDisplay = NULL;
	// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
	m_htiSysInfoGroup = NULL;
	m_htiSysInfo = NULL;
	m_htiSysInfoGlobal = NULL;
	// <== CPU/MEM usage [$ick$/Stulle] - Stulle
	m_htiShowSrcOnTitle = NULL; // Show sources on title - Stulle
	m_htiShowGlobalHL = NULL; // show global HL - Stulle
	m_htiShowFileHLconst = NULL; // show HL per file constantly
	m_htiShowInMSN7 = NULL; // Show in MSN7 [TPT] - Stulle
	m_htiTrayComplete = NULL; // Completed in Tray - Stulle
	m_htiShowSpeedMeter = NULL; // High resulution speedmeter on toolbar [eFMod/Stulle] - Stulle

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

	// ==> TBH: minimule - Stulle
	m_htiMMGroup = NULL;
	m_htiShowMM = NULL;
	m_htiMMLives = NULL;
	m_htiMMUpdateTime = NULL;
	m_htiMMTrans = NULL;
	m_htiMMCompl = NULL;
	m_htiMMOpen = NULL;
	// <== TBH: minimule - Stulle

	// ==> Control download priority [tommy_gun/iONiX] - Stulle
	m_htiAutoDownPrioGroup = NULL;
	m_htiAutoDownPrioOff = NULL;
	m_htiAutoDownPrioPerc = NULL;
	m_htiAutoDownPrioPercVal = NULL;
	m_htiAutoDownPrioSize = NULL;
	m_htiAutoDownPrioSizeVal = NULL;
	m_htiAutoDownPrioValGroup = NULL;
	m_htiAutoDownPrioLow = NULL;
	m_htiAutoDownPrioNormal = NULL;
	m_htiAutoDownPrioHigh = NULL;
	// <== Control download priority [tommy_gun/iONiX] - Stulle

	m_htiMisc = NULL;
	// ==> Spread Credits Slot - Stulle
	m_htiSpreadCreditsSlotGroup = NULL;
	m_htiSpreadCreditsSlot = NULL;
	m_htiSpreadCreditsSlotCounter = NULL;
	m_htiSpreadCreditsSlotPS = NULL;
	// <== Spread Credits Slot - Stulle
	// ==> Global Source Limit - Stulle
	m_htiGlobalHL = NULL;
	m_htiGlobalHlLimit = NULL;
	m_htiGlobalHlAll = NULL;
	m_htiGlobalHlDefault = NULL;
	// <== Global Source Limit - Stulle
	// ==> Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	m_htiEmulatorGroup = NULL;
	m_htiEmuMLDonkey = NULL;
	m_htiEmueDonkey = NULL;
	m_htiEmueDonkeyHybrid = NULL;
	m_htiEmuShareaza = NULL;
	m_htiEmuLphant = NULL;
	m_htiLogEmulator = NULL;
	// <== Emulate others [WiZaRd/Spike/shadow2004] - Stulle
	// ==> Release Bonus [sivka] - Stulle
	m_htiReleaseBonusGroup = NULL;
	m_htiReleaseBonus0 = NULL;
	m_htiReleaseBonus1 = NULL;
	m_htiReleaseBonusDays = NULL;
	m_htiReleaseBonusDaysEdit = NULL;
	// <== Release Bonus [sivka] - Stulle
	m_htiReleaseScoreAssurance = NULL; // Release Score Assurance - Stulle
	// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
	m_htiAutoSharedGroup = NULL;
	m_htiAutoSharedUpdater = NULL;
	m_htiSingleSharedDirUpdater = NULL;
	m_htiTimeBetweenReloads = NULL;
#endif
	// <== Automatic shared files updater [MoNKi] - Stulle

	CPropertyPage::OnDestroy();
}
LRESULT CPPgStulle::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_STULLE_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;

	BOOL bCheck;

		// ==> ban systems optional - Stulle
		if (m_htiEnableAntiLeecher && pton->hItem == m_htiEnableAntiLeecher)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiEnableAntiLeecher, bCheck))
			{
				if (m_htiBadModString)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadModString, bCheck);
				if (m_htiBadNickBan)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadNickBan, bCheck);
				if (m_htiGhostMod)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiGhostMod, bCheck);
				if (m_htiAntiModIdFaker)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiAntiModIdFaker, bCheck);
				if (m_htiAntiNickThief)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiAntiNickThief, bCheck);
				if (m_htiEmptyNick)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiEmptyNick, bCheck);
				if (m_htiFakeEmule)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFakeEmule, bCheck);
				if (m_htiLeecherName)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLeecherName, bCheck);
				if (m_htiCommunityCheck)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiCommunityCheck, bCheck);
				if (m_htiHexCheck)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiHexCheck, bCheck);
				if (m_htiEmcrypt)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiEmcrypt, bCheck);
				if (m_htiBadInfo)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadInfo, bCheck);
				if (m_htiBadHello)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiBadHello, bCheck);
				if (m_htiSnafu)				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSnafu, bCheck);
				if (m_htiExtraBytes)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiExtraBytes, bCheck);
				if (m_htiNickChanger)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiNickChanger, bCheck);
				if (m_htiFileFaker)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFileFaker, bCheck);
				if (m_htiVagaa)				m_ctrlTreeOptions.SetCheckBoxEnable(m_htiVagaa, bCheck);

				// ==> Reduce Score for leecher - Stulle
				if(m_htiPunishmentGroup)	m_ctrlTreeOptions.SetGroupEnable(m_htiPunishmentGroup, bCheck);
				if(bCheck) m_ctrlTreeOptions.Expand(m_htiPunishmentGroup, TVE_EXPAND);
				else m_ctrlTreeOptions.Expand(m_htiPunishmentGroup, TVE_COLLAPSE);
				if(bCheck) m_ctrlTreeOptions.Expand(m_htiReduce, TVE_EXPAND);
				else m_ctrlTreeOptions.Expand(m_htiReduce, TVE_COLLAPSE);
				// <== Reduce Score for leecher - Stulle
				if(m_htiNoBadPushing)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiNoBadPushing, bCheck); // Disable PS/PBF for leechers [Stulle/idea by sfrqlxert] - Stulle
			}
		}
		// <== ban systems optional - Stulle

		// ==> Quick start [TPT] - Stulle
		if (m_htiQuickStart && pton->hItem == m_htiQuickStart)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiQuickStart, bCheck))
			{
				if (m_htiQuickStartAfterIPChange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartAfterIPChange, bCheck);
			}
		}
		// <== Quick start [TPT] - Stulle

		// ==> CPU/MEM usage [$ick$/Stulle] - Stulle
		if (m_htiSysInfo && pton->hItem == m_htiSysInfo)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiSysInfo, bCheck))
			{
				if (m_htiSysInfoGlobal)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSysInfoGlobal, bCheck);
			}
		}
		// <== CPU/MEM usage [$ick$/Stulle] - Stulle

		// ==> Automatic shared files updater [MoNKi] - Stulle
#ifdef ASFU
		if (m_htiAutoSharedUpdater && pton->hItem == m_htiAutoSharedUpdater)
		{
			if (m_ctrlTreeOptions.GetCheckBox(m_htiAutoSharedUpdater, bCheck))
			{
				if (m_htiSingleSharedDirUpdater)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiSingleSharedDirUpdater, bCheck);
			}
		}
#endif
		// <== Automatic shared files updater [MoNKi] - Stulle

		SetModified();
	}
	return 0;
}

void CPPgStulle::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
	// ==> push small files [sivka] - Stulle
	if( pScrollBar->GetSafeHwnd() == m_ctlPushSmallSize.m_hWnd )
		ShowPushSmallFileValues();
	// <== push small files [sivka] - Stulle

	SetModified(TRUE);
//	UpdateData(false); 
//	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ==> push small files [sivka] - Stulle
void CPPgStulle::ShowPushSmallFileValues()
{
	GetDlgItem(IDC_PUSHSMALL)->SetWindowText(CastItoXBytes(float(m_ctlPushSmallSize.GetPos()<<10)));
}
// <== push small files [sivka] - Stulle

void CPPgStulle::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgStulle::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgStulle::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}