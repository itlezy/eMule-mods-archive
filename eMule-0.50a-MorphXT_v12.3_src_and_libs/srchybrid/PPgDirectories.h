#pragma once
#include "DirectoryTreeCtrl.h"


// MORPH START leuk_he tooltipped
/*
class CPPgDirectories : public CPropertyPage
*/
class CPPgDirectories : public CPPgtooltipped  
// MORPH END leuk_he tooltipped
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
	HICON m_icoBrowse;

	void LoadSettings(void);
	void FillUncList(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedSelincdir();
	afx_msg void OnBnClickedSeltempdir();
	afx_msg void OnBnClickedAddUNC();
	afx_msg void OnBnClickedRemUNC();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedSeltempdiradd();
	afx_msg void OnDestroy();
};
// MOROPH START sharesubdir
class CAddSharedDirDialog : public CDialog
{
	DECLARE_DYNAMIC(CAddSharedDirDialog )

public:
	CAddSharedDirDialog (LPTSTR  sUnc,bool bSubdir,CWnd* pParent = NULL);   //  constructor
//	virtual ~CAddSharedDirDialog ();
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	enum { IDD = IDD_ADDSHAREDIR };
private:
	CString m_sUnc;
	bool m_bSubdir ;

public:
	CString GetUNC () { return m_sUnc; } ;
	bool    GetSubDir () { return m_bSubdir ;}  ;

protected:
	DECLARE_MESSAGE_MAP()

};
// MOROPH END sharesubdir

