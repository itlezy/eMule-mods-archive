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
	void LoadSettings();

	// Generated message map functions
	afx_msg void OnSettingsChange() {SetModified();}
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()

	// for dialog data exchange and validation
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	uint32 processprio; //Xman process prio
	
public:
	afx_msg void OnOpenMoreSlots();
	afx_msg void OnBnClickedNafcfullcontrol();
	afx_msg void OnBnClickedrioRadio(); //Xman process prio
};
