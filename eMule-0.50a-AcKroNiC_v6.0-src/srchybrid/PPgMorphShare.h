#pragma once
#include "TreeOptionsCtrlEx.h"
// CPPgMorphShare dialog

// MORPH START leuk_he tooltipped
/*
class CPPgMorphShare : public CPropertyPage  
*/
class CPPgMorphShare : public CPPgtooltipped // leuk_he tooltipped
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgMorphShare)

public:
	CPPgMorphShare();
	virtual ~CPPgMorphShare();

// Dialog Data
	enum { IDD = IDD_PPG_MORPH_SHARE };
protected:

	int m_iPowershareMode; //MORPH - Added by SiRoB, Avoid misusing of powersharing
	int	m_iSpreadbar; //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	int m_iHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	bool m_bSelectiveShare;  //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	int m_iShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	int m_iPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
	int m_iPermissions; //MORPH - Added by SiRoB, Show Permissions
	// Mighty Knife: Community visualization
	CString   m_sCommunityName;
	// [end] Mighty Knife
	bool m_bPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
	bool m_bFolderIcons;
	bool m_bStaticIcon; //MORPH - Added, Static Tray Icon
	// MORPH START show less controls
	bool m_bShowLessControls;
public:
	bool m_bOverideBySetExtControls;
protected:
	// MORPH END  show less controls
	bool m_bFakeAlyzerIndications; //MORPH - Added by Stulle, Fakealyzer [netfinity]

	
	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;
	HTREEITEM m_htiSFM;
	//MORPH START - Added by SiRoB, Avoid misusing of powersharing
	HTREEITEM m_htiPowershareMode;
	HTREEITEM m_htiPowershareDisabled;
	HTREEITEM m_htiPowershareActivated;
	HTREEITEM m_htiPowershareAuto;
	HTREEITEM m_htiPowershareLimited;
	//MORPH END   - Added by SiRoB, Avoid misusing of powersharing

	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	HTREEITEM m_htiSpreadbar;
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file

	HTREEITEM m_htiHideOS;	//MORPH - Added by SiRoB, SLUGFILLER: hideOS
	HTREEITEM m_htiSelectiveShare; //MORPH - Added by SiRoB, SLUGFILLER: hideOS
	HTREEITEM m_htiShareOnlyTheNeed; //MORPH - Added by SiRoB, SHARE_ONLY_THE_NEED
	HTREEITEM m_htiPowerShareLimit; //MORPH - Added by SiRoB, POWERSHARE Limit
	HTREEITEM m_htiPowershareInternalPrio; //Morph - added by AndCyle, selective PS internal Prio
	//MORPH START - Added by SiRoB, Show Permission
	HTREEITEM m_htiPermissions;
	HTREEITEM m_htiPermAll;
	HTREEITEM m_htiPermFriend;
	HTREEITEM m_htiPermNone;
	// Mighty Knife: Community visible filelist
	HTREEITEM m_htiPermCommunity;
	HTREEITEM m_htiCommunityName;
	// [end] Mighty Knife
	//MORPH END   - Added by SiRoB, Show Permission
	HTREEITEM m_htiDisplay;
	HTREEITEM m_htiFolderIcons;
	HTREEITEM m_htiStaticIcon; //MORPH - Added, Static Tray Icon
	HTREEITEM m_htiShowLessControls ;// Show less controls
	HTREEITEM m_htiFakeAlyzerIndications; //MORPH - Added by Stulle, Fakealyzer [netfinity]
	
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
