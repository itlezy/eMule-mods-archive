#pragma once

//#include "preferences.h"
//#include "afxwin.h"
// CPPgXtreme dialog

class CPPgXtreme : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgXtreme)

public:
	CPPgXtreme();
	virtual ~CPPgXtreme();

	// Dialog Data
	enum { IDD = IDD_PPG_Xtreme };

	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();

	void Localize();


protected:
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified
	void LoadSettings();

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange() { m_bModified = true;	SetModified();} // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnOpenMoreSlots();

	// for dialog data exchange and validation
	//virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
};
