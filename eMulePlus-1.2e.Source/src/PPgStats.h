#pragma once

#include "ColorButton.h"

class CPreferences;

class CPPgStats : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgStats)

public:
	CPPgStats();
	virtual ~CPPgStats();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	enum { IDD = IDD_PPG_STATS };

	afx_msg void OnSettingsChange()		{ SetModified(); }
	afx_msg void OnCbnSelchangeColorselector();
	afx_msg LONG OnSelChange(UINT lParam, LONG wParam);

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void ShowInterval();

	DECLARE_MESSAGE_MAP()

protected:
	BOOL	m_bModified;
	CPreferences *m_pPrefs;
	CString	m_strStat;
	int		m_iGraphsUpdateInterval;
	int		m_iAverageGraphTime;
	int		m_iStatisticsUpdInterval;

private:
	DWORD			m_dwStatColors[12];
	CColorButton	m_ColorButton;
	CComboBox		m_ColorsCombo;
	CComboBox		m_RatioCombo;
	CSliderCtrl		m_UpdateIntervalSlider;
	CSliderCtrl		m_AverageGraphTimeSlider;
	CSliderCtrl		m_StatisticsUpdateIntervalSlider;
};
