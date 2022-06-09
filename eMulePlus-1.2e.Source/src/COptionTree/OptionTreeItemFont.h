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

#ifndef OT_ITEMFONT
#define OT_ITEMFONT

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionTreeItemFont.h : header file
//

// Added Headers
#include "CommonRes.h"
#include "OptionTreeDef.h"
#include "OptionTreeItem.h"
#include "OptionTreeFontSel.h"

class COptionTreeItemFont : public COptionTreeItem
{
public:
	COptionTreeItemFont();
	void SetOption(DWORD dwOption, BOOL bSet);
	BOOL GetOption(DWORD dwOption);
	void SetApplyWindow(CWnd *pWnd);
	void SetCustomSample(CString strSample);
	COLORREF GetDefaultTextColor();
	COLORREF GetTextColor();
	void SetTextColor(COLORREF crColor);
	void SetDefaultTextColor(COLORREF crColor);
	void SetDefaultFont(CHARFORMAT cfFont);
	void SetCurFont(CHARFORMAT cfFont);
	void SetDefaultFont(LOGFONT lf);
	void SetCurFont(LOGFONT lf);
	BOOL CreateFontItem(LOGFONT lfFont, COLORREF crFontColor, LOGFONT lfDefaultFont, COLORREF crDefaultFontColor, DWORD dwOptions);
	BOOL CreateFontItem(LOGFONT lfFont, COLORREF crFontColor, DWORD dwOptions);
	BOOL CreateFontItem(CHARFORMAT cfFont, COLORREF crFontColor, CHARFORMAT cfDefaultFont, COLORREF crDefaultFontColor, DWORD dwOptions);
	BOOL CreateFontItem(CHARFORMAT cfFont, COLORREF crFontColor, DWORD dwOptions);
	virtual ~COptionTreeItemFont();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void OnActivate();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);

protected:
	void ConvertFont(LOGFONT &lf, CHARFORMAT &cfFont);
	void ConvertFont(CHARFORMAT &cfFont, LOGFONT &lf);
	COLORREF m_crDefTextColor;
	COLORREF m_crTextColor;
	LOGFONT m_lfFont;
	LOGFONT m_lfDefaultFont;
	DWORD m_dwOptions;
	CString m_strCustomSample;
	CWnd *m_pApplyWindow;
	long m_lDefaultHeight;

};

#endif // !OT_ITEMFONT
