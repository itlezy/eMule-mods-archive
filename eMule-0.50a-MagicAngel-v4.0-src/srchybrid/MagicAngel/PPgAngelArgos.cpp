// PpgAngelArgos.cpp : implementation file
// by sFrQlXeRt
//

#include "stdafx.h"
#include "emule.h"
#include "PPgAngelArgos.h"
#include "emuledlg.h"
#include "UserMsgs.h"
#include "OtherFunctions.h"
#include "MagicAngel/DLP.h" // => DLP [Xman] - sFrQlXeRt
#include "MagicAngel/MagicDLP.h" // => MagicDLP - sFrQlXeRt


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CPPgAngelArgos dialog

IMPLEMENT_DYNAMIC(CPPgAngelArgos, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgAngelArgos, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_BN_CLICKED(IDC_DLP_RELOAD, OnBnClickedDlpReload) // => DLP [Xman] - sFrQlXeRt
	ON_BN_CLICKED(IDC_MDLP_RELOAD, OnBnClickedMDlpReload) // => MagicDLP - sFrQlXeRt
END_MESSAGE_MAP()

CPPgAngelArgos::CPPgAngelArgos()
	: CPropertyPage(CPPgAngelArgos::IDD)
	
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;

	m_htiGeneralOptions = NULL;
	m_htiDontBanFriends = NULL;
	m_htiInformLeechers = NULL; // => Inform Leechers - sFrQlXeRt
	m_htiInformLeechersText = NULL; // => Inform Leechers Text - evcz
	m_htiDisPSForLeechers = NULL;
	// ==> Anti Upload Protection - sFrQlXeRt
	m_htiAntiUploadProtection = NULL;
	m_htiUploadProtectionLimit = NULL;
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	m_htiLeecherModDetection = NULL;
	m_htiDetectModstrings = NULL;
	m_htiDetectUsernames = NULL;
	m_htiDetectUserhashes = NULL;
	m_htiGplBreakerPunishment = NULL;
	m_htiHardLeecherPunishment = NULL;
	m_htiSoftLeecherPunishment = NULL;
	m_htiBadModPunishment = NULL;
	m_htiIpBan = NULL;
	m_htiUploadBan = NULL;
	m_htiScore01 = NULL;
	m_htiScore02 = NULL;
	m_htiScore03 = NULL;
	m_htiScore04 = NULL;
	m_htiScore05 = NULL;
	m_htiScore06 = NULL;
	m_htiScore07 = NULL;
	m_htiScore08 = NULL;
	m_htiScore09 = NULL;
	m_htiNoBan = NULL;
	m_htiDetectWrongHello = NULL;
	m_htiWrongHelloPunishment = NULL;
	m_htiDetectBadHello = NULL;
	m_htiBadHelloPunishment = NULL;
	m_htiDetectCreditHack = NULL;
	m_htiCreditHackPunishment = NULL;
	m_htiDetectModThief = NULL;
	m_htiModThiefPunishment = NULL;
	m_htiDetectNickThief = NULL;
	m_htiNickThiefPunishment = NULL;
	m_htiDetectFakeEmule = NULL;
	m_htiFakeEmulePunishment = NULL;
	m_htiDetectGhostMod = NULL;
	m_htiGhostModPunishment = NULL;
	m_htiDetectSpam = NULL;
	m_htiSpamPunishment = NULL;
	m_htiDetectEmcrypt = NULL;
	m_htiEmcryptPunishment = NULL;
	m_htiDetectXSExploiter = NULL;
	m_htiXSExploiterPunishment = NULL;
	m_htiDetectFileFaker = NULL;
	m_htiFileFakerPunishment = NULL;
	
	m_htiAgressiveGroup = NULL;
	m_htiDetectAgressive = NULL;
	m_htiAgressiveTime = NULL;
	m_htiAgressiveCounter = NULL;
	m_htiAgressiveLog = NULL;
	m_htiAgressivePunishment = NULL;

	m_htiPunishDonkeys = NULL;
	m_htiPunishMlDonkey = NULL;
	m_htiPunishEdonkey = NULL;
	m_htiPunishEdonkeyHybrid = NULL;
	m_htiPunishShareaza = NULL;
	m_htiPunishLphant = NULL;
	m_htiPunishSuiFailed = NULL;
	m_htiDonkeyPunishment = NULL;
}

CPPgAngelArgos::~CPPgAngelArgos()
{
}

void CPPgAngelArgos::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANGELARGOS_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int	iImgGeneralOptions = 8;
		int iImgLeecherModDetection = 8;
		int iImgPunishment = 8;
		int	iImgAgressive = 8;
		int	iImgPunishDonkeys = 8;
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgGeneralOptions = piml->Add(CTempIconLoader(_T("ANGELARGOSGENERAL"))); // icon from NeoMule - sFrQlXeRt
			iImgLeecherModDetection = piml->Add(CTempIconLoader(_T("LEECHER")));
			iImgPunishment = piml->Add(CTempIconLoader(_T("ANGELARGOSPUNISHMENT"))); // icon from NeoMule - sFrQlXeRt
			iImgAgressive = piml->Add(CTempIconLoader(_T("SECURITY")));
			iImgPunishDonkeys = piml->Add(CTempIconLoader(_T("EMULATOR")));
		}

		m_htiGeneralOptions = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_ANGELARGOS_GENERAL), iImgGeneralOptions, TVI_ROOT);
		m_htiDontBanFriends = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DONT_BAN_FRIENDS), m_htiGeneralOptions, m_bDontBanFriends);
		m_htiInformLeechers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INFORM_LEECHERS), m_htiGeneralOptions, m_bInformLeechers); // => Inform Leechers - sFrQlXeRt
		m_htiDisPSForLeechers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DIS_PS_FOR_LEECHERS), m_htiGeneralOptions, m_bDisPSForLeechers);
		// ==> Inform Leechers Text - evcz
		m_htiInformLeechersText = m_ctrlTreeOptions.InsertItem(GetResString(IDS_INFORM_LEECHERS_TEXT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiInformLeechers);
		m_ctrlTreeOptions.AddEditBox(m_htiInformLeechersText, RUNTIME_CLASS(CTreeOptionsEdit));
		// <== Inform Leechers Text - evcz
		// ==> Anti Upload Protection - sFrQlXeRt
		m_htiAntiUploadProtection = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ANTI_UP_PROTECTION), m_htiGeneralOptions, m_bAntiUploadProtection);
		m_htiUploadProtectionLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UP_PROTECTION_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAntiUploadProtection);
		m_ctrlTreeOptions.AddEditBox(m_htiUploadProtectionLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// <== Anti Upload Protection - sFrQlXeRt
		// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
		m_htiAntiUploaderBanLimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UNBAN_UPLOADER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiGeneralOptions);
		m_ctrlTreeOptions.AddEditBox(m_htiAntiUploaderBanLimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAntiCase1 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_1), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 0);
		m_htiAntiCase2 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_2), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 1);
		m_htiAntiCase3 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANTI_CASE_3), m_htiAntiUploaderBanLimit, m_iAntiUploaderBanCase == 2);
		// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

		m_htiLeecherModDetection = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MOD_DETECTION), iImgLeecherModDetection, TVI_ROOT);
		m_htiDetectModstrings = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_MODSTRINGS), m_htiLeecherModDetection, m_bDetectModstrings); 
		m_htiDetectUsernames = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_USERNAMES), m_htiLeecherModDetection, m_bDetectUsernames);
		m_htiDetectUserhashes = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_USERHASHES), m_htiLeecherModDetection, m_bDetectUserhashes);
		m_htiGplBreakerPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_GPL_BREAKER_PUNISHMENT), iImgPunishment, m_htiLeecherModDetection);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 0);
		m_htiUploadBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_UPLOAD_BAN), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 1);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 2);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 3);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 4);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 5);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 6);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 7);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 8);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 9);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 10);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiGplBreakerPunishment, m_iGplBreakerPunishment == 11);
		m_htiHardLeecherPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_HARD_LEECHER_PUNISHMENT), iImgPunishment, m_htiLeecherModDetection);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 0);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 1);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 2);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 3);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 4);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 5);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 6);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 7);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 8);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 9);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiHardLeecherPunishment, m_iHardLeecherPunishment == 10);
		m_htiSoftLeecherPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SOFT_LEECHER_PUNISHMENT), iImgPunishment, m_htiLeecherModDetection);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 0);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 1);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 2);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 3);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 4);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 5);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 6);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment == 7);
		m_htiBadModPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_BAD_MOD_PUNISHMENT), iImgPunishment, m_htiLeecherModDetection);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiBadModPunishment, m_iBadModPunishment == 0);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiBadModPunishment, m_iBadModPunishment == 1);

		m_htiDetectWrongHello = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_WRONG_HELLO), TVI_ROOT, m_bDetectWrongHello);
		m_htiWrongHelloPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_WRONG_HELLO_PUNISHMENT), iImgPunishment, m_htiDetectWrongHello);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 0);
		m_htiUploadBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_UPLOAD_BAN), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 1);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 2);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 3);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 4);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 5);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 6);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 7);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 8);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 9);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 10);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiWrongHelloPunishment, m_iWrongHelloPunishment == 11);

		m_htiDetectBadHello = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_BADHELLO), TVI_ROOT, m_bDetectBadHello);
		m_htiBadHelloPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_BADHELLO_PUNISHMENT), iImgPunishment, m_htiDetectBadHello);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiBadHelloPunishment, m_iBadHelloPunishment == 0);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiBadHelloPunishment, m_iBadHelloPunishment == 1);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiBadHelloPunishment, m_iBadHelloPunishment == 2);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiBadHelloPunishment, m_iBadHelloPunishment == 3);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiBadHelloPunishment, m_iBadHelloPunishment == 4);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiBadHelloPunishment, m_iBadHelloPunishment == 5);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiBadHelloPunishment, m_iBadHelloPunishment == 6);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiBadHelloPunishment, m_iBadHelloPunishment == 7);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiBadHelloPunishment, m_iBadHelloPunishment == 8);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiBadHelloPunishment, m_iBadHelloPunishment == 9);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiBadHelloPunishment, m_iBadHelloPunishment == 10);

		m_htiDetectCreditHack = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_CREDIT_HACK), TVI_ROOT, m_bDetectCreditHack);
		m_htiCreditHackPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CREDIT_HACK_PUNISHMENT), iImgPunishment, m_htiDetectCreditHack);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiCreditHackPunishment, m_iCreditHackPunishment == 0);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiCreditHackPunishment, m_iCreditHackPunishment == 1);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiCreditHackPunishment, m_iCreditHackPunishment == 2);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiCreditHackPunishment, m_iCreditHackPunishment == 3);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiCreditHackPunishment, m_iCreditHackPunishment == 4);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiCreditHackPunishment, m_iCreditHackPunishment == 5);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiCreditHackPunishment, m_iCreditHackPunishment == 6);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiCreditHackPunishment, m_iCreditHackPunishment == 7);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiCreditHackPunishment, m_iCreditHackPunishment == 8);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiCreditHackPunishment, m_iCreditHackPunishment == 9);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiCreditHackPunishment, m_iCreditHackPunishment == 10);

		m_htiDetectModThief = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_MOD_THIEF), TVI_ROOT, m_bDetectModThief);
		m_htiModThiefPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MOD_THIEF_PUNISHMENT), iImgPunishment, m_htiDetectModThief);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiModThiefPunishment, m_iModThiefPunishment == 0);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiModThiefPunishment, m_iModThiefPunishment == 1);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiModThiefPunishment, m_iModThiefPunishment == 2);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiModThiefPunishment, m_iModThiefPunishment == 3);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiModThiefPunishment, m_iModThiefPunishment == 4);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiModThiefPunishment, m_iModThiefPunishment == 5);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiModThiefPunishment, m_iModThiefPunishment == 6);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiModThiefPunishment, m_iModThiefPunishment == 7);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiModThiefPunishment, m_iModThiefPunishment == 8);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiModThiefPunishment, m_iModThiefPunishment == 9);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiModThiefPunishment, m_iModThiefPunishment == 10);

		m_htiDetectNickThief = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_NICK_THIEF), TVI_ROOT, m_bDetectNickThief);
		m_htiNickThiefPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_NICK_THIEF_PUNISHMENT), iImgPunishment, m_htiDetectNickThief);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiNickThiefPunishment, m_iNickThiefPunishment == 0);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiNickThiefPunishment, m_iNickThiefPunishment == 1);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiNickThiefPunishment, m_iNickThiefPunishment == 2);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiNickThiefPunishment, m_iNickThiefPunishment == 3);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiNickThiefPunishment, m_iNickThiefPunishment == 4);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiNickThiefPunishment, m_iNickThiefPunishment == 5);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiNickThiefPunishment, m_iNickThiefPunishment == 6);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiNickThiefPunishment, m_iNickThiefPunishment == 7);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiNickThiefPunishment, m_iNickThiefPunishment == 8);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiNickThiefPunishment, m_iNickThiefPunishment == 9);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiNickThiefPunishment, m_iNickThiefPunishment == 10);

		m_htiDetectFakeEmule = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_FAKE_EMULE), TVI_ROOT, m_bDetectFakeEmule);
		m_htiFakeEmulePunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FAKE_EMULE_PUNISHMENT), iImgPunishment, m_htiDetectFakeEmule);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 0);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 1);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 2);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 3);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 4);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 5);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 6);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 7);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 8);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 9);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiFakeEmulePunishment, m_iFakeEmulePunishment == 10);

		m_htiDetectGhostMod = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_GHOST_MOD), TVI_ROOT, m_bDetectGhostMod);
		m_htiGhostModPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_GHOST_MOD_PUNISHMENT), iImgPunishment, m_htiDetectGhostMod);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiGhostModPunishment, m_iGhostModPunishment == 0);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiGhostModPunishment, m_iGhostModPunishment == 1);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiGhostModPunishment, m_iGhostModPunishment == 2);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiGhostModPunishment, m_iGhostModPunishment == 3);

		m_htiDetectSpam = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_SPAM), TVI_ROOT, m_bDetectSpam);
		m_htiSpamPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SPAM_PUNISHMENT), iImgPunishment, m_htiDetectSpam);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiSpamPunishment, m_iSpamPunishment == 0);
		m_htiUploadBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_UPLOAD_BAN), m_htiSpamPunishment, m_iSpamPunishment == 1);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiSpamPunishment, m_iSpamPunishment == 2);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiSpamPunishment, m_iSpamPunishment == 3);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiSpamPunishment, m_iSpamPunishment == 4);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiSpamPunishment, m_iSpamPunishment == 5);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiSpamPunishment, m_iSpamPunishment == 6);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiSpamPunishment, m_iSpamPunishment == 7);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiSpamPunishment, m_iSpamPunishment == 8);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiSpamPunishment, m_iSpamPunishment == 9);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiSpamPunishment, m_iSpamPunishment == 10);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiSpamPunishment, m_iSpamPunishment == 11);
		
		m_htiDetectEmcrypt = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_EMCRYPT), TVI_ROOT, m_bDetectEmcrypt);
		m_htiEmcryptPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EMCRYPT_PUNISHMENT), iImgPunishment, m_htiDetectEmcrypt);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiEmcryptPunishment, m_iEmcryptPunishment == 0);
		m_htiUploadBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_UPLOAD_BAN), m_htiEmcryptPunishment, m_iEmcryptPunishment == 1);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiEmcryptPunishment, m_iEmcryptPunishment == 2);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiEmcryptPunishment, m_iEmcryptPunishment == 3);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiEmcryptPunishment, m_iEmcryptPunishment == 4);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiEmcryptPunishment, m_iEmcryptPunishment == 5);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiEmcryptPunishment, m_iEmcryptPunishment == 6);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiEmcryptPunishment, m_iEmcryptPunishment == 7);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiEmcryptPunishment, m_iEmcryptPunishment == 8);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiEmcryptPunishment, m_iEmcryptPunishment == 9);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiEmcryptPunishment, m_iEmcryptPunishment == 10);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiEmcryptPunishment, m_iEmcryptPunishment == 11);

		m_htiDetectXSExploiter = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_XS_EXPLOITER), TVI_ROOT, m_bDetectXSExploiter);
		m_htiXSExploiterPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_XSEXPLOITER_PUNISHMENT), iImgPunishment, m_htiDetectXSExploiter);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiXSExploiterPunishment, m_iXSExploiterPunishment == 0);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiXSExploiterPunishment, m_iXSExploiterPunishment == 1);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiXSExploiterPunishment, m_iXSExploiterPunishment == 2);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiXSExploiterPunishment, m_iXSExploiterPunishment == 3);

		m_htiDetectFileFaker = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_FILEFAKER), TVI_ROOT, m_bDetectFileFaker);
		m_htiFileFakerPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FILEFAKER_PUNISHMENT), iImgPunishment, m_htiDetectFileFaker);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiFileFakerPunishment, m_iFileFakerPunishment == 0);
		m_htiUploadBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_UPLOAD_BAN), m_htiFileFakerPunishment, m_iFileFakerPunishment == 1);
		m_htiScore01 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_01), m_htiFileFakerPunishment, m_iFileFakerPunishment == 2);
		m_htiScore02 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_02), m_htiFileFakerPunishment, m_iFileFakerPunishment == 3);
		m_htiScore03 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_03), m_htiFileFakerPunishment, m_iFileFakerPunishment == 4);
		m_htiScore04 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_04), m_htiFileFakerPunishment, m_iFileFakerPunishment == 5);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiFileFakerPunishment, m_iFileFakerPunishment == 6);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiFileFakerPunishment, m_iFileFakerPunishment == 7);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiFileFakerPunishment, m_iFileFakerPunishment == 8);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiFileFakerPunishment, m_iFileFakerPunishment == 9);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiFileFakerPunishment, m_iFileFakerPunishment == 10);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiFileFakerPunishment, m_iEmcryptPunishment == 11);

		m_htiAgressiveGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DETECT_AGRESSIVE), iImgAgressive, TVI_ROOT);
		m_htiDetectAgressive = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DETECT_AGRESSIVE), m_htiAgressiveGroup, m_bDetectAgressive);
		m_htiAgressiveTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AGRESSIVE_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAgressiveGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiAgressiveTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAgressiveCounter = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AGRESSIVE_COUNTER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAgressiveGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiAgressiveCounter, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAgressiveLog = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AGRESSIVE_LOG), m_htiAgressiveGroup, m_bAgressiveLog);
		m_htiAgressivePunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AGRESSIVE_PUNISHMENT), iImgPunishment, m_htiAgressiveGroup);
		m_htiIpBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_IP_BAN), m_htiAgressivePunishment, m_iAgressivePunishment == 0);
		m_htiScore05 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_05), m_htiAgressivePunishment, m_iAgressivePunishment == 1);
		m_htiScore06 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_06), m_htiAgressivePunishment, m_iAgressivePunishment == 2);
		m_htiScore07 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_07), m_htiAgressivePunishment, m_iAgressivePunishment == 3);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiAgressivePunishment, m_iAgressivePunishment == 4);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiAgressivePunishment, m_iAgressivePunishment == 5);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiAgressivePunishment, m_iAgressivePunishment == 6);
		
		m_htiPunishDonkeys = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PUNISH_DONKEYS), iImgPunishDonkeys, TVI_ROOT);
		m_htiPunishMlDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUNISH_MLDONKEY), m_htiPunishDonkeys, m_bPunishMlDonkey);
		m_htiPunishEdonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUNISH_EDONKEY), m_htiPunishDonkeys, m_bPunishEdonkey);
		m_htiPunishEdonkeyHybrid = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUNISH_EDONKEY_HYBRID), m_htiPunishDonkeys, m_bPunishEdonkeyHybrid);
		m_htiPunishShareaza = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUNISH_SHAREAZA), m_htiPunishDonkeys, m_bPunishShareaza);
		m_htiPunishLphant = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUNISH_LPHANT), m_htiPunishDonkeys, m_bPunishLphant);
		m_htiPunishSuiFailed = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PUNISH_SUI_FAILED), m_htiPunishDonkeys, m_bPunishSuiFailed);
		m_htiDonkeyPunishment = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DONKEY_PUNISHMENT), iImgPunishment, m_htiPunishDonkeys);
		m_htiScore08 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_08), m_htiDonkeyPunishment, m_iDonkeyPunishment == 0);
		m_htiScore09 = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SCORE_09), m_htiDonkeyPunishment, m_iDonkeyPunishment == 1);
		m_htiNoBan = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NO_BAN), m_htiDonkeyPunishment, m_iDonkeyPunishment == 2);


		m_ctrlTreeOptions.Expand(m_htiAntiUploadProtection, TVE_EXPAND); // => Anti Upload Protection - sFrQlXeRt
		m_ctrlTreeOptions.Expand(m_htiInformLeechersText, TVE_EXPAND); // ==> Inform Leechers Text - sFrQlXeRt
		m_ctrlTreeOptions.Expand(m_htiAntiUploaderBanLimit, TVE_EXPAND); // => Anti Uploader Ban [Stulle] - sFrQlXeRt
		m_ctrlTreeOptions.Expand(m_htiWrongHelloPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiBadHelloPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCreditHackPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiModThiefPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiNickThiefPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiFakeEmulePunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiGhostModPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiSpamPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiEmcryptPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiXSExploiterPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiFileFakerPunishment, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiAgressivePunishment, TVE_EXPAND);

		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}

	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDontBanFriends, m_bDontBanFriends);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiInformLeechers, m_bInformLeechers); // => Inform Leechers - sFrQlXeRt
	// ==> Inform Leechers Text - evcz
	DDX_TreeEdit(pDX, IDC_ANGELARGOS_OPTS, m_htiInformLeechersText, m_strInformLeechersText);
	DDV_MaxChars(pDX, m_strInformLeechersText, 30);
	// <== Inform Leechers Text - evcz
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDisPSForLeechers, m_bDisPSForLeechers);
	// ==> Anti Upload Protection - sFrQlXeRt
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiAntiUploadProtection, m_bAntiUploadProtection);
	DDX_TreeEdit(pDX, IDC_ANGELARGOS_OPTS, m_htiUploadProtectionLimit, m_iUploadProtectionLimit);
	DDV_MinMaxInt(pDX, m_iUploadProtectionLimit, 1000, 2800);
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	DDX_TreeEdit(pDX, IDC_ANGELARGOS_OPTS, m_htiAntiUploaderBanLimit, m_iAntiUploaderBanLimit);
	DDV_MinMaxInt(pDX, m_iAntiUploaderBanLimit, 0, 20000);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiAntiUploaderBanLimit, (int &)m_iAntiUploaderBanCase);
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	//==sFrQlXeRt=> Only enable Features that can be used
	// cache the values
	bool bBothDLPAvailable = theApp.dlp->IsDLPavailable() && theApp.MagicDLP->IsMagicDLPavailable();
	bool bDLPAvailable = theApp.dlp->IsDLPavailable();
	//<=sFrQlXeRt== Only enable Features that can be used

	if (m_htiDetectModstrings) DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectModstrings, m_bDetectModstrings);
	if (m_htiDetectModstrings) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectModstrings, bBothDLPAvailable);
	if (m_htiDetectUsernames) DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectUsernames, m_bDetectUsernames);
	if (m_htiDetectUsernames) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectUsernames, bBothDLPAvailable);
	if (m_htiDetectUserhashes) DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectUserhashes, m_bDetectUserhashes);
	if (m_htiDetectUserhashes) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectUserhashes, bDLPAvailable);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiGplBreakerPunishment, m_iGplBreakerPunishment);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiHardLeecherPunishment, m_iHardLeecherPunishment);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiSoftLeecherPunishment, m_iSoftLeecherPunishment);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiBadModPunishment, m_iBadModPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectWrongHello, m_bDetectWrongHello);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiWrongHelloPunishment, m_iWrongHelloPunishment);
	if (m_htiDetectBadHello) DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectBadHello, m_bDetectBadHello);
	if (m_htiDetectBadHello) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectBadHello, bDLPAvailable);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiBadHelloPunishment, m_iBadHelloPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectCreditHack, m_bDetectCreditHack);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiCreditHackPunishment, m_iCreditHackPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectModThief, m_bDetectModThief);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiModThiefPunishment, m_iModThiefPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectNickThief, m_bDetectNickThief);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiNickThiefPunishment, m_iNickThiefPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectFakeEmule, m_bDetectFakeEmule);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiFakeEmulePunishment, m_iFakeEmulePunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectGhostMod, m_bDetectGhostMod);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiGhostModPunishment, m_iGhostModPunishment);
	if (m_htiDetectSpam) DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectSpam, m_bDetectSpam);
	if (m_htiDetectSpam) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectSpam, bDLPAvailable);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiSpamPunishment, m_iSpamPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectEmcrypt, m_bDetectEmcrypt);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiEmcryptPunishment, m_iEmcryptPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectXSExploiter, m_bDetectXSExploiter);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiXSExploiterPunishment, m_iXSExploiterPunishment);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectFileFaker, m_bDetectFileFaker);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiFileFakerPunishment, m_iFileFakerPunishment);

	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiDetectAgressive, m_bDetectAgressive);
	DDX_TreeEdit(pDX, IDC_ANGELARGOS_OPTS, m_htiAgressiveTime, m_iAgressiveTime);
	DDV_MinMaxInt(pDX, m_iAgressiveTime, 5, 15);
	DDX_TreeEdit(pDX, IDC_ANGELARGOS_OPTS, m_htiAgressiveCounter, m_iAgressiveCounter);
	DDV_MinMaxInt(pDX, m_iAgressiveCounter, 3, 10);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiAgressiveLog, m_bAgressiveLog);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiAgressivePunishment, m_iAgressivePunishment);

	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiPunishMlDonkey, m_bPunishMlDonkey);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiPunishEdonkey, m_bPunishEdonkey);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiPunishEdonkeyHybrid, m_bPunishEdonkeyHybrid);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiPunishShareaza, m_bPunishShareaza);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiPunishLphant, m_bPunishLphant);
	DDX_TreeCheck(pDX, IDC_ANGELARGOS_OPTS, m_htiPunishSuiFailed, m_bPunishSuiFailed);
	DDX_TreeRadio(pDX, IDC_ANGELARGOS_OPTS, m_htiDonkeyPunishment, m_iDonkeyPunishment);
}

void CPPgAngelArgos::LoadSettings(void)
{
	CString buffer;

	// ==> DLP [Xman] - sFrQlXeRt
	if(theApp.dlp->IsDLPavailable())
	{
		buffer.Format(_T("antiLeech.dll (DLP v%u)"), theApp.dlp->GetDLPVersion());
		GetDlgItem(IDC_DLP_STATIC)->SetWindowText(buffer);
	}
	else
		GetDlgItem(IDC_DLP_STATIC)->SetWindowText(_T("DLP not loaded"));
	// <== DLP [Xman] - sFrQlXeRt

	// ==> MagicDLP  - sFrQlXeRt
	if(theApp.MagicDLP->IsMagicDLPavailable())
	{
		buffer.Format(_T("MagicAntiLeech.dll (MagicDLP v%u)"), theApp.MagicDLP->GetMagicDLPVersion());
		GetDlgItem(IDC_MDLP_STATIC)->SetWindowText(buffer);
	}
	else
		GetDlgItem(IDC_MDLP_STATIC)->SetWindowText(_T("MagicDLP not loaded"));
	// <== MagicDLP  - sFrQlXeRt

	//==sFrQlXeRt=> Only enable Features that can be used
	bool bBothDLPAvailable = theApp.dlp->IsDLPavailable() && theApp.MagicDLP->IsMagicDLPavailable();
	bool bDLPAvailable = theApp.dlp->IsDLPavailable();

	if (m_htiDetectModstrings) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectModstrings, bBothDLPAvailable);
	if (m_htiDetectUsernames) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectUsernames, bBothDLPAvailable);
	if (m_htiDetectUserhashes) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectUserhashes, bDLPAvailable);

	if (m_htiDetectBadHello) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectBadHello, bDLPAvailable);
	if (m_htiDetectSpam) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDetectSpam, bDLPAvailable);
	//<=sFrQlXeRt== Only enable Features that can be used
}


BOOL CPPgAngelArgos::OnInitDialog()
{
	m_bDontBanFriends = thePrefs.IsDontBanFriends();
	m_bInformLeechers = thePrefs.IsInformLeechers(); // => Inform Leechers - sFrQlXeRt
	m_strInformLeechersText = thePrefs.GetInformLeechersText(); // => Inform Leechers Text - evcz
	m_bDisPSForLeechers = thePrefs.DisPSForLeechers();
	// ==> Anti Upload Protection - sFrQlXeRt
	m_bAntiUploadProtection = thePrefs.IsAntiUploadProtection();
	m_iUploadProtectionLimit = thePrefs.GetUploadProtectionLimit();
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	m_iAntiUploaderBanLimit = thePrefs.GetAntiUploaderBanLimit();
	m_iAntiUploaderBanCase = thePrefs.GetAntiUploaderBanCase();
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	m_bDetectModstrings = thePrefs.IsDetectModstrings();
	m_bDetectUsernames = thePrefs.IsDetectUsernames();
	m_bDetectUserhashes = thePrefs.IsDetectUserhashes();
	m_iGplBreakerPunishment = thePrefs.GetGplBreakerPunishment();
	m_iHardLeecherPunishment = thePrefs.GetHardLeecherPunishment();
	m_iSoftLeecherPunishment = thePrefs.GetSoftLeecherPunishment();
	m_iBadModPunishment = thePrefs.GetBadModPunishment();

	m_bDetectWrongHello = thePrefs.IsDetectWrongHello();
	m_iWrongHelloPunishment= thePrefs.GetWrongHelloPunishment();

	m_bDetectBadHello = thePrefs.IsDetectBadHello();
	m_iBadHelloPunishment = thePrefs.GetBadHelloPunishment();

	m_bDetectCreditHack = thePrefs.IsDetectCreditHack();
	m_iCreditHackPunishment = thePrefs.GetCreditHackPunishment();

	m_bDetectModThief = thePrefs.IsDetectModThief();
	m_iModThiefPunishment = thePrefs.GetModThiefPunishment();

	m_bDetectNickThief = thePrefs.IsDetectNickThief();
	m_iNickThiefPunishment = thePrefs.GetNickThiefPunishment();

	m_bDetectFakeEmule = thePrefs.IsDetectFakeEmule();
	m_iFakeEmulePunishment = thePrefs.GetFakeEmulePunishment();

	m_bDetectGhostMod = thePrefs.IsDetectGhostMod();
	m_iGhostModPunishment = thePrefs.GetGhostModPunishment();

	m_bDetectSpam = thePrefs.IsDetectSpam();
	m_iSpamPunishment = thePrefs.GetSpamPunishment();

	m_bDetectEmcrypt = thePrefs.IsDetectEmcrypt();
	m_iEmcryptPunishment = thePrefs.GetEmcryptPunishment();

	m_bDetectXSExploiter = thePrefs.IsDetectXSExploiter();
	m_iXSExploiterPunishment = thePrefs.GetXSExploiterPunishment();

	m_bDetectFileFaker = thePrefs.IsDetectFileFaker();
	m_iFileFakerPunishment = thePrefs.GetFileFakerPunishment();

	m_bDetectAgressive = thePrefs.IsDetectAgressive();
	m_iAgressiveTime = (int)(thePrefs.GetAgressiveTime());
	m_iAgressiveCounter = (int)(thePrefs.GetAgressiveCounter());
	m_bAgressiveLog = thePrefs.IsAgressiveLog();
	m_iAgressivePunishment = thePrefs.GetAgressivePunishment();

	m_bPunishMlDonkey = thePrefs.IsPunishMlDonkey();
	m_bPunishEdonkey = thePrefs.IsPunishEdonkey();
	m_bPunishEdonkeyHybrid = thePrefs.IsPunishEdonkeyHybrid();
	m_bPunishShareaza = thePrefs.IsPunishShareaza();
	m_bPunishLphant = thePrefs.IsPunishLphant();
	m_bPunishSuiFailed = thePrefs.IsPunishSuiFailed();
	m_iDonkeyPunishment = (int)(thePrefs.GetDonkeyPunishment());

	CPropertyPage::OnInitDialog();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgAngelArgos::OnKillActive()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgAngelArgos::OnApply()
{
	// if prop page is closed by pressing VK_ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	thePrefs.m_bDontBanFriends = m_bDontBanFriends;
	thePrefs.m_bInformLeechers = m_bInformLeechers; // => Inform Leechers - sFrQlXeRt
	thePrefs.m_strInformLeechersText = m_strInformLeechersText; // => Inform Leechers Text - evcz
	thePrefs.m_bDisPSForLeechers = m_bDisPSForLeechers;
	// ==> Anti Upload Protection - sFrQlXeRt
	thePrefs.m_bAntiUploadProtection = m_bAntiUploadProtection;
	thePrefs.m_iUploadProtectionLimit = (uint16)m_iUploadProtectionLimit;
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	thePrefs.m_iAntiUploaderBanLimit = (uint16)m_iAntiUploaderBanLimit;
	thePrefs.AntiUploaderBanCaseMode = (uint8)m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	thePrefs.m_bDetectModstrings = m_bDetectModstrings;
	thePrefs.m_bDetectUsernames = m_bDetectUsernames;
	thePrefs.m_bDetectUserhashes = m_bDetectUserhashes;
	thePrefs.m_iGplBreakerPunishment = (uint8)m_iGplBreakerPunishment;
	thePrefs.m_iHardLeecherPunishment = (uint8)m_iHardLeecherPunishment;
	thePrefs.m_iSoftLeecherPunishment = (uint8)m_iSoftLeecherPunishment;
	thePrefs.m_iBadModPunishment = (uint8)m_iBadModPunishment;

	thePrefs.m_bDetectWrongHello = m_bDetectWrongHello;
	thePrefs.m_iWrongHelloPunishment = (uint8)m_iWrongHelloPunishment;

	thePrefs.m_bDetectBadHello = m_bDetectBadHello;
	thePrefs.m_iBadHelloPunishment = (uint8)m_iBadHelloPunishment;
	
	thePrefs.m_bDetectCreditHack = m_bDetectCreditHack;
	thePrefs.m_iCreditHackPunishment = (uint8)m_iCreditHackPunishment;
	
	thePrefs.m_bDetectModThief = m_bDetectModThief;
	thePrefs.m_iModThiefPunishment = (uint8)m_iModThiefPunishment;
	
	thePrefs.m_bDetectNickThief = m_bDetectNickThief;
	thePrefs.m_iNickThiefPunishment = (uint8)m_iNickThiefPunishment;
	
	thePrefs.m_bDetectFakeEmule = m_bDetectFakeEmule;
	thePrefs.m_iFakeEmulePunishment = (uint8)m_iFakeEmulePunishment;

	thePrefs.m_bDetectGhostMod = m_bDetectGhostMod;
	thePrefs.m_iGhostModPunishment = (uint8)m_iGhostModPunishment;
	
	thePrefs.m_bDetectSpam = m_bDetectSpam;
	thePrefs.m_iSpamPunishment = (uint8)m_iSpamPunishment;

	thePrefs.m_bDetectEmcrypt = m_bDetectEmcrypt;
	thePrefs.m_iEmcryptPunishment = (uint8)m_iEmcryptPunishment;

	thePrefs.m_bDetectXSExploiter = m_bDetectXSExploiter;
	thePrefs.m_iXSExploiterPunishment = m_iXSExploiterPunishment;

	thePrefs.m_bDetectFileFaker = m_bDetectFileFaker;
	thePrefs.m_iFileFakerPunishment = m_iFileFakerPunishment;

	thePrefs.m_bDetectAgressive = m_bDetectAgressive;
	thePrefs.m_iAgressiveTime = (uint16)m_iAgressiveTime;
	thePrefs.m_iAgressiveCounter = (uint16)m_iAgressiveCounter;
	thePrefs.m_bAgressiveLog = m_bAgressiveLog;
	thePrefs.m_iAgressivePunishment = (uint8)m_iAgressivePunishment;

	thePrefs.m_bPunishMlDonkey = m_bPunishMlDonkey;
	thePrefs.m_bPunishEdonkey = m_bPunishEdonkey;
	thePrefs.m_bPunishEdonkeyHybrid = m_bPunishEdonkeyHybrid;
	thePrefs.m_bPunishShareaza = m_bPunishShareaza;
	thePrefs.m_bPunishLphant = m_bPunishLphant;
	thePrefs.m_bPunishSuiFailed = m_bPunishSuiFailed;
	thePrefs.m_iDonkeyPunishment = (uint8)m_iDonkeyPunishment;

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgAngelArgos::Localize(void)
{
	if(m_hWnd)
	{
		CString buffer;
		//GetDlgItem(IDC_WARNINGMORPH)->SetWindowText(GetResString(IDS_ANGELARGOS_INFO));

		// ==> DLP [Xman] - sFrQlXeRt
		GetDlgItem(IDC_DLP_RELOAD)->SetWindowText(_T("Reload"));
		if(theApp.dlp->IsDLPavailable())
		{
			buffer.Format(_T("antiLeech.dll (DLP v%u)"), theApp.dlp->GetDLPVersion());
			GetDlgItem(IDC_DLP_STATIC)->SetWindowText(buffer);
		}
		else
			GetDlgItem(IDC_DLP_STATIC)->SetWindowText(_T("DLP not loaded"));
		// <== DLP [Xman] - sFrQlXeRt

		// ==> MagicDLP - sFrQlXeRt
		GetDlgItem(IDC_MDLP_RELOAD)->SetWindowText(_T("Reload"));
		if(theApp.MagicDLP->IsMagicDLPavailable())
		{
			buffer.Format(_T("MagicAntiLeech.dll (MagicDLP v%u)"), theApp.MagicDLP->GetMagicDLPVersion());
			GetDlgItem(IDC_MDLP_STATIC)->SetWindowText(buffer);
		}
		else
			GetDlgItem(IDC_MDLP_STATIC)->SetWindowText(_T("MagicDLP not loaded"));
		// <== MagicDLP - sFrQlXeRt

		if (m_htiDontBanFriends) m_ctrlTreeOptions.SetItemText(m_htiDontBanFriends, GetResString(IDS_DONT_BAN_FRIENDS));
		if (m_htiInformLeechers) m_ctrlTreeOptions.SetItemText(m_htiInformLeechers, GetResString(IDS_INFORM_LEECHERS)); // => Inform Leechers - sFrQlXeRt
		if (m_htiInformLeechersText) m_ctrlTreeOptions.SetEditLabel(m_htiInformLeechersText, GetResString(IDS_INFORM_LEECHERS_TEXT)); // => Inform Leechers Text - evcz
		if (m_htiDisPSForLeechers) m_ctrlTreeOptions.SetItemText(m_htiDisPSForLeechers, GetResString(IDS_DIS_PS_FOR_LEECHERS));
		if (m_htiAntiUploadProtection) m_ctrlTreeOptions.SetItemText(m_htiAntiUploadProtection, GetResString(IDS_ANTI_UP_PROTECTION)); // => Anti Upload Protection - sFrQlXeRt
		if (m_htiAntiUploaderBanLimit) m_ctrlTreeOptions.SetEditLabel(m_htiAntiUploaderBanLimit, GetResString(IDS_UNBAN_UPLOADER)); // => Anti Uploader Ban [Stulle] - sFrQlXeRt

		if (m_htiDetectModstrings) m_ctrlTreeOptions.SetItemText(m_htiDetectModstrings, GetResString(IDS_DETECT_MODSTRINGS));
		if (m_htiDetectUsernames) m_ctrlTreeOptions.SetItemText(m_htiDetectUsernames, GetResString(IDS_DETECT_USERNAMES));
		if (m_htiDetectUserhashes) m_ctrlTreeOptions.SetItemText(m_htiDetectUserhashes, GetResString(IDS_DETECT_USERHASHES));
		if (m_htiDetectWrongHello) m_ctrlTreeOptions.SetItemText(m_htiDetectWrongHello, GetResString(IDS_DETECT_WRONG_HELLO));
		if (m_htiDetectBadHello) m_ctrlTreeOptions.SetItemText(m_htiDetectBadHello, GetResString(IDS_DETECT_BADHELLO));
		if (m_htiDetectCreditHack) m_ctrlTreeOptions.SetItemText(m_htiDetectCreditHack, GetResString(IDS_DETECT_CREDIT_HACK));
		if (m_htiDetectModThief) m_ctrlTreeOptions.SetItemText(m_htiDetectModThief, GetResString(IDS_DETECT_MOD_THIEF));
		if (m_htiDetectNickThief) m_ctrlTreeOptions.SetItemText(m_htiDetectNickThief, GetResString(IDS_DETECT_NICK_THIEF));
		if (m_htiDetectFakeEmule) m_ctrlTreeOptions.SetItemText(m_htiDetectFakeEmule, GetResString(IDS_DETECT_FAKE_EMULE));
		if (m_htiDetectGhostMod) m_ctrlTreeOptions.SetItemText(m_htiDetectGhostMod, GetResString(IDS_DETECT_GHOST_MOD));
		if (m_htiDetectSpam) m_ctrlTreeOptions.SetItemText(m_htiDetectSpam, GetResString(IDS_DETECT_SPAM));
		if (m_htiDetectEmcrypt) m_ctrlTreeOptions.SetItemText(m_htiDetectEmcrypt, GetResString(IDS_DETECT_EMCRYPT));
		if (m_htiDetectXSExploiter) m_ctrlTreeOptions.SetItemText(m_htiDetectXSExploiter, GetResString(IDS_DETECT_XS_EXPLOITER));
		if (m_htiDetectFileFaker) m_ctrlTreeOptions.SetItemText(m_htiDetectFileFaker, GetResString(IDS_DETECT_FILEFAKER));

		if (m_htiAgressiveGroup) m_ctrlTreeOptions.SetItemText(m_htiAgressiveGroup, GetResString(IDS_DETECT_AGRESSIVE));
		if (m_htiDetectAgressive) m_ctrlTreeOptions.SetItemText(m_htiDetectAgressive, GetResString(IDS_DETECT_AGRESSIVE));
		if (m_htiAgressiveCounter) m_ctrlTreeOptions.SetEditLabel(m_htiAgressiveTime, GetResString(IDS_AGRESSIVE_TIME));
		if (m_htiAgressiveCounter) m_ctrlTreeOptions.SetEditLabel(m_htiAgressiveCounter, GetResString(IDS_AGRESSIVE_COUNTER));
		if (m_htiAgressiveLog) m_ctrlTreeOptions.SetItemText(m_htiAgressiveLog, GetResString(IDS_AGRESSIVE_LOG));

		if (m_htiPunishDonkeys) m_ctrlTreeOptions.SetItemText(m_htiPunishDonkeys, GetResString(IDS_PUNISH_DONKEYS));
		if (m_htiPunishMlDonkey) m_ctrlTreeOptions.SetItemText(m_htiPunishMlDonkey, GetResString(IDS_PUNISH_MLDONKEY));
		if (m_htiPunishEdonkey) m_ctrlTreeOptions.SetItemText(m_htiPunishEdonkey, GetResString(IDS_PUNISH_EDONKEY));
		if (m_htiPunishEdonkeyHybrid) m_ctrlTreeOptions.SetItemText(m_htiPunishEdonkeyHybrid, GetResString(IDS_PUNISH_EDONKEY_HYBRID));
		if (m_htiPunishShareaza) m_ctrlTreeOptions.SetItemText(m_htiPunishShareaza, GetResString(IDS_PUNISH_SHAREAZA));
		if (m_htiPunishLphant) m_ctrlTreeOptions.SetItemText(m_htiPunishLphant, GetResString(IDS_PUNISH_LPHANT));
		if (m_htiPunishSuiFailed) m_ctrlTreeOptions.SetItemText(m_htiPunishSuiFailed, GetResString(IDS_PUNISH_SUI_FAILED));
	}

}

void CPPgAngelArgos::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	m_htiGeneralOptions = NULL;
	m_htiDontBanFriends = NULL;
	m_htiInformLeechers = NULL; // => Inform Leechers - sFrQlXeRt
	m_htiInformLeechersText = NULL; // => Inform Leechers Text - evcz
	m_htiDisPSForLeechers = NULL;
	// ==> Anti Upload Protection - sFrQlXeRt
	m_htiAntiUploadProtection = NULL;
	m_htiUploadProtectionLimit = NULL;
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	m_htiAntiUploaderBanLimit = NULL;
	m_htiAntiCase1 = NULL;
	m_htiAntiCase2 = NULL;
	m_htiAntiCase3 = NULL;
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	m_htiLeecherModDetection = NULL;
	m_htiDetectModstrings = NULL;
	m_htiDetectUsernames = NULL;
	m_htiDetectUserhashes = NULL;
	m_htiGplBreakerPunishment = NULL;
	m_htiHardLeecherPunishment = NULL;
	m_htiSoftLeecherPunishment = NULL;
	m_htiBadModPunishment = NULL;
	m_htiIpBan = NULL;
	m_htiUploadBan = NULL;
	m_htiScore01 = NULL;
	m_htiScore02 = NULL;
	m_htiScore03 = NULL;
	m_htiScore04 = NULL;
	m_htiScore05 = NULL;
	m_htiScore06 = NULL;
	m_htiScore07 = NULL;
	m_htiScore08 = NULL;
	m_htiScore09 = NULL;
	m_htiNoBan = NULL;
	m_htiDetectWrongHello = NULL;
	m_htiWrongHelloPunishment = NULL;
	m_htiDetectBadHello = NULL;
	m_htiBadHelloPunishment = NULL;
	m_htiDetectCreditHack = NULL;
	m_htiCreditHackPunishment = NULL;
	m_htiDetectModThief = NULL;
	m_htiModThiefPunishment = NULL;
	m_htiDetectNickThief = NULL;
	m_htiNickThiefPunishment = NULL;
	m_htiDetectFakeEmule = NULL;
	m_htiFakeEmulePunishment = NULL;
	m_htiDetectGhostMod = NULL;
	m_htiGhostModPunishment = NULL;
	m_htiDetectSpam = NULL;
	m_htiSpamPunishment = NULL;
	m_htiDetectEmcrypt = NULL;
	m_htiEmcryptPunishment = NULL;
	m_htiDetectXSExploiter = NULL;
	m_htiXSExploiterPunishment = NULL;
	m_htiDetectFileFaker = NULL;
	m_htiFileFakerPunishment = NULL;

	m_htiAgressiveGroup = NULL;
	m_htiDetectAgressive = NULL;
	m_htiAgressiveTime = NULL;
	m_htiAgressiveCounter = NULL;
	m_htiAgressiveLog = NULL;
	m_htiAgressivePunishment = NULL;

	m_htiPunishDonkeys = NULL;
	m_htiPunishMlDonkey = NULL;
	m_htiPunishEdonkey = NULL;
	m_htiPunishEdonkeyHybrid = NULL;
	m_htiPunishShareaza = NULL;
	m_htiPunishLphant = NULL;
	m_htiPunishSuiFailed = NULL;
	m_htiDonkeyPunishment = NULL;

	CPropertyPage::OnDestroy();
}

LRESULT CPPgAngelArgos::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == IDC_ANGELARGOS_OPTS){


		SetModified();
	}
	return 0;
}

void CPPgAngelArgos::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);
	CString temp;
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgAngelArgos::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgAngelArgos::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgAngelArgos::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

// ==> DLP [Xman] - sFrQlXeRt
void CPPgAngelArgos::OnBnClickedDlpReload()
{
	theApp.dlp->Reload();
	LoadSettings();
}
// <== DLP [Xman] - sFrQlXeRt

// ==> MagicDLP - sFrQlXeRt
void CPPgAngelArgos::OnBnClickedMDlpReload()
{
	theApp.MagicDLP->Reload();
	LoadSettings();
}
// <== MagicDLP - sFrQlXeRt

