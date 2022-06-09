#pragma once

class CPPgFiles : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgFiles)

public:
	CPPgFiles();
	virtual ~CPPgFiles();

// Dialog Data
	enum { IDD = IDD_PPG_FILES };

	void Localize(void);

protected:
	CListBox m_uncfolders;
	HICON m_icoBrowse;

	void LoadSettings(void);
	void OnSettingsChangeCat(uint8 index);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	// Tux: LiteMule: Remove Help

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSetCleanupFilter();
	afx_msg void BrowseVideoplayer();
	afx_msg void OnSettingsChange();
	afx_msg void OnSettingsChangeCat1() {OnSettingsChangeCat(1);}
	afx_msg void OnSettingsChangeCat2()	{OnSettingsChangeCat(2);}
	// Tux: LiteMule: Remove Help
	afx_msg void OnDestroy();
};
