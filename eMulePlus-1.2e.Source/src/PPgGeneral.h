#pragma once

#include "3DPreviewControl.h"

class CPreferences;

class CPPgGeneral : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgGeneral)

public:
	CPPgGeneral();
	virtual ~CPPgGeneral();

	void SetPrefs(CPreferences* in_prefs) { m_pPrefs = in_prefs; }
	void Localize(void);

// Dialog Data
	enum { IDD = IDD_PPG_GENERAL };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnSettingsChange()						{SetModified();}
	afx_msg void On3DDepth(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEditWebservices();
	afx_msg void OnBnClickedEd2kfix();
	afx_msg void OnBnClickedFakeUpdate();

protected:
	void SetModified(BOOL bChanged = TRUE)
	{
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()

private:
	void LoadSettings(void);
	void LoadLanguagesCombo(void);
	void DrawPreview();

protected:
	CPreferences *m_pPrefs;
	BOOL	m_bModified;
	CString	m_strUserNick;
	CString	m_strTooltipDelay;
	CString	m_strUpdateFakeListURL;
	int		m_iMainProcess;
	BOOL	m_bBeepOnErrors;
	BOOL	m_bShowSplashscreen;
	BOOL	m_bAllowMultipleInstances;
	BOOL	m_bOnlineSignature;
	BOOL	m_bDoubleClickClientDetails;
	BOOL	m_bAutoTakeEd2kLinks;
	BOOL	m_bUpdateFakeList;
	BOOL	m_bWatchClipboard;
	BOOL	m_bLocalizedLinks;

private:
	CSliderCtrl m_3DSlider;
	CComboBox m_LanguageCombo;
	C3DPreviewControl m_3DPreview;
};
