#pragma once
#include "afxcmn.h" //stats

class CPPgDisplay : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDisplay)

public:
	CPPgDisplay();
	virtual ~CPPgDisplay();

// Dialog Data
	enum { IDD = IDD_PPG_DISPLAY };

	void Localize(void);

protected:
	void LoadSettings(void);

	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified
    bool m_bFillGraphs;
 
//stats +
    int m_iStatsColors;
	DWORD* m_pdwStatsColors;
	CSliderCtrl m_ctlGraphsUpdate;
	CSliderCtrl m_ctlGraphsAvgTime;
	CSliderCtrl m_ctlStatsUpdate;
	int m_iGraphsUpdate;
	int m_iGraphsAvgTime;
	int m_iStatsUpdate;
    
	void ShowInterval();
//stats -
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified

//stats +
	afx_msg void OnDestroy();
};
