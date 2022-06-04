#pragma once
#include "Preferences.h"
#include "TreeOptionsCtrlEx.h"
// CPPgEastShare dialog

// MORPH START leuk_he tooltipped
/*
class CPPgEastShare : public CPropertyPage
*/
class CPPgEastShare : public CPPgtooltipped
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgEastShare)

public:
	CPPgEastShare();
	virtual ~CPPgEastShare();

// Dialog Data
	enum { IDD = IDD_PPG_EASTSHARE };
protected:
	
	bool m_bEnablePreferShareAll;//EastShare - PreferShareAll by AndCycle
	HTREEITEM m_htiEnablePreferShareAll;//EastShare - PreferShareAll by AndCycle

	bool m_bIsPayBackFirst;//EastShare - added by AndCycle, Pay Back First
	HTREEITEM m_htiIsPayBackFirst; //EastShare - added by AndCycle, Pay Back First
	
	int m_iPayBackFirstLimit; //MORPH - Added by SiRoB, Pay Back First Tweak
	HTREEITEM m_htiPayBackFirstLimit; //MORPH - Added by SiRoB, Pay Back First Tweak

	bool m_bOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)
	HTREEITEM m_htiOnlyDownloadCompleteFiles;//EastShare - Added by AndCycle, Only download complete files v2.1 (shadow)

	bool m_bSaveUploadQueueWaitTime;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)
	HTREEITEM m_htiSaveUploadQueueWaitTime;//Morph - added by AndCycle, Save Upload Queue Wait Time (MSUQWT)

	//EastShare - Added by Pretender, Option for ChunkDots
	bool m_bEnableChunkDots;
	HTREEITEM m_htiEnableChunkDots;
	//EastShare - Added by Pretender, Option for ChunkDots

	//EastShare - added by AndCycle, IP to Country
	int	m_iIP2CountryName;
	HTREEITEM m_htiIP2CountryName;
	HTREEITEM m_htiIP2CountryName_DISABLE;
	HTREEITEM m_htiIP2CountryName_SHORT;
	HTREEITEM m_htiIP2CountryName_MID;
	HTREEITEM m_htiIP2CountryName_LONG;
	bool m_bIP2CountryShowFlag;
	HTREEITEM m_htiIP2CountryShowFlag;
	//EastShare - added by AndCycle, IP to Country

	// EastShare START - Added by linekin, new creditsystem by [lovelace]
	int m_iCreditSystem;
	HTREEITEM m_htiCreditSystem;
	HTREEITEM m_htiOfficialCredit;
	HTREEITEM m_htiLovelaceCredit;
	HTREEITEM m_htiRatioCredit;
	HTREEITEM m_htiPawcioCredit;
	HTREEITEM m_htiESCredit;
	// EastShare START - Added by linekin, new creditsystem by [lovelace]

	//Morph - added by AndCycle, Equal Chance For Each File
	bool	m_bEnableEqualChanceForEachFile;
	HTREEITEM m_htiEnableEqualChanceForEachFile;
	//Morph - added by AndCycle, Equal Chance For Each File

	// EastShare Start - FollowTheMajority by AndCycle
	bool	m_bFollowTheMajority;
	HTREEITEM m_htiFollowTheMajority;
	// EastShare End   - FollowTheMajority by AndCycle

	// EastShare	Start - FairPlay by AndCycle
	bool	m_bFairPlay;
	HTREEITEM m_htiFairPlay;
	// EastShare	End   - FairPlay by AndCycle

	// EastShare START - Added by TAHO, .met control
	HTREEITEM m_htiMetControl;
	int m_iPurgeMode;
	HTREEITEM m_htiDontPurge;
	HTREEITEM m_htiPartiallyPurge;
	HTREEITEM m_htiCompletelyPurge;
	int m_iKnownMetDays;
	HTREEITEM m_htiKnownMet;
	bool m_bRemoveAichImmediatly;
	HTREEITEM m_htiRemoveAichImmediatly;
	// EastShare END - Added by TAHO, .met control

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

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
