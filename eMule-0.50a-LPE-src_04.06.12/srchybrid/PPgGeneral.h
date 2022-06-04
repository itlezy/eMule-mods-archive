#pragma once

class CPPgGeneral : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgGeneral)

public:
	CPPgGeneral();
	virtual ~CPPgGeneral();

// Dialog Data
	enum { IDD = IDD_PPG_GENERAL };

	void Localize(void);

protected:
	CComboBox m_language;
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified
	void LoadSettings(void);
	void UpdateEd2kLinkFixCtrl();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnBnClickedEd2kfix();
	afx_msg void OnLangChange();
	afx_msg void OnCbnCloseupLangs();
};
