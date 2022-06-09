#pragma once

#include "DirectoryTreeCtrl.h"

class CPreferences;

class CPPgDirectories : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDirectories)

public:
	CPPgDirectories();
	virtual ~CPPgDirectories();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);

	enum { IDD = IDD_PPG_DIRECTORIES };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnBnClickedSelincdir();
	afx_msg void OnBnClickedSeltempdir();
	afx_msg void OnBnClickedSelvlc();
	afx_msg void OnBnClickedBackupPreview();
	afx_msg void OnEnChange()	{ SetModified(); }

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);
	BOOL SelectDir(const TCHAR* indir, TCHAR* outdir, const CString& titletext);
	void CheckSharedChanges(void);
	void CheckTempChanges(void);

protected:
	BOOL	m_bModified;
	CString	m_strIncomingDir;
	CString	m_strMainTempDir;
	CString	m_strPlayer;
	CString	m_strPlayerArgs;
	BOOL	m_bVideoBackup;
	BOOL	m_bSmallBlocks;

private:
	CPreferences *m_pPrefs;
	CDirectoryTreeCtrl m_ShareSelector;
	CDirectoryTreeCtrl m_TempSelector;
	bool	m_bSharedDirsModified;
};
