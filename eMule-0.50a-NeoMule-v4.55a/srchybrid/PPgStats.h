#pragma once
#include "ColorButton.h"

class CPPgStats : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgStats)

public:
	CPPgStats();
	virtual ~CPPgStats();

	// Dialog Data
	enum { IDD = IDD_PPG_STATS };

	void Localize(void);

protected:
	int m_iStatsColors;
	DWORD* m_pdwStatsColors;
	CComboBox m_colors;
	CComboBox m_cratio;
	// NEO: SI - [SysInfo] -- Xanatos -->
	CComboBox m_mratio;
	CComboBox m_tratio;
	CComboBox m_sratio;
	// NEO: SI END <-- Xanatos --
	CColorButton m_ctlColor;
	CSliderCtrl m_ctlGraphsUpdate;
	CSliderCtrl m_ctlGraphsAvgTime;
	CSliderCtrl m_ctlStatsUpdate;
	int m_iGraphsUpdate;
	int m_iGraphsAvgTime;
	int m_iStatsUpdate;
	BOOL m_bModified;
	bool m_bFillGraphs;

	void ShowInterval();
	void SetModified(BOOL bChanged = TRUE);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCbnSelChangeColorSelector();
	afx_msg LONG OnColorPopupSelChange(UINT lParam, LONG wParam);
	afx_msg void OnEnChangeCGraphScale() { SetModified(); }
	afx_msg void OnCbnSelChangeCRatio()	{ SetModified(); }
	// NEO: SI - [SysInfo] -- Xanatos -->
	afx_msg void OnCbnSelchangeMRatio()	{ SetModified(); }
	afx_msg void OnCbnSelchangeTRatio()	{ SetModified(); }
	afx_msg void OnCbnSelchangeSRatio()	{ SetModified(); }
	// NEO: SI END <-- Xanatos --
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedFillGraphs();
};
