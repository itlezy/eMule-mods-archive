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
	//CListBox m_uncfolders;	//Enig123:: Avi3k: fix code
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	void LoadSettings(void);
	void OnSettingsChangeCat(uint8 index);

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnSetCleanupFilter();
	afx_msg void BrowseVideoplayer();
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); }// X: [CI] - [Code Improvement] // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnSettingsChangeSNF();// X: [CI] - [Code Improvement]
	afx_msg void OnSettingsChangeExt();// X: [DCE] - [DontCompressExt]
	afx_msg void OnSettingsChangeCat1() {OnSettingsChangeCat(1);}
	afx_msg void OnSettingsChangeCat2()	{OnSettingsChangeCat(2);}
	afx_msg void OnBnClickedRememberdownloaded(); //Xman remove unused AICH-hashes
};
