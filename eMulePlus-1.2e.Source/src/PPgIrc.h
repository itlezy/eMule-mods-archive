#pragma once

class CPreferences;

class CPPgIRC : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgIRC)

public:
	CPPgIRC();
	virtual ~CPPgIRC();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);

	enum { IDD = IDD_PPG_IRC };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnSettingsChange()				{ SetModified(); }
	afx_msg void OnEnChangeNick()				{ SetModified(); m_bNickModified = true; }
	afx_msg void OnBnClickedUseFilter();
	afx_msg void OnBnClickedUsePerform();

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
	bool m_bNickModified;

protected:
	CPreferences *m_pPrefs;
	BOOL	m_bModified;
	CString	m_strServer;
	CString	m_strNick;
	CString	m_strName;
	CString	m_strMinUser;
	CString	m_strPerform;
	BOOL	m_bUseChannelFilter;
	BOOL	m_bUsePerform;
	BOOL	m_bTimeStamp;
	BOOL	m_bListOnConnect;
	BOOL	m_bIgnoreInfoMessages;
	BOOL	m_bStripColor;
};
