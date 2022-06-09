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

// Added Headers
#include "CommonRes.h"


#ifndef OT_FONTSEL_COLORBUTTON
#define OT_FONTSEL_COLORBUTTON
#pragma once

#ifndef OT_COLORPOPUP
	#include "OptionTreeColorPopUp.h"
#endif

void AFXAPI DDX_FontSelColorButton(CDataExchange *pDX, int nIDC, COLORREF& crColor);

class COptionTreeFontSelColorButton : public CButton
{
public:
	DECLARE_DYNCREATE(COptionTreeFontSelColorButton);

	COptionTreeFontSelColorButton();
	virtual ~COptionTreeFontSelColorButton();	
	COLORREF GetColor() const;
	void SetColor(COLORREF crColor);
	COLORREF GetDefaultColor() const;
	void SetDefaultColor(COLORREF crColor);
	void SetCustomText(LPCTSTR tszText);
	void SetDefaultText(LPCTSTR tszText);
	void SetTrackSelection(BOOL bTrack);
	BOOL GetTrackSelection() const;

	//{{AFX_VIRTUAL(COptionTreeFontSelColorButton)
    public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(COptionTreeFontSelColorButton)
    afx_msg BOOL OnClicked();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
	afx_msg long OnSelEndOK(UINT lParam, long wParam);
    afx_msg long OnSelEndCancel(UINT lParam, long wParam);
    afx_msg long OnSelChange(UINT lParam, long wParam);
	static void DrawArrow(CDC* pDC, RECT* pRect, int nDirection = 0, COLORREF crArrow = RGB(0, 0, 0));


	DECLARE_MESSAGE_MAP()

	COLORREF m_crColor;
	COLORREF m_crDefaultColor;
	CString m_strDefaultText;
	CString m_strCustomText;
	BOOL m_bPopupActive;
	BOOL m_bTrackSelection;

private:

	typedef CButton _Inherited;

};

#endif //!OT_FONTSEL_COLORBUTTON

