#pragma once

class CCustomAutoComplete;

class CPPgSecurity : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSecurity)

public:
	CPPgSecurity();
	virtual ~CPPgSecurity();

// Dialog Data
	enum { IDD = IDD_PPG_SECURITY };

	void Localize(void);
	void DeleteDDB();

protected:
	CCustomAutoComplete* m_pacIPFilterURL;
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	void LoadSettings(void);

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnReloadIPFilter();
	afx_msg void OnEditIPFilter();
	afx_msg void OnLoadIPFFromURL();
	afx_msg void OnEnChangeUpdateUrl();
	afx_msg void OnDDClicked();
	afx_msg void OnBnClickedRunAsUser();
	afx_msg void OnDestroy();
	afx_msg void OnObfuscatedDisabledChange();
	afx_msg void OnObfuscatedRequestedChange();
};
