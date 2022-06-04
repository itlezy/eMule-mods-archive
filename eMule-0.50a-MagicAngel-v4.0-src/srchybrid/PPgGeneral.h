#pragma once

// MORPH START  leuk_he tooltipped
#include "PPGtooltipped.h" 

/*
class CPPgGeneral : public CPropertyPage 
*/
class CPPgGeneral : public CPPgtooltipped  
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgGeneral)

public:
	CPPgGeneral();
	virtual ~CPPgGeneral();

// Dialog Data
	enum { IDD = IDD_PPG_GENERAL };

	void Localize(void);

protected:
	UINT	m_iActualKeyModifier; //Commander - Added: Invisible Mode [TPT]

	CComboBox m_language;
	void LoadSettings(void);
	void UpdateEd2kLinkFixCtrl();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBoxesChange() {SetBoxes();SetModified();}//Commander - Added: Invisible Mode [TPT]
	afx_msg void OnCbnSelchangeKeymodcombo(); //Commander - Added: Invisible Mode [TPT]
	afx_msg void OnBnClickedEd2kfix();
	afx_msg void OnBnClickedEditWebservices();
	afx_msg void OnLangChange();
	afx_msg void OnBnClickedCheck4Update();
	afx_msg void OnCbnCloseupLangs();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

private:
	void SetBoxes(); //Commander - Added: Invisible Mode [TPT] - Start
};
