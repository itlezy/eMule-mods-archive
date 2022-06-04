#pragma once
#include "preferences.h"
#include "TreeOptionsCtrlEx.h"

// CPPgAngelArgos dialog
// by sFrQlXeRt
//


class CPPgAngelArgos : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAngelArgos)

public:
	CPPgAngelArgos();
	virtual ~CPPgAngelArgos();

// Dialog Data
	enum { IDD = IDD_PPG_ANGELARGOS };
protected:

	bool	m_bDontBanFriends;
	bool	m_bInformLeechers; // => Inform Leechers - sFrQlXeRt
	CString	m_strInformLeechersText; // => Inform Leechers Text - evcz
	bool	m_bDisPSForLeechers;
	// ==> Anti Upload Protection - sFrQlXeRt
	bool	m_bAntiUploadProtection;
	int		m_iUploadProtectionLimit;
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	int m_iAntiUploaderBanLimit;
	int m_iAntiUploaderBanCase;
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	bool	m_bDetectModstrings;
	bool	m_bDetectUsernames;
	bool	m_bDetectUserhashes;
	int		m_iGplBreakerPunishment;
	int		m_iHardLeecherPunishment;
	int		m_iSoftLeecherPunishment;
	int		m_iBadModPunishment;
	bool	m_bDetectWrongHello;
	int		m_iWrongHelloPunishment;
	bool	m_bDetectBadHello;
	int		m_iBadHelloPunishment;
	bool	m_bDetectCreditHack;
	int		m_iCreditHackPunishment;
	bool	m_bDetectModThief;
	int		m_iModThiefPunishment;
	bool	m_bDetectNickThief;
	int		m_iNickThiefPunishment;
	bool	m_bDetectFakeEmule;
	int		m_iFakeEmulePunishment;
	bool	m_bDetectGhostMod;
	int		m_iGhostModPunishment;
	bool	m_bDetectSpam;
	int		m_iSpamPunishment;
	bool	m_bDetectEmcrypt;
	int		m_iEmcryptPunishment;
	bool	m_bDetectXSExploiter;
	int		m_iXSExploiterPunishment;
	bool	m_bDetectFileFaker;
	int		m_iFileFakerPunishment;
	
	bool	m_bDetectAgressive;
	int		m_iAgressiveTime;
	int		m_iAgressiveCounter;
	bool	m_bAgressiveLog;
	int		m_iAgressivePunishment;

	bool	m_bPunishMlDonkey;
	bool	m_bPunishEdonkey;
	bool	m_bPunishEdonkeyHybrid;
	bool	m_bPunishShareaza;
	bool	m_bPunishLphant;
	bool	m_bPunishSuiFailed;
	int		m_iDonkeyPunishment;

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	HTREEITEM	m_htiGeneralOptions;
	HTREEITEM	m_htiDontBanFriends;
	HTREEITEM	m_htiInformLeechers; // => Inform Leechers - sFrQlXeRt
	HTREEITEM	m_htiInformLeechersText; // => Inform Leechers Text - evcz
	// ==> Anti Upload Protection - sFrQlXeRt
	HTREEITEM	m_htiAntiUploadProtection;
	HTREEITEM	m_htiUploadProtectionLimit;
	HTREEITEM	m_htiDisPSForLeechers;
	// <== Anti Upload Protection - sFrQlXeRt
	// ==> Anti Uploader Ban [Stulle] - sFrQlXeRt
	HTREEITEM m_htiAntiUploaderBanLimit;
	HTREEITEM m_htiAntiCase1;
	HTREEITEM m_htiAntiCase2;
	HTREEITEM m_htiAntiCase3;
	// <== Anti Uploader Ban [Stulle] - sFrQlXeRt

	HTREEITEM	m_htiLeecherModDetection;
	HTREEITEM	m_htiDetectModstrings;
	HTREEITEM	m_htiDetectUsernames;
	HTREEITEM	m_htiDetectUserhashes;
	HTREEITEM	m_htiGplBreakerPunishment;
	HTREEITEM	m_htiHardLeecherPunishment;
	HTREEITEM	m_htiSoftLeecherPunishment;
	HTREEITEM	m_htiBadModPunishment;
	HTREEITEM	m_htiIpBan;
	HTREEITEM	m_htiUploadBan;
	HTREEITEM	m_htiScore01;
	HTREEITEM	m_htiScore02;
	HTREEITEM	m_htiScore03;
	HTREEITEM	m_htiScore04;
	HTREEITEM	m_htiScore05;
	HTREEITEM	m_htiScore06;
	HTREEITEM	m_htiScore07;
	HTREEITEM	m_htiScore08;
	HTREEITEM	m_htiScore09;
	HTREEITEM	m_htiNoBan;
	HTREEITEM	m_htiDetectWrongHello;
	HTREEITEM	m_htiWrongHelloPunishment;
	HTREEITEM	m_htiDetectBadHello;
	HTREEITEM	m_htiBadHelloPunishment;
	HTREEITEM	m_htiDetectCreditHack;
	HTREEITEM	m_htiCreditHackPunishment;
	HTREEITEM	m_htiDetectModThief;
	HTREEITEM	m_htiModThiefPunishment;
	HTREEITEM	m_htiDetectNickThief;
	HTREEITEM	m_htiNickThiefPunishment;
	HTREEITEM	m_htiDetectFakeEmule;
	HTREEITEM	m_htiFakeEmulePunishment;
	HTREEITEM	m_htiDetectGhostMod;
	HTREEITEM	m_htiGhostModPunishment;
	HTREEITEM	m_htiDetectSpam;
	HTREEITEM	m_htiSpamPunishment;
	HTREEITEM	m_htiDetectEmcrypt;
	HTREEITEM	m_htiEmcryptPunishment;
	HTREEITEM	m_htiDetectXSExploiter;
	HTREEITEM	m_htiXSExploiterPunishment;
	HTREEITEM	m_htiDetectFileFaker;
	HTREEITEM	m_htiFileFakerPunishment;

	HTREEITEM	m_htiAgressiveGroup;
	HTREEITEM	m_htiDetectAgressive;
	HTREEITEM	m_htiAgressiveTime;
	HTREEITEM	m_htiAgressiveCounter;
	HTREEITEM	m_htiAgressiveLog;
	HTREEITEM	m_htiAgressivePunishment;

	HTREEITEM	m_htiPunishDonkeys;
	HTREEITEM	m_htiPunishMlDonkey;
	HTREEITEM	m_htiPunishEdonkey;
	HTREEITEM	m_htiPunishEdonkeyHybrid;
	HTREEITEM	m_htiPunishShareaza;
	HTREEITEM	m_htiPunishLphant;
	HTREEITEM	m_htiPunishSuiFailed;
	HTREEITEM	m_htiDonkeyPunishment;


	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void Localize(void);	
	void LoadSettings(void);
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();
	afx_msg void OnSettingsChange()			{ SetModified(); }
	afx_msg void OnBnClickedDlpReload(); // => DLP [Xman] - sFrQlXeRt
	afx_msg void OnBnClickedMDlpReload(); // => MagicDLP - sFrQlXeRt
};