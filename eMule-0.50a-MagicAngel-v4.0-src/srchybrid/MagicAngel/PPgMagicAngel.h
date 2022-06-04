#pragma once
#include "preferences.h"
#include "TreeOptionsCtrlEx.h"
// CPPgMagicAngel dialog
// by sFrQlXeRt

class CPPgMagicAngel : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgMagicAngel)

public:
	CPPgMagicAngel();
	virtual ~CPPgMagicAngel();

// Dialog Data
	enum { IDD = IDD_PPG_MAGICANGEL };
protected:

	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	bool EmuMLDonkey;
	bool EmueDonkey;
	bool EmueDonkeyHybrid;
	bool EmuShareaza;
	bool EmuLphant;
	bool EmuCommunityNickAddons;
	bool m_bLogEmulator;
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	bool	m_bQuickStart;
	int		m_iQuickStartTime;
	int		m_iQuickStartTimePerFile;
	int		m_iQuickMaxConperFive;
	int		m_iQuickMaxHalfOpen;
	int		m_iQuickMaxConnections;
	bool	m_bQuickStartOnIPChange;
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	bool m_bIsReleaseBoost;
	int m_iGetReleaseBoost;
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	bool	m_bPushSmallFiles;
	int		m_iPushSmallFilesSize;
	bool	m_bPushRareFiles;
	int		m_iPushRareFilesValue;
	bool	m_bPushRatioFiles;
	int		m_iPushRatioFilesValue;
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	bool m_bReaskSourcesAfterIpChange; // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
	bool m_bSpreadReask; // => manual enable spread reask - sFrQlXeRt
	int  m_iReaskTime; // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	// <== Reask Tweaks - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	int m_iFnTag;
	CString m_sFnCustomTag;
	bool m_bFnTagAtEnd;
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	int m_iColoredUpload;
	bool m_bColorQueue;
	bool m_bColorKnownClients;
	bool m_bDontShowMorphColors;
	// <== Colored Upload - sFrQlXeRt

	bool m_bUseIntelligentFlush; //==> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	bool m_bReconnectKadOnIpChange; //==sFrQlXeRt=> Reconncect Kad on IP change

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	// ==> Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt
	HTREEITEM m_htiEmulator;
	HTREEITEM m_htiEnableMLDonkey;
	HTREEITEM m_htiEnableeDonkey;
	HTREEITEM m_htiEnableeDonkeyHybrid;
	HTREEITEM m_htiEnableShareaza;
	HTREEITEM m_htiEnableLphant;
	HTREEITEM m_htiEnableCommunityNickAddons;
	HTREEITEM m_htiLogEmulator;
	// <== Emulate others by WiZaRd/Spike/shadow2004 - sFrQlXeRt

	// ==> Quickstart [DavidXanatos] - sFrQlXeRt
	HTREEITEM m_htiQuickStart;
	HTREEITEM m_htiQuickStartEnable;
	HTREEITEM m_htiQuickStartTime;
	HTREEITEM m_htiQuickStartTimePerFile;
	HTREEITEM m_htiQuickMaxConperFive;
	HTREEITEM m_htiQuickMaxHalfOpen;
	HTREEITEM m_htiQuickMaxConnections;
	HTREEITEM m_htiQuickStartOnIPChange;
	// <== Quickstart [DavidXanatos] - sFrQlXeRt

	// ==> Reask Tweaks - sFrQlXeRt
	HTREEITEM m_htiReaskTweaks;
	HTREEITEM m_htiReaskSourcesAfterIpChange; // => Reask Sources after IP Change v4 [Xman] - sFrQlXeRt
	HTREEITEM m_htiSpreadReask; // => manual enable spread reask - sFrQlXeRt
	HTREEITEM m_htiReaskTime; // => Timer for ReAsk File Sources [Stulle] - sFrQlXeRt
	// <== Reask Tweaks - sFrQlXeRt

	// ==> Release Boost - sFrQlXeRt
	HTREEITEM m_htiReleaseBoostGroup;
	HTREEITEM m_htiEnableReleaseBoost;
	HTREEITEM m_htiReleaseBoost; 
	// <== Release Boost - sFrQlXeRt

	// ==> Push Files [sivka/NeoMule] - sFrQlXeRt
	HTREEITEM m_htiFilePushTweaks;
	HTREEITEM m_htiPushSmallFiles;
	HTREEITEM m_htiPushSmallFilesSize;
	HTREEITEM m_htiPushRareFiles;
	HTREEITEM m_htiPushRareFilesValue;
	HTREEITEM m_htiPushRatioFiles;
	HTREEITEM m_htiPushRatioFilesValue;
	// <== Push Files [sivka/NeoMule] - sFrQlXeRt

	// ==> FunnyNick Tag [Stulle] - sFrQlXeRt
	HTREEITEM m_htiFnTag;
	HTREEITEM m_htiNoTag;
	HTREEITEM m_htiShortTag;
	HTREEITEM m_htiFullTag;
	HTREEITEM m_htiCustomTag;
	HTREEITEM m_htiFnCustomTag;
	HTREEITEM m_htiFnTagAtEnd;
	// <== FunnyNick Tag [Stulle] - sFrQlXeRt

	// ==> Colored Upload - sFrQlXeRt
	HTREEITEM m_htiColoredUpload;
	HTREEITEM m_htiNoColoredUpload;
	HTREEITEM m_htiColorQueueranks;
	HTREEITEM m_htiColorFSandPS;
	HTREEITEM m_htiColorQueue;
	HTREEITEM m_htiColorKnownClients;
	HTREEITEM m_htiDontShowMorphColors;
	// <== Colored Upload - sFrQlXeRt

	HTREEITEM m_htiOtherSettings;
	HTREEITEM m_htiUseIntelligentFlush; //==> Intelligent Filebuffer Flushing [WiZaRd] - sFrQlXeRt
	HTREEITEM m_htiReconnectKadOnIpChange; //==sFrQlXeRt=> Reconncect Kad on IP change


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
};