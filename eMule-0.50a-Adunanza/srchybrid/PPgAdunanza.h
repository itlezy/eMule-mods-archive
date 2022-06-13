#pragma once

class CPPgAdunanzA : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAdunanzA)

public:
	CPPgAdunanzA();
	virtual ~CPPgAdunanzA();
	enum { IDD = IDD_PPG_ADUNANZA };
protected:
	afx_msg void OnEnChangeAduEdit();
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	DECLARE_MESSAGE_MAP()
	CSliderCtrl m_ctlStreaming; 
public:
	void LoadSettings(void);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void UpdateBwPosition();
	afx_msg void OnBnClickedAduAnonymStats();
	afx_msg void OnBnClickedAduNoObf();
	afx_msg void OnBnClickedAduNoTips();
	afx_msg void OnBnClickedBandaExtCheck();
	afx_msg void OnEnChangeValorebandaupesx();
	afx_msg void StreamingChange(NMHDR *pNMHDR, LRESULT *pResult);
};

// finestra di dialogo CAskExit by Anis

class CAskExit : public CDialog
{
	DECLARE_DYNAMIC(CAskExit)

public:
	CAskExit(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CAskExit();
// Dati della finestra di dialogo
	enum { IDD = IDD_ASKEXIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedNo();
	afx_msg void OnBnClickedNominimize();
};
