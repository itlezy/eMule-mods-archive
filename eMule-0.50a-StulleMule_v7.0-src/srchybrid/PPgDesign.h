#pragma once
#ifdef DESIGN_SETTINGS
#include "ColorButton.h"
#include "BtnST.h"

class CPPgDesign : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDesign)
public:
	CPPgDesign();
	virtual ~CPPgDesign();

	// Dialog Data
	enum { IDD = IDD_PPG_DESIGN };
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
	void Localize(void);
	void InitMasterStyleCombo();
	void InitSubStyleCombo();
	void UpdateStyles();
	void OnFontStyle(int iStyle);
	StylesStruct GetStyle(int nMaster, int nStyle);
	void SetStyle(int nMaster, int nStyle, StylesStruct *style=NULL);

private:
	void LoadSettings(void);

	StylesStruct nClientStyles[style_c_count];
	StylesStruct nDownloadStyles[style_d_count];
	StylesStruct nShareStyles[style_s_count];
	StylesStruct nServerStyles[style_se_count];
	StylesStruct nBackgroundStyles[style_b_count];
	StylesStruct nWindowStyles[style_w_count];
	bool m_bFocusWasOnCombo;
	bool m_bDesignChanged;

	CComboBox	m_MasterCombo;
	CComboBox	m_SubCombo;
	CButton		m_OnOff;
	CButtonST	m_bold;
	CButtonST	m_underlined;
	CButtonST	m_italic;
	CColorButton	m_FontColor;
	CColorButton	m_BackColor;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg LONG OnColorPopupSelChange(UINT lParam, LONG wParam);
	afx_msg void OnBnClickedBold();
	afx_msg void OnBnClickedUnderlined();
	afx_msg void OnBnClickedItalic();
	afx_msg void OnCbnSelchangeStyleselMaster();
	afx_msg void OnCbnSelchangeStyleselSub();
	afx_msg void OnBnClickedOnOff();
	afx_msg void OnCbClickedAutoText();
	afx_msg void OnEnKillfocusMasterCombo();
	afx_msg void OnEnKillfocusSubCombo();
	afx_msg void OnHelp();
};
#endif