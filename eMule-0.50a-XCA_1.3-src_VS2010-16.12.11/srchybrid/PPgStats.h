#pragma once
#include "ColorButton.h"
#include "afxcmn.h"

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
	size_t m_iStatsColors;
	DWORD* m_pdwStatsColors;
	CComboBox m_colors;
	CComboBox m_cratio;
	CColorButton m_ctlColor;
	CSliderCtrl m_ctlGraphsUpdate;
	CSliderCtrl m_ctlGraphsAvgTime;
	CSliderCtrl m_ctlStatsUpdate;
	int m_iGraphsUpdate;
	int m_iGraphsAvgTime;
	int m_iStatsUpdate;
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified
	bool m_bFillGraphs;

	void ShowInterval();
	void OnSettingsChange(){ m_bModified = true;	SetModified();} // X: [CI] - [Code Improvement] Apply if modified

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCbnSelChangeColorSelector();
	afx_msg LRESULT OnColorPopupSelChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
};
