#pragma once

class CPPgBackup : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgBackup)
public:
	CPPgBackup();
	virtual ~CPPgBackup();

	// Dialog Data
	enum { IDD = IDD_PPG_BACKUP };
protected:
	BOOL m_bModified;
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void Backup(LPCTSTR extensionToBack, BOOL conFirm);
	void Backup3(); //eastshare added by linekin, backup backup
	void Localize(void);

private:
	void Backup2(LPCTSTR extensionToBack);
	void LoadSettings(void);
	BOOL y2All;
	BOOL n2All;
public:

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBackupnow();
	afx_msg void OnBnClickedDat();
	afx_msg void OnBnClickedMet();
	afx_msg void OnBnClickedIni();
	afx_msg void OnBnClickedPart();
	afx_msg void OnBnClickedPartMet();
	afx_msg void OnBnClickedSelectall();
	afx_msg void OnBnClickedAutobackup();
	//EastShare Start- Added by Pretender, Double Backup
	afx_msg void OnBnClickedAutobackup2();
	//EastShare End- Added by Pretender, Double Backup
};
