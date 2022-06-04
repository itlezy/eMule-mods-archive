#pragma once
//#include "HypertextCtrl.h"

class CPPgIonixWebServer : public CPPgtooltipped  
{
	DECLARE_DYNAMIC(CPPgIonixWebServer)

public:
	CPPgIonixWebServer();
	virtual ~CPPgIonixWebServer();

	enum { IDD = IDD_PPG_IONIXWEBSRV };

	void Localize(void);

	BOOL m_bIsInit;
	// MORPH START  tabbed options [leuk_he]
private:
	enum eTab	{NONE, WEBSERVER,MULTIWEBSERVER,NTSERVICE};
	CTabCtrl	m_tabCtr;
	eTab		m_currentTab;
	CImageList	m_imageList;
	void		SetTab(eTab tab);
public:
	afx_msg void OnTcnSelchangeTab(NMHDR * /* pNMHDR*/, LRESULT *pResult);
	void	InitTab(bool firstinit, int Page = 0);
 // MORPH END tabbed options [leuk_he]
protected:
	BOOL m_bModified;

	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnSetActive(); 
	void SetModified(BOOL bChanged = TRUE){
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	DECLARE_MESSAGE_MAP()	
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	afx_msg void OnSettingsChange();
public:
	afx_msg void OnEnableChange(); //lh 
protected:
	afx_msg void OnMultiPWChange();
	afx_msg void OnMultiCatsChange();
	//afx_msg void OnSettingsChangeBox()			{ SetMultiBoxes(); OnSettingsChange(); }
	afx_msg void OnBnClickedNew();
	afx_msg void OnBnClickedDel();
	afx_msg void UpdateSelection();
	CComboBox	m_cbAccountSelector;
	CComboBox	m_cbUserlevel;
	void	SetBoxes();
	void	FillComboBox();
	void	FillUserlevelBox();
//<<< [ionix] - iONiX::Advanced WebInterface Account Management
};
