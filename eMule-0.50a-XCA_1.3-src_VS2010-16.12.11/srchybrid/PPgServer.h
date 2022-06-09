#pragma once

class CPPgServer : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgServer)

public:
	CPPgServer();
	virtual ~CPPgServer();

	// Dialog Data
	enum { IDD = IDD_PPG_SERVER };

	void Localize(void);

protected:
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified
	void LoadSettings(void);

	//virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnBnClickedEditadr();
};
