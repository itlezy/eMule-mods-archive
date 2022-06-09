#pragma once

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
	enum ESelectFont
	{
		sfServer,
		sfLog
	} m_eSelectFont;
	void LoadSettings(void);

	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	static UINT_PTR CALLBACK ChooseFontHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnBnClickedSelectHypertextFont();
	afx_msg void OnBtnClickedResetHist();
};
