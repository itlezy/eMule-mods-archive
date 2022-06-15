#pragma once

#include "Neo\GUI\DblScope.h" // NEO: PSM - [PlusSpeedMeter] <-- Xanatos --

#define IDC_TOOLBAR			16127
#define IDC_TOOLBARBUTTON	16129

#define	TBBTN_CONNECT	(IDC_TOOLBARBUTTON + 0)
#define	TBBTN_KAD		(IDC_TOOLBARBUTTON + 1)
#define	TBBTN_SERVER	(IDC_TOOLBARBUTTON + 2)
#define	TBBTN_TRANSFERS	(IDC_TOOLBARBUTTON + 3)
#define	TBBTN_SEARCH	(IDC_TOOLBARBUTTON + 4)
#define	TBBTN_SHARED	(IDC_TOOLBARBUTTON + 5)
#define	TBBTN_MESSAGES	(IDC_TOOLBARBUTTON + 6)
#define	TBBTN_IRC		(IDC_TOOLBARBUTTON + 7)
#define	TBBTN_STATS		(IDC_TOOLBARBUTTON + 8)
#define	TBBTN_OPTIONS	(IDC_TOOLBARBUTTON + 9)
#define	TBBTN_TOOLS		(IDC_TOOLBARBUTTON + 10)
#define	TBBTN_HELP		(IDC_TOOLBARBUTTON + 11)
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#define	TBBTN_VOODOO	(IDC_TOOLBARBUTTON + 12)
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

#define	MULE_TOOLBAR_BAND_NR	0

enum EToolbarLabelType {
	NoLabels	= 0,
	LabelsBelow = 1,
	LabelsRight = 2
};

class CMuleToolbarCtrl : public CToolBarCtrl
{
	DECLARE_DYNAMIC(CMuleToolbarCtrl)

public:
	CMuleToolbarCtrl();
	virtual ~CMuleToolbarCtrl();

	void Init();
	void Localize();
	void Refresh();
	void SaveCurHeight();
	void UpdateBackground();
	void PressMuleButton(int nID);
	BOOL GetMaxSize(LPSIZE pSize) const;

	static int GetDefaultLabelType() { return (int)LabelsBelow; }

	// NEO: PSM - [PlusSpeedMeter] -- Xanatos -->
	void SetSpeedMeterRange(int iValue1, int iValue2){
														int iMaxRange = iValue2;
														if (iValue1 > iMaxRange)
															iMaxRange = iValue1;
														UINT nLastMaxRange, nLastMinRange;
														m_ctrlSpeedMeter.GetRange(nLastMaxRange, nLastMinRange);
														if ((int)nLastMaxRange != iMaxRange)
															m_ctrlSpeedMeter.SetRange(iMaxRange, nLastMinRange);
													}
	void SetSpeedMeterValues(int iValue1, int iValue2)	{ m_ctrlSpeedMeter.AddValues(iValue1/1024,iValue2/1024); }
	void ShowSpeedMeter(bool bShow);
	bool IsSpeedMeterEnabled() const					{ return m_bUseSpeedMeter == TRUE; }
	// NEO: PSM END <-- Xanatos --

protected:
	CSize		m_sizBtnBmp;
	int			m_iPreviousHeight;
	int			m_iLastPressedButton;
	int			m_buttoncount;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	TBBUTTON	TBButtons[13];
	TCHAR		TBStrings[13][200];
#else
	TBBUTTON	TBButtons[12];
	TCHAR		TBStrings[12][200];
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	CStringArray m_astrToolbarPaths;
	EToolbarLabelType m_eLabelType;
	CStringArray m_astrSkinPaths;
	CBitmap		m_bmpBack;

	// NEO: PSM - [PlusSpeedMeter] -- Xanatos -->
	CDblScope		m_ctrlSpeedMeter;
	bool			m_bUseSpeedMeter;
	// NEO: PSM END <-- Xanatos --

	void ChangeToolbarBitmap(const CString& rstrPath, bool bRefresh);
	void ChangeTextLabelStyle(EToolbarLabelType eLabelType, bool bRefresh, bool bForceUpdateButtons = false);
	void UpdateIdealSize();
	void SetAllButtonsStrings();
	void SetAllButtonsWidth();
	void ForceRecalcLayout();

#ifdef _DEBUG
	void Dump();
#endif

	void AutoSize();
	virtual	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnQueryDelete(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnQueryInsert(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnGetButtonInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnToolbarChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnReset(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnInitCustomize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnEndAdjust(NMHDR* pNMHDR, LRESULT* pResult);
};
