#pragma once
#include "HypertextCtrl.h"
// MORPH START leuk_he tooltipped
#include "PPGtooltipped.h" 
class CPPgWebServer : public CPPgtooltipped  
/*
class CPPgWebServer : public CPropertyPage
*/
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgWebServer)

public:
	CPPgWebServer();
	virtual ~CPPgWebServer();

	enum { IDD = IDD_PPG_WEBSRV };

	void Localize(void);

	bool bCreated; //>>> [ionix] - iONiX::Advanced WebInterface Account Management - needs to be public
	 // MORPH START  tabbed options [leuk_he]
private:
	enum eTab	{NONE, WEBSERVER,MULTIWEBSERVER,NTSERVICE};
	CTabCtrl	m_tabCtr;
	eTab		m_currentTab;
	CImageList	m_imageList;
	void		SetTab(eTab tab);
public:
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	void	InitTab(bool firstinit, int Page = 0);
 // MORPH END tabbed options [leuk_he]


protected:
	BOOL m_bModified;
	/* Moved to public 
	bool bCreated;
	*/
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
public: //MORPH [ionix] - iONiX::Advanced WebInterface Account Management
	afx_msg void OnEnChangeWSEnabled();
protected: //MORPH [ionix] - iONiX::Advanced WebInterface Account Management
	afx_msg void OnEnChangeMMEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	afx_msg void OnHelp();
	afx_msg void SetTmplButtonState();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnDataChange()				{SetModified(); SetTmplButtonState(); }
	afx_msg void OnDestroy();
};
