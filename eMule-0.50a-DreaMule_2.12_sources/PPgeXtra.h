#pragma once
#include "Preferences.h"

class CPPgeXtra : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgeXtra)

public:
	CPPgeXtra();
	virtual ~CPPgeXtra();
    // Dialog Data
	enum { IDD = IDD_PPG_eXtra };

	void Localize(void);

protected:
	int key;
	int mod;
	int fdcspos;                 //[FDC]  slider position for FDC sensitivity
	bool b_localdoubleFDC;       //[FDC] double error for alert
	bool uploadslotfocus;        //[FS]   slot focus
	bool enablestate;            //hidden hot keys
	bool b_localHideFiltersState;//[PSF] Preset Search Filetrs
	CString localPresetFilters;  //[PSF] Preset Search Filetrs
	CString urlinput;            //[IPUP] IP update
 	virtual void DoDataExchange(CDataExchange* pDX);   // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

  	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedEnHotkey();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnCbnSelchangeCombokey();
	afx_msg void OnNMCustomdrawSliderfdc(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedSlotfocus();
	afx_msg void OnEnChangeEditurl();
	afx_msg void OnBnClickedHidefilters();
	afx_msg void OnEnChangeSearchpresetedit();
	afx_msg void OnBnClickedFdctea();
};