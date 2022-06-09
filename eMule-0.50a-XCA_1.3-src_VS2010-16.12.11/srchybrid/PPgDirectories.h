#pragma once
#include "DirectoryTreeCtrl.h"

class CPPgDirectories : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDirectories)

public:
	CPPgDirectories();									// standard constructor
	virtual ~CPPgDirectories();

// Dialog Data
	enum { IDD = IDD_PPG_DIRECTORIES };

	void Localize(void);

protected:
	CDirectoryTreeCtrl m_ShareSelector;
	CListCtrl m_ctlUncPaths;
	CListCtrl m_ctlTempPaths; // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified

	void LoadSettings(void);
	void FillUncList(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ m_bModified = true;	SetModified(); } // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnBnClickedSelincdir();
//	afx_msg void OnBnClickedSeltempdir();// NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	afx_msg void OnBnClickedAddUNC();
	afx_msg void OnBnClickedRemUNC();
	afx_msg void OnBnClickedAddTemp();// NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	afx_msg void OnBnClickedRemTemp();// NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	afx_msg void OnSettingsChangeExt();// X: [DSE] - [DontShareExt]
//	afx_msg void OnBnClickedSeltempdiradd();// NEO: MTD - [MultiTempDirectories] <-- Xanatos --
};
