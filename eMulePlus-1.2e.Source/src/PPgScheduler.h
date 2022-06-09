#pragma once

class CPreferences;

class CPPgScheduler : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgScheduler)

public:
	CPPgScheduler();
	virtual ~CPPgScheduler();
	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

// Dialog Data
	enum { IDD = IDD_PPG_SCHEDULER };

	afx_msg void OnSettingsChange()			{SetModified();}
	afx_msg void OnBnClickedSCHEnabled();

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);

protected:
	BOOL	m_bModified;
	BOOL	m_bSchedulerEnabled;
	BOOL	m_bExceptMon;
	BOOL	m_bExceptTue;
	BOOL	m_bExceptWed;
	BOOL	m_bExceptThu;
	BOOL	m_bExceptFri;
	BOOL	m_bExceptSat;
	BOOL	m_bExceptSun;
	CString	m_strShift1Down;
	CString	m_strShift1Up;
	CString	m_strShift1Conn;
	CString	m_strShift1Conn5Sec;
	CString	m_strShift2Up;
	CString	m_strShift2Down;
	CString	m_strShift2Conn;
	CString	m_strShift2Conn5Sec;

private:
	CPreferences *m_pPrefs;
	CComboBox m_Shift1TimeCombo;
	CComboBox m_Shift2TimeCombo;
};
