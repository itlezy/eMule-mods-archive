// PpgMagicAngel.cpp : implementation file
// by sFrQlXeRt
//

#include "stdafx.h"
#include "emule.h"
#include "PPgMagicAngel.h"
#include "emuledlg.h"
#include "UserMsgs.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CPPgMagicAngel dialog

IMPLEMENT_DYNAMIC(CPPgMagicAngel, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgMagicAngel, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
END_MESSAGE_MAP()

CPPgMagicAngel::CPPgMagicAngel()
	: CPropertyPage(CPPgMagicAngel::IDD)
	
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bUseIntelligentFlush = false; // ==> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt

	m_bInitializedTreeOpts = false;

	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	m_htiEmulator = NULL;
	m_htiEnableMLDonkey = NULL;
	m_htiEnableeDonkey = NULL;
	m_htiEnableeDonkeyHybrid = NULL;
	m_htiEnableShareaza = NULL;
	m_htiEnableLphant = NULL;
	m_htiEnableCommunityNickAddons = NULL;
	m_htiLogEmulator = NULL;
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	m_htiQuickStart = NULL;
	m_htiQuickStartEnable = NULL;
	m_htiQuickStartTime = NULL;
	m_htiQuickStartTimePerFile = NULL;
	m_htiQuickMaxConperFive = NULL;
	m_htiQuickMaxHalfOpen = NULL;
	m_htiQuickMaxConnections = NULL;
	m_htiQuickStartOnIPChange = NULL;
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	m_htiReleaseBoostGroup = NULL;
	m_htiEnableReleaseBoost = NULL;
	m_htiReleaseBoost = NULL;
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	m_htiFilePushTweaks = NULL;
	m_htiPushSmallFiles = NULL;
	m_htiPushSmallFilesSize = NULL;
	m_htiPushRareFiles = NULL;
	m_htiPushRareFilesValue = NULL;
	m_htiPushRatioFiles = NULL;
	m_htiPushRatioFilesValue = NULL;
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	m_htiReaskTweaks = NULL;
	m_htiReaskSourcesAfterIpChange = NULL; // => RReask Sources after IP Change v4 [Xman] - sFrQlXeRt
	m_htiSpreadReask = NULL; // => manual enable spread reask - sFrQlXeRt
	m_htiReaskTime = NULL; // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	// <== Reask Tweaks - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	m_htiColoredUpload = NULL;
	m_htiNoColoredUpload = NULL;
	m_htiColorQueueranks = NULL;
	m_htiColorFSandPS = NULL;
	m_htiColorQueue = NULL;
	m_htiColorKnownClients = NULL;
	m_htiDontShowMorphColors = NULL;
	// <== Colored Upload - sFrQlXeRt

	m_htiOtherSettings = NULL;
	m_htiUseIntelligentFlush = NULL; //==MagicAngel=> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	m_htiReconnectKadOnIpChange = NULL; //==sFrQlXeRt=> Reconncect Kad on IP change
}

CPPgMagicAngel::~CPPgMagicAngel()
{
}

void CPPgMagicAngel::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAGICANGEL_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgEmulator = 8; // => Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
		int	iImgQuickStart = 8; // => Quickstart [DavidXanatos] - sFrQlXeRt
		int iImgReleaseBoost = 8; // => Release Boost (icon from NeoMule) - sFrQlXeRt
		int	iImgPushFiles = 0; // => Push Files [sivka/NeoMule] - sFrQlXeRt
		int	iImgReaskTweaks = 8; // => Reask Tweaks (icon from NeoMule) - sFrQlXeRt
		int iImgFunnyNick = 8; // => FunnyNick Tag [Stulle] - sFrQlXeRt
		int	iImgColoredUpload = 8; // => Colored Upload (icon from NeoMule) - sFrQlXeRt
		int iImgOtherSettings = 8; // => Other Settings(icon from NeoMule) - sFrQlXeRt
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgEmulator = piml->Add(CTempIconLoader(_T("EMULATOR"))); // => Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
			iImgQuickStart = piml->Add(CTempIconLoader(_T("QUICKSTART"))); // => Quickstart [DavidXanatos] - sFrQlXeRt 
			iImgReleaseBoost = piml->Add(CTempIconLoader(_T("RELEASEBOOST"))); // => Release Boost (icon from NeoMule) - sFrQlXeRt
			iImgPushFiles = piml->Add(CTempIconLoader(_T("PUSHFILES"))); // => Push Files [sivka/NeoMule] - sFrQlXeRt
			iImgReaskTweaks = piml->Add(CTempIconLoader(_T("REASK"))); // => Reask Tweaks (icon from NeoMule) - sFrQlXeRt
			iImgFunnyNick = piml->Add(CTempIconLoader(_T("FUNNYNICK"))); // => FunnyNick Tag [Stulle] - sFrQlXeRt
			iImgColoredUpload = piml->Add(CTempIconLoader(_T("COLOREDUPLOAD"))); // => Colored Upload (icon from NeoMule) - sFrQlXeRt
			iImgOtherSettings = piml->Add(CTempIconLoader(_T("OTHERSETTINGS"))); // => Other Settings(icon from NeoMule) - sFrQlXeRt
		}

		// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
		m_htiEmulator = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EMULATOR), iImgEmulator, TVI_ROOT);
		m_htiEnableMLDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMUMLDONKEY), m_htiEmulator, EmuMLDonkey);
		m_htiEnableeDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMUEDONKEY), m_htiEmulator, EmueDonkey);
		m_htiEnableeDonkeyHybrid = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMUEDONKEYHYBRID), m_htiEmulator, EmueDonkeyHybrid);
		m_htiEnableShareaza = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMUSHAREAZA2), m_htiEmulator, EmuShareaza);
		m_htiEnableLphant = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULPHANT), m_htiEmulator, EmuLphant);
		m_htiEnableCommunityNickAddons = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMUCOMMUNITYNICKADDOS), m_htiEmulator, EmuCommunityNickAddons);
		m_htiLogEmulator = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOGEMULATOR), m_htiEmulator, m_bLogEmulator);
		// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

		// ==> Quickstart [DavidXanatos] - sFrQlXeRt
		m_htiQuickStart = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_QUICKSTART), iImgQuickStart, TVI_ROOT);
		m_htiQuickStartEnable = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICKSTART_ENABLE), m_htiQuickStart, m_bQuickStart);
		m_htiQuickStartTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICKSTART_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartTimePerFile = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICKSTART_TIME_PER_FILE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickStartTimePerFile, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxConperFive = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICKSTART_MAXPER5), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxConperFive, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxHalfOpen = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICKSTART_MAXHALF), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxHalfOpen, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickMaxConnections = m_ctrlTreeOptions.InsertItem(GetResString(IDS_QUICKSTART_MAXCON), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiQuickStart);
		m_ctrlTreeOptions.AddEditBox(m_htiQuickMaxConnections, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiQuickStartOnIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_QUICKSTART_ONIP), m_htiQuickStart, m_bQuickStartOnIPChange);
		// <== Quickstart [DavidXanatos] - sFrQlXeRt

		// ==> Release Boost - sFrQlXeRt
		m_htiReleaseBoostGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RELEASE_BOOST), iImgReleaseBoost, TVI_ROOT);
		m_htiEnableReleaseBoost = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLE_RELEASE_BOOST), m_htiReleaseBoostGroup, m_bIsReleaseBoost);
		m_htiReleaseBoost = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RELEASE_BOOST), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReleaseBoostGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiReleaseBoost, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Release Boost - sFrQlXeRt

		// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
		m_htiFilePushTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PUSH_FILES), iImgPushFiles, TVI_ROOT);
		m_htiPushSmallFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_SMALL_FILES), m_htiFilePushTweaks, m_bPushSmallFiles);
		m_htiPushSmallFilesSize = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PUSH_SMALL_FILES_SIZE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFilePushTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiPushSmallFilesSize, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPushRareFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_RARE_FILES), m_htiFilePushTweaks, m_bPushRareFiles);
		m_htiPushRareFilesValue = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PUSH_RARE_FILES_VALUE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFilePushTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiPushRareFilesValue, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPushRatioFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUSH_RATIO_FILES), m_htiFilePushTweaks, m_bPushRatioFiles);
		m_htiPushRatioFilesValue = m_ctrlTreeOptions.InsertItem(GetResString(IDS_PUSH_RATIO_FILES_VALUE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFilePushTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiPushRatioFilesValue, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Push Files [sivka/NeoMule] - sFrQlXeRt

		// ==> Reask Tweaks - sFrQlXeRt
		m_htiReaskTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_REASK_TWEAKS), iImgReaskTweaks, TVI_ROOT);
		m_htiReaskSourcesAfterIpChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_REASK_AFTER_IP_CHANGE), m_htiReaskTweaks, m_bReaskSourcesAfterIpChange); // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
		m_htiSpreadReask = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPREAD_REASK), m_htiReaskTweaks, m_bSpreadReask); // => manual enable spread reask - sFrQlXeRt
		// ==> Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
		m_htiReaskTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REASK_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReaskTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiReaskTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
		// <== Reask Tweaks - sFrQlXeRt

		// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
		m_htiFnTag = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FN_TAG), iImgFunnyNick, TVI_ROOT);
		m_htiNoTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_FN_TAG), m_htiFnTag, m_iFnTag == 0);
		m_htiShortTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHORT_FN_TAG), m_htiFnTag, m_iFnTag == 1);
		m_htiFullTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_FULL_FN_TAG), m_htiFnTag, m_iFnTag == 2);
		m_htiCustomTag = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_CUSTOM_FN_TAG),m_htiFnTag,m_iFnTag == 3);
		m_htiFnCustomTag = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SET_CUSTOM_FN_TAG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCustomTag);
		m_ctrlTreeOptions.AddEditBox(m_htiFnCustomTag, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiFnTagAtEnd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FN_TAG_AT_END), m_htiFnTag, m_bFnTagAtEnd);
		m_ctrlTreeOptions.Expand(m_htiCustomTag, TVE_EXPAND);
		// <== FunnyNick Tag [Stulle] - sFrQlXeRt

		// ==> Colored Upload - sFrQlXeRt
		m_htiColoredUpload = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_COLORED_UPLOAD), iImgColoredUpload, TVI_ROOT);
		m_htiNoColoredUpload = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_COLORED_UPLOAD), m_htiColoredUpload, m_iColoredUpload == 0);
		m_htiColorQueueranks = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COLOR_QUEUERANKS), m_htiColoredUpload, m_iColoredUpload == 1);
		m_htiColorFSandPS = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COLOR_FS_AND_PS), m_htiColoredUpload, m_iColoredUpload == 2);
		m_htiColorQueue = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COLOR_QUEUE), m_htiColoredUpload, m_bColorQueue);
		m_htiColorKnownClients = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_COLOR_KNOWNCLIENTS), m_htiColoredUpload, m_bColorKnownClients);
		m_htiDontShowMorphColors = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_MORPH_COLORS), m_htiColoredUpload, m_bDontShowMorphColors);
		// <== Colored Upload - sFrQlXeRt

		m_htiOtherSettings = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_OTHER_SETTINGS), iImgOtherSettings, TVI_ROOT);
		m_htiUseIntelligentFlush = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INTELLIGENT_FLUSH), m_htiOtherSettings, m_bUseIntelligentFlush); //==MagicAngel=> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
		m_htiReconnectKadOnIpChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RECONNECT_KAD), m_htiOtherSettings, m_bReconnectKadOnIpChange); //==sFrQlXeRt=> Reconncect Kad on IP change

		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}
	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableMLDonkey, EmuMLDonkey);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableeDonkey, EmueDonkey);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableeDonkeyHybrid, EmueDonkeyHybrid);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableShareaza, EmuShareaza);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableLphant, EmuLphant);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableCommunityNickAddons, EmuCommunityNickAddons);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiLogEmulator, m_bLogEmulator);
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickStartEnable, m_bQuickStart);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickStartTime, m_iQuickStartTime);
	DDV_MinMaxInt(pDX, m_iQuickStartTime, 5, 25);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickStartTimePerFile, m_iQuickStartTimePerFile);
	DDV_MinMaxInt(pDX, m_iQuickStartTimePerFile, 30, 150);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickMaxConperFive, m_iQuickMaxConperFive);
	DDV_MinMaxInt(pDX, m_iQuickMaxConperFive, 5, INT_MAX);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickMaxHalfOpen, m_iQuickMaxHalfOpen);
	DDV_MinMaxInt(pDX, m_iQuickMaxHalfOpen, 5, INT_MAX);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickMaxConnections, m_iQuickMaxConnections);
	DDV_MinMaxInt(pDX, m_iQuickMaxConnections, 5, INT_MAX);
	if (m_htiQuickStartOnIPChange) DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiQuickStartOnIPChange, m_bQuickStartOnIPChange);
	if (m_htiQuickStartOnIPChange) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartOnIPChange, m_bQuickStart);
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiEnableReleaseBoost, m_bIsReleaseBoost);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiReleaseBoost, m_iGetReleaseBoost);
	DDV_MinMaxInt(pDX, m_iGetReleaseBoost, 2, 20);
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiPushSmallFiles, m_bPushSmallFiles);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiPushSmallFilesSize, m_iPushSmallFilesSize);
	DDV_MinMaxInt(pDX, m_iPushSmallFilesSize, 1024, 18432);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiPushRareFiles, m_bPushRareFiles);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiPushRareFilesValue, m_iPushRareFilesValue);
	DDV_MinMaxInt(pDX, m_iPushRareFilesValue, 4, 25);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiPushRatioFiles, m_bPushRatioFiles);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiPushRatioFilesValue, m_iPushRatioFilesValue);
	DDV_MinMaxInt(pDX, m_iPushRatioFilesValue, 2, 20);
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiReaskSourcesAfterIpChange, m_bReaskSourcesAfterIpChange); // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiSpreadReask, m_bSpreadReask); // => manual enable spread reask - sFrQlXeRt
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiReaskTime, m_iReaskTime); // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	DDV_MinMaxInt(pDX, m_iReaskTime, 29, 57);
	// <== Reask Tweaks - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	DDX_TreeRadio(pDX, IDC_MAGICANGEL_OPTS, m_htiFnTag, (int &)m_iFnTag);
	DDX_TreeEdit(pDX, IDC_MAGICANGEL_OPTS, m_htiFnCustomTag, m_sFnCustomTag);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiFnTagAtEnd, m_bFnTagAtEnd);
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	DDX_TreeRadio(pDX, IDC_MAGICANGEL_OPTS, m_htiColoredUpload, m_iColoredUpload);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiColorQueue, m_bColorQueue);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiColorKnownClients, m_bColorKnownClients);
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiDontShowMorphColors, m_bDontShowMorphColors);
	// <== Colored Upload - sFrQlXeRt

	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiUseIntelligentFlush, m_bUseIntelligentFlush); //==MagicAngel=> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_MAGICANGEL_OPTS, m_htiReconnectKadOnIpChange, m_bReconnectKadOnIpChange); //==sFrQlXeRt=> Reconncect Kad on IP change
}


BOOL CPPgMagicAngel::OnInitDialog()
{
	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	EmuMLDonkey      = thePrefs.EmuMLDonkey;
	EmueDonkey       = thePrefs.EmueDonkey;
	EmueDonkeyHybrid = thePrefs.EmueDonkeyHybrid;
	EmuShareaza      = thePrefs.EmuShareaza;
	EmuLphant		 = thePrefs.EmuLphant;
	EmuCommunityNickAddons		 = thePrefs.EmuCommunityNickAddons;
	m_bLogEmulator   = thePrefs.m_bLogEmulator;
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	m_bQuickStart		= thePrefs.UseQuickStart();
	m_iQuickStartTime	= thePrefs.GetQuickStartMaxTime();
	m_iQuickStartTimePerFile = thePrefs.GetQuickStartTimePerFile();
	m_iQuickMaxConperFive = thePrefs.GetQuickStartConperFive();
	m_iQuickMaxHalfOpen	= thePrefs.GetQuickStartHalfOpen();
	m_iQuickMaxConnections = thePrefs.GetQuickStartMaxConnections();
	if (thePrefs.UseQuickStart())
		m_bQuickStartOnIPChange = thePrefs.UseQuickStartOnIPChange();
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	m_bIsReleaseBoost = thePrefs.IsReleaseBoost();
	m_iGetReleaseBoost = (int)(thePrefs.GetReleaseBoost());
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	m_bPushSmallFiles = thePrefs.IsPushSmallFiles();
	m_iPushSmallFilesSize = thePrefs.GetPushSmallFilesSize();
	m_bPushRareFiles = thePrefs.IsPushRareFiles();
	m_iPushRareFilesValue = thePrefs.GetPushRareFilesValue();
	m_bPushRatioFiles = thePrefs.IsPushRatioFiles();
	m_iPushRatioFilesValue = thePrefs.GetPushRatioFilesValue();
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	m_bReaskSourcesAfterIpChange = thePrefs.IsRASAIC(); // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
	// ==> manual enable spread reask - sFrQlXeRt
	m_bSpreadReask = thePrefs.SpreadReask(); 
	if(thePrefs.SpreadReask())
		m_iReaskTime = (thePrefs.GetReAskTimeDif() + FILEREASKTIME)/60000;  // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	else
		m_iReaskTime = thePrefs.GetReAskTimeDif()/60000;
	// <== manual enable spread reask - sFrQlXeRt
	// <== Reask Tweaks - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	m_iFnTag = thePrefs.GetFnTag();
	m_sFnCustomTag = thePrefs.m_sFnCustomTag;
	m_bFnTagAtEnd = thePrefs.GetFnTagAtEnd();
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	m_iColoredUpload = thePrefs.ColoredUpload();
	m_bColorQueue = thePrefs.ColorQueue();
	m_bColorKnownClients = thePrefs.ColorKnownClients();
	m_bDontShowMorphColors = thePrefs.DontShowMorphColors();
	// <== Colored Upload - sFrQlXeRt

	m_bUseIntelligentFlush = thePrefs.UseIntelligentFlush(); //==MagicAngel=> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	m_bReconnectKadOnIpChange = thePrefs.ReconnectKadOnIpChange(); //==sFrQlXeRt=> Reconncect Kad on IP change

	CPropertyPage::OnInitDialog();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgMagicAngel::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgMagicAngel::OnApply()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	thePrefs.EmuMLDonkey		= EmuMLDonkey;
	thePrefs.EmueDonkey			= EmueDonkey;
	thePrefs.EmueDonkeyHybrid	= EmueDonkeyHybrid;
	thePrefs.EmuShareaza		= EmuShareaza;
	thePrefs.EmuLphant          = EmuLphant;
	thePrefs.EmuCommunityNickAddons          = EmuCommunityNickAddons;
	thePrefs.m_bLogEmulator		= m_bLogEmulator;
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	thePrefs.m_bQuickStart = m_bQuickStart;
	thePrefs.m_iQuickStartTime = m_iQuickStartTime;
	thePrefs.m_iQuickStartTimePerFile = m_iQuickStartTimePerFile;
	thePrefs.m_iQuickMaxConperFive = m_iQuickMaxConperFive;
	thePrefs.m_iQuickMaxHalfOpen = m_iQuickMaxHalfOpen;
	thePrefs.m_iQuickMaxConnections = m_iQuickMaxConnections;
	if (thePrefs.UseQuickStart())
		thePrefs.m_bQuickStartOnIPChange = m_bQuickStartOnIPChange;
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	thePrefs.m_bIsReleaseBoost = m_bIsReleaseBoost;
	thePrefs.m_iGetReleaseBoost = (uint16)m_iGetReleaseBoost;
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	thePrefs.m_bPushSmallFiles = m_bPushSmallFiles;
	thePrefs.m_iPushSmallFilesSize = m_iPushSmallFilesSize;
	thePrefs.m_bPushRareFiles = m_bPushRareFiles;
	thePrefs.m_iPushRareFilesValue = m_iPushRareFilesValue;
	thePrefs.m_bPushRatioFiles = m_bPushRatioFiles;
	thePrefs.m_iPushRatioFilesValue = m_iPushRatioFilesValue;
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	thePrefs.m_breaskSourceAfterIPChange = m_bReaskSourcesAfterIpChange; // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
	thePrefs.m_bSpreadReask = m_bSpreadReask; // => manual enable spread reask - sFrQlXeRt
	if(thePrefs.SpreadReask())
		thePrefs.m_uReAskTimeDif = (m_iReaskTime-29)*60000; // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	else
		thePrefs.m_uReAskTimeDif = m_iReaskTime*60000;
	// <== Reask Tweaks - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	thePrefs.FnTagMode = (uint8)m_iFnTag;
	_stprintf (thePrefs.m_sFnCustomTag,_T("%s"), m_sFnCustomTag);
	thePrefs.m_bFnTagAtEnd = m_bFnTagAtEnd;
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	thePrefs.m_iColoredUpload = (uint8)m_iColoredUpload;
	thePrefs.m_bColorQueue = m_bColorQueue;
	thePrefs.m_bColorKnownClients = m_bColorKnownClients;
	thePrefs.m_bDontShowMorphColors = m_bDontShowMorphColors;
	// <== Colored Upload - sFrQlXeRt

	thePrefs.m_bUseIntelligentFlush = m_bUseIntelligentFlush; //==> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	thePrefs.m_bReconnectKadOnIpChange = m_bReconnectKadOnIpChange; //==sFrQlXeRt=> Reconncect Kad on IP change

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgMagicAngel::Localize(void)
{
	if(m_hWnd)
	{
		GetDlgItem(IDC_WARNINGMORPH)->SetWindowText(GetResString(IDS_MAGICANGEL_INFO));

		// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
		if (m_htiEnableMLDonkey) m_ctrlTreeOptions.SetItemText(m_htiEnableMLDonkey, GetResString(IDS_EMUMLDONKEY));
		if (m_htiEnableeDonkey) m_ctrlTreeOptions.SetItemText(m_htiEnableeDonkey, GetResString(IDS_EMUEDONKEY));
		if (m_htiEnableeDonkeyHybrid) m_ctrlTreeOptions.SetItemText(m_htiEnableeDonkeyHybrid, GetResString(IDS_EMUEDONKEYHYBRID));
		if (m_htiEnableShareaza) m_ctrlTreeOptions.SetItemText(m_htiEnableShareaza, GetResString(IDS_EMUSHAREAZA2));
		if (m_htiEnableLphant) m_ctrlTreeOptions.SetItemText(m_htiEnableLphant, GetResString(IDS_EMULPHANT));
		if (m_htiEnableCommunityNickAddons) m_ctrlTreeOptions.SetItemText(m_htiEnableCommunityNickAddons, GetResString(IDS_EMUCOMMUNITYNICKADDOS));
		if (m_htiLogEmulator) m_ctrlTreeOptions.SetItemText(m_htiLogEmulator, GetResString(IDS_LOGEMULATOR));
		// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

		// ==> Quickstart [DavidXanatos] - sFrQlXeRt
		if (m_htiQuickStartEnable) m_ctrlTreeOptions.SetItemText(m_htiQuickStartEnable, GetResString(IDS_QUICKSTART_ENABLE));
		if (m_htiQuickStartTime) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartTime, GetResString(IDS_QUICKSTART_TIME));
		if (m_htiQuickStartTimePerFile) m_ctrlTreeOptions.SetEditLabel(m_htiQuickStartTimePerFile, GetResString(IDS_QUICKSTART_TIME_PER_FILE));
		if (m_htiQuickMaxConperFive) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxConperFive, GetResString(IDS_QUICKSTART_MAXPER5));
		if (m_htiQuickMaxHalfOpen) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxHalfOpen, GetResString(IDS_QUICKSTART_MAXHALF));
		if (m_htiQuickMaxConnections) m_ctrlTreeOptions.SetEditLabel(m_htiQuickMaxConnections, GetResString(IDS_QUICKSTART_MAXCON));
		if (m_htiQuickStartOnIPChange) m_ctrlTreeOptions.SetItemText(m_htiQuickStartOnIPChange, GetResString(IDS_QUICKSTART_ONIP));
		// <== Quickstart [DavidXanatos] - sFrQlXeRt

		// ==> Release Boost - sFrQlXeRt
		if (m_htiEnableReleaseBoost) m_ctrlTreeOptions.SetItemText(m_htiEnableReleaseBoost, GetResString(IDS_ENABLE_RELEASE_BOOST));
		if (m_htiReleaseBoost) m_ctrlTreeOptions.SetEditLabel(m_htiReleaseBoost, GetResString(IDS_RELEASE_BOOST));
		// <== Release Boost - sFrQlXeRt

		// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
		if (m_htiPushSmallFiles) m_ctrlTreeOptions.SetItemText(m_htiPushSmallFiles, GetResString(IDS_PUSH_SMALL_FILES));
		if (m_htiPushSmallFilesSize) m_ctrlTreeOptions.SetEditLabel(m_htiPushSmallFilesSize, GetResString(IDS_PUSH_SMALL_FILES_SIZE));
		if (m_htiPushRareFiles) m_ctrlTreeOptions.SetItemText(m_htiPushRareFiles, GetResString(IDS_PUSH_RARE_FILES));
		if (m_htiPushRareFilesValue) m_ctrlTreeOptions.SetEditLabel(m_htiPushRareFilesValue, GetResString(IDS_PUSH_RARE_FILES_VALUE));
		if (m_htiPushRatioFiles) m_ctrlTreeOptions.SetItemText(m_htiPushRatioFiles, GetResString(IDS_PUSH_RATIO_FILES));
		if (m_htiPushRatioFilesValue) m_ctrlTreeOptions.SetEditLabel(m_htiPushRatioFilesValue, GetResString(IDS_PUSH_RATIO_FILES_VALUE));
		// <== Push Files [sivka/NeoMule] - sFrQlXeRt

		// ==> Reask Tweaks - sFrQlXeRt
		if (m_htiReaskTweaks) m_ctrlTreeOptions.SetItemText(m_htiReaskTweaks, GetResString(IDS_REASK_TWEAKS));
		if (m_htiReaskSourcesAfterIpChange) m_ctrlTreeOptions.SetItemText(m_htiReaskSourcesAfterIpChange, GetResString(IDS_REASK_AFTER_IP_CHANGE)); // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
		if (m_htiSpreadReask) m_ctrlTreeOptions.SetItemText(m_htiSpreadReask, GetResString(IDS_SPREAD_REASK)); // => manual enable spread reask - sFrQlXeRt
		if (m_htiReaskTime) m_ctrlTreeOptions.SetEditLabel(m_htiReaskTime, GetResString(IDS_REASK_TIME)); // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
		// <== Reask Tweaks - sFrQlXeRt

		// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
		if (m_htiFnCustomTag) m_ctrlTreeOptions.SetEditLabel(m_htiFnCustomTag, GetResString(IDS_SET_CUSTOM_FN_TAG));
		if (m_htiFnTagAtEnd) m_ctrlTreeOptions.SetItemText(m_htiFnTagAtEnd, GetResString(IDS_FN_TAG_AT_END));
		// <== FunnyNick Tag [Stulle] - sFrQlXeRt

		// ==> Colored Upload - sFrQlXeRt
		if (m_htiColorQueue) m_ctrlTreeOptions.SetItemText(m_htiColorQueue, GetResString(IDS_COLOR_QUEUE));
		if (m_htiColorKnownClients) m_ctrlTreeOptions.SetItemText(m_htiColorKnownClients, GetResString(IDS_COLOR_KNOWNCLIENTS));
		if (m_htiDontShowMorphColors) m_ctrlTreeOptions.SetItemText(m_htiDontShowMorphColors, GetResString(IDS_MORPH_COLORS));
		// <== Colored Upload - sFrQlXeRt

		if (m_htiUseIntelligentFlush) m_ctrlTreeOptions.SetItemText(m_htiUseIntelligentFlush, GetResString(IDS_INTELLIGENT_FLUSH)); //==> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
		if (m_htiReconnectKadOnIpChange) m_ctrlTreeOptions.SetItemText(m_htiReconnectKadOnIpChange, GetResString(IDS_RECONNECT_KAD)); //==sFrQlXeRt=> Reconncect Kad on IP change
	}

}

void CPPgMagicAngel::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	m_htiEmulator = NULL;
	m_htiEnableMLDonkey = NULL;
	m_htiEnableeDonkey = NULL;
	m_htiEnableeDonkeyHybrid = NULL;
	m_htiEnableShareaza = NULL;
	m_htiEnableLphant = NULL;
	m_htiEnableCommunityNickAddons = NULL;
	m_htiLogEmulator = NULL;
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	m_htiQuickStart = NULL;
	m_htiQuickStartEnable = NULL;
	m_htiQuickStartTime = NULL;
	m_htiQuickStartTimePerFile = NULL;
	m_htiQuickMaxConperFive = NULL;
	m_htiQuickMaxHalfOpen = NULL;
	m_htiQuickMaxConnections = NULL;
	m_htiQuickStartOnIPChange = NULL;
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	m_htiReleaseBoostGroup = NULL;
	m_htiEnableReleaseBoost = NULL;
	m_htiReleaseBoost = NULL;
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	m_htiFilePushTweaks = NULL;
	m_htiPushSmallFiles = NULL;
	m_htiPushSmallFilesSize = NULL;
	m_htiPushRareFiles = NULL;
	m_htiPushRareFilesValue = NULL;
	m_htiPushRatioFiles = NULL;
	m_htiPushRatioFilesValue = NULL;
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	m_htiReaskTweaks = NULL;
	m_htiReaskSourcesAfterIpChange = NULL; // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
	m_htiSpreadReask = NULL; // => manual enable spread reask - sFrQlXeRt
	m_htiReaskTime = NULL; // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	// <== Reask Tweaks - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	m_htiFnTag = NULL;
	m_htiNoTag = NULL;
	m_htiShortTag = NULL;
	m_htiFullTag = NULL;
	m_htiCustomTag = NULL;
	m_htiFnCustomTag = NULL;
	m_htiFnTagAtEnd = NULL;
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	m_htiColoredUpload = NULL;
	m_htiNoColoredUpload = NULL;
	m_htiColorQueueranks = NULL;
	m_htiColorFSandPS = NULL;
	m_htiColorQueue = NULL;
	m_htiColorKnownClients = NULL;
	m_htiDontShowMorphColors = NULL;
	// <== Colored Upload - sFrQlXeRt

	m_htiOtherSettings = NULL;
	m_htiUseIntelligentFlush = NULL; //==MagicAngel=> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	m_htiReconnectKadOnIpChange = NULL; //==sFrQlXeRt=> Reconncect Kad on IP change

	CPropertyPage::OnDestroy();
}

LRESULT CPPgMagicAngel::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_MAGICANGEL_OPTS){
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if (m_htiQuickStart && pton->hItem == m_htiQuickStart)
		{
			BOOL bCheck;
			if (m_ctrlTreeOptions.GetCheckBox(m_htiQuickStart, bCheck))
				if (m_htiQuickStartOnIPChange) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiQuickStartOnIPChange, bCheck);
		}
		SetModified();
	}
	return 0;
}

void CPPgMagicAngel::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);
	CString temp;
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgMagicAngel::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgMagicAngel::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgMagicAngel::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}