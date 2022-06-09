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
	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	// Tux: LiteMule: Remove Help

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedEditadr();
	// Tux: LiteMule: Remove Help
};
