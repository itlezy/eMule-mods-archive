#pragma once
#include "HypertextCtrl.h"

class CPPgWebServer : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgWebServer)

public:
	CPPgWebServer();
	virtual ~CPPgWebServer();

	enum { IDD = IDD_PPG_WEBSRV };

	void Localize(void);

protected:
	BOOL m_bModified;
	bool bCreated;
	CHyperTextCtrl m_wndMobileLink;
	HICON m_icoBrowse;

	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void SetModified(BOOL bChanged = TRUE){
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	DECLARE_MESSAGE_MAP()
public: // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	afx_msg void OnEnChangeWSEnabled();
protected: // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	afx_msg void OnEnChangeMMEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	afx_msg void OnHelp();
	afx_msg void SetTmplButtonState();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnDataChange()				{SetModified(); SetTmplButtonState(); }
	afx_msg void OnDestroy();
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
public:
	enum eTab	{WEBSERVER, MULTIWEBSERVER, NTSERVICE};
private:
	CTabCtrl	m_tabCtr;
	eTab		m_currentTab;
	CImageList	m_imageList;
	void		SetTab(eTab tab);
public:
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	void	InitTab(bool firstinit, int Page = 0);
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle

	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle
	afx_msg void OnEnableChange(); //lh 
protected:
	virtual BOOL OnSetActive();
	afx_msg void OnMultiPWChange();
	afx_msg void OnMultiCatsChange();
	afx_msg void OnSettingsChange();
	//afx_msg void OnSettingsChangeBox()			{ SetMultiBoxes(); OnSettingsChange(); }
	afx_msg void OnBnClickedNew();
	afx_msg void OnBnClickedDel();
	afx_msg void UpdateSelection();
	CComboBox	m_cbAccountSelector;
	CComboBox	m_cbUserlevel;
	void	SetMultiBoxes();
	void	FillComboBox();
	void	FillUserlevelBox();
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he/Stulle] - Stulle

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
	afx_msg void OnBnClickedInstall();	
	afx_msg void OnBnClickedUnInstall();	
	afx_msg void OnBnStartSystem();	
	afx_msg void OnBnManualStart();	
	afx_msg void OnBnAllSettings();	
	afx_msg void OnBnRunBRowser();	
	afx_msg void OnBnReplaceStart();
	afx_msg void OnCbnSelChangeOptLvl()		{SetModified();}
	CComboBox	m_cbOptLvl;

	int  FillStatus();
	void InitOptLvlCbn(bool bFirstInit = false);
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
};
