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
	// Tux: LiteMule: Remove MobileMule

	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	// Tux: LiteMule: Remove Help
	void SetModified(BOOL bChanged = TRUE){
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeWSEnabled();
	afx_msg void OnEnChangeMMEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	// Tux: LiteMule: Remove Help
	afx_msg void SetTmplButtonState();
	// Tux: LiteMule: Remove Help
	afx_msg void OnDataChange()				{SetModified(); SetTmplButtonState(); }
};
