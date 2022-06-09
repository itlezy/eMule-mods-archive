// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#ifndef OT_FONTSEL
#define OT_FONTSEL

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FontSel.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeFontSelFontCombo.h"
#include "OptionTreeFontSelSizeCombo.h"
#include "OptionTreeFontSelGlobals.h"
#include "OptionTreeFontSelColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// COptionTreeFontSel dialog

class COptionTreeFontSel : public CDialog
{
// Construction
public:
	void SetControlID(UINT uID);
	void GetCharFormat(CHARFORMAT &cf);
	void GetLogFont(LOGFONT &lf);
	void SetCustomSample(CString strSample);
	DWORD GetDialogFlags();
	void SetCurFont(LOGFONT lf);
	void SetDefaultFont(LOGFONT lf);
	void SetCurFont(CHARFORMAT cfFont);
	void SetDefaultFont(CHARFORMAT cfFont);
	COLORREF GetTextColor();
	COLORREF GetDefaultTextColor();
	void SetTextColor(COLORREF crColor);
	void SetDefaultTextColor(COLORREF crColor);
	COptionTreeFontSel(DWORD dwFlags, LOGFONT lfFont, LOGFONT lfDefault, CWnd* pParent = NULL);
	COptionTreeFontSel(DWORD dwFlags, LOGFONT lfFont, CWnd* pParent = NULL);
	COptionTreeFontSel(DWORD dwFlags, CHARFORMAT cfFont, CHARFORMAT cfDefault, CWnd* pParent = NULL);
	COptionTreeFontSel(DWORD dwFlags, CHARFORMAT cfFont, CWnd* pParent = NULL);
	virtual ~COptionTreeFontSel();

// Dialog Data
	//{{AFX_DATA(COptionTreeFontSel)
	enum { IDD = IDD_FONTSEL };
	CStatic	m_ctlSample;
	CComboBox	m_ctlFontStyles;
	COptionTreeFontSelSizeCombo	m_ctlFontSizes;
	COptionTreeFontSelFontCombo	m_ctlFontNames;
	CButton	m_ctlUnderline;
	CButton	m_ctlStrikeout;
	CButton	m_ctlButtonDefault;
	CButton	m_ctlButtonApply;
	COptionTreeFontSelColorButton	m_ctlButtonColor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionTreeFontSel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	void GetSelectedFont(LOGFONT &lf);
	void ConvertFont(CHARFORMAT &cfFont, LOGFONT &lf);
	void ConvertFont(LOGFONT &lf, CHARFORMAT &cfFont);
	void OnSelChangeFontScript();
	BOOL ValidateControls();
	void SelectCurFont(LOGFONT lf);
	void FillSizeStyleList();
	void FillFaceNameList();
	static int CALLBACK EnumFontFamiliesCallBack(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int nFontType, LPARAM lParam);
	static int CALLBACK EnumFontFamiliesCallBack2(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int nFontType, LPARAM lParam);
	void OnObjectsChanged();
	void UpdateSampleFont();
	void OnEditUpdateFontNames();
	void OnEditUpdateFontSizes();
	void OnSelChangeFontNames();
	void OnSelChangeFontSizes();
	void OnSelChangeFontStyles();
	void OnEditChangeFontStyles();
	void OnStrikeout();
	void OnUnderline();


	CFont m_fSampleFont;
	DWORD m_nCurrentStyle;
	DWORD m_nActualStyle;
	DWORD m_nStyles;
	BOOL m_bStrikeOut;
	BOOL m_bUnderline;
	CString m_strFontSize;
	LOGFONT m_lfFont;
	LOGFONT m_lfDefault;
	DWORD m_dwDialogFlags;
	CString m_strCustomSample;
	UINT m_uControlID;

	// Generated message map functions
	//{{AFX_MSG(COptionTreeFontSel)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnButtonOK();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonDefault();
	afx_msg void OnButtonApply();
	//}}AFX_MSG
	afx_msg long OnSelChangeColor(UINT lParam, long wParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_FONTSEL
