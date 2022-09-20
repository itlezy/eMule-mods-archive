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
	// Tux: LiteMule: Remove Help
	afx_msg void OnBnClickedSeltempdiradd();
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

