#pragma once
#include "Preferences.h"

class CPPgProxy : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgProxy)

public:
	CPPgProxy();
	virtual ~CPPgProxy();

	// Dialog Data
	enum { IDD = IDD_PPG_PROXY };

	void Localize(void);

protected:
	ProxySettings proxy;
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	void LoadSettings();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEnableProxy();
	afx_msg void OnBnClickedEnableAuthentication();
	afx_msg void OnCbnSelChangeProxyType();
	afx_msg void OnSettingsChange() { m_bModified = true;	SetModified(TRUE); } // X: [CI] - [Code Improvement] Apply if modified
};
