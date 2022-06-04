#pragma once
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped

void SysTimeToStr(LPSYSTEMTIME st, LPTSTR str); //MORPH - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]

// CPPgMorph dialog
// MORPH START leuk_he tooltipped
/*
class CPPgMorph2 : public CPropertyPage
*/
class CPPgMorph2 : public CPPgtooltipped  
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgMorph2)

public:
	CPPgMorph2();
	virtual ~CPPgMorph2();

// Dialog Data
	enum { IDD = IDD_PPG_MORPH2 };
protected:
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	void Localize(void);
	void LoadSettings(void);
	afx_msg void OnSettingsChange() {SetModified();}
	afx_msg void OnBnClickedUpdatefakes();//MORPH START - Added by milobac and Yun.SF3, FakeCheck, FakeReport, Auto-updating
	afx_msg void OnBnClickedResetfakes();
	afx_msg void OnBnClickedUpdateipfurl();//MORPH START added by Yun.SF3: Ipfilter.dat update
	afx_msg void OnBnClickedResetipfurl();
	afx_msg void OnBnClickedUpdateipcurl();// Commander - Added: IP2Country auto-updating
	afx_msg void OnBnClickedResetipcurl();
};
