#pragma once

class CPreferences;

class CPPgSecurity : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgSecurity)

public:
	CPPgSecurity();
	virtual ~CPPgSecurity();
	void SetPrefs(CPreferences* pPrefs)	{ m_pPrefs = pPrefs; }
	void Localize(void);

	enum { IDD = IDD_PPG_SECURITY };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	afx_msg void OnSettingsChange()		{ SetModified(); }
	afx_msg void OnBnClickedIpfilterUpdateButton();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedIpfilterUpdateonstartCheck();
	afx_msg void OnBnClickedAvbrowse();
	afx_msg void OnBnClickedAvEnable();
	afx_msg void OnObfuscatedDisabledChange();
	afx_msg void OnObfuscatedRequestedChange();

protected:
	CPreferences*	m_pPrefs;
	BOOL			m_bModified;

	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);
	void SetDaysCaption(int iSliderPos);

protected:
	CString	m_strIPFilterURLEdit;
	CString	m_strAVParams;
	CString	m_strAVPath;
	BOOL	m_bFilterClientsCheck;
	BOOL	m_bFilterServersCheck;
	BOOL	m_bIPFilterUpdateOnStartCheck;
	BOOL	m_bFileScanFilterCheck;
	BOOL	m_bFakeDataFilterCheck;
	BOOL	m_bAVEnabledCheck;
	BOOL	m_bAVScanCompletedCheck;

private:
	CSpinButtonCtrl m_FilterLevelSpinCtrl;
	CSliderCtrl m_UpdateDaysSlider;
	CButton	m_AVBrowseButton;
	CEdit	m_strAVParamsEdit;
	CEdit	m_strAVPathEdit;
};
