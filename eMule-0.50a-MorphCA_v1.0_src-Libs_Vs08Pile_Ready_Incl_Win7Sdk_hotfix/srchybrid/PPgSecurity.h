#pragma once

void SysTimeToStr(LPSYSTEMTIME st, LPTSTR str); //MORPH - Added by Stulle, New IP Filter by Ozzy [Stulle/Ozzy]

class CCustomAutoComplete;

class CPPgSecurity : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSecurity)

public:
	CPPgSecurity();
	virtual ~CPPgSecurity();

// Dialog Data
	enum { IDD = IDD_PPG_SECURITY };

	void Localize(void);
	void DeleteDDB();

protected:
	CCustomAutoComplete* m_pacIPFilterURL;

	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnReloadIPFilter();
	afx_msg void OnEditIPFilter();
	afx_msg void OnLoadIPFFromURL();
	afx_msg void OnEnChangeUpdateUrl();
	afx_msg void OnDDClicked();
	afx_msg void OnBnClickedRunAsUser();
	afx_msg void OnDestroy();
	afx_msg void OnObfuscatedDisabledChange();
	afx_msg void OnObfuscatedRequestedChange();

    afx_msg void OnBnClickedUpdateipfurl();//MORPH START added by Yun.SF3: Ipfilter.dat update
	afx_msg void OnBnClickedResetipfurl();
};
