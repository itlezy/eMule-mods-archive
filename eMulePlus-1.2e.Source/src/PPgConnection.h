#pragma once

class CPreferences;

class CPPgConnection : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgConnection)

public:
	CPPgConnection();
	virtual ~CPPgConnection();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	// Dialog Data
	enum { IDD = IDD_PPG_CONNECTION };

	afx_msg void OnCbnSelchangeConType();
	afx_msg void OnSettingsChange()				{ SetModified(); }
	afx_msg void OnEnChangeUDPDisable();
	afx_msg void OnQueueChange();
	afx_msg void OnSpeedChange();
	afx_msg void OnBnClickedLimitless();
	void Localize(void);

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
	void EnableEditBoxes(BOOL bEnable = TRUE);

protected:
	CPreferences *m_pPrefs;
	CComboBox m_conprof;
	BOOL	m_bModified;
	BOOL	m_bShowOverhead;
	BOOL	m_bLancastEnable;
	BOOL	m_bOpenPorts;
	BOOL	m_bLimitlessDownload;
	BOOL	m_bUdpDisable;
	CString	m_strQueueSize;
	CString	m_strMaxConn;
	CString	m_strMaxCon5;
	CString	m_strUploadCap;
	CString	m_strUploadMax;
	CString	m_strDownloadCap;
	CString	m_strDownloadMax;
	CString	m_strPort;
	CString	m_strUdpPort;
};
