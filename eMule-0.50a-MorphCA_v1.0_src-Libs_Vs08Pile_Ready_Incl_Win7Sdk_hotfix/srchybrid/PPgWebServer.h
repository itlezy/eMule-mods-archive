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
	void SetModified(BOOL bChanged = TRUE){
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeWSEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	afx_msg void SetTmplButtonState();
	afx_msg void OnDataChange()				{SetModified(); SetTmplButtonState(); }
	afx_msg void OnDestroy();
};
