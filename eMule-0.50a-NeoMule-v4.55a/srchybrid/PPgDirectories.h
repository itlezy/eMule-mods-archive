#pragma once
#include "Neo/GUI/CP/DirectoryTreeCtrl.h" // NEO: SSD - [ShareSubDirectories] <-- Xanatos --

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
	//CListCtrl m_ctlUncPaths; // NEO: SSD - [ShareSubDirectories] <-- Xanatos --
	CListCtrl m_ctlTempPaths; // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	

	void LoadSettings(void);
	//void FillUncList(void); // NEO: SSD - [ShareSubDirectories] <-- Xanatos --

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedSelincdir();
	//afx_msg void OnBnClickedSeltempdir(); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	// NEO: SSD - [ShareSubDirectories] -- Xanatos --
	//afx_msg void OnBnClickedAddUNC();
	//afx_msg void OnBnClickedRemUNC();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//afx_msg void OnBnClickedSeltempdiradd(); 
	// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
	afx_msg void OnBnClickedAddTemp();
	afx_msg void OnBnClickedRemTemp();
	// NEO: MTD END <-- Xanatos --
};
