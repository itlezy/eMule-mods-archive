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
	bool m_bModified; // X: [CI] - [Code Improvement] Apply if modified
	bool bCreated;

	void LoadSettings(void);

	//virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeWSEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	afx_msg void SetTmplButtonState();
	afx_msg void OnSettingsChange()			{ m_bModified = true;	SetModified();} // X: [CI] - [Code Improvement] Apply if modified
	afx_msg void OnDataChange()				{OnSettingsChange(); SetTmplButtonState(); }
};
