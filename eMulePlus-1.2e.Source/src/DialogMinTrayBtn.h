// ------------------------------------------------------------
//  CDialogMinTrayBtn template class
//  MFC CDialog with minimize to systemtray button (0.04a)
//  Supports WinXP styles (thanks to David Yuheng Zhao for CVisualStylesXP - yuheng_zhao@yahoo.com)
// ------------------------------------------------------------
//  DialogMinTrayBtn.h
//  zegzav - 2002,2003 - eMule project (http://www.emule-project.net)
// ------------------------------------------------------------

#pragma once

#include "ThemeHelperST.h"

#define HTMINTRAYBUTTON	65
#define SC_MINIMIZETRAY	0xE000

template <class BASE = CDialog> class CDialogMinTrayBtn : public BASE
{
public:
	CDialogMinTrayBtn();
	CDialogMinTrayBtn(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	CDialogMinTrayBtn(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	virtual ~CDialogMinTrayBtn();

	void MinTrayBtnShow();
	void MinTrayBtnHide();
	BOOL MinTrayBtnIsVisible() const { return m_bMinTrayBtnVisible; }

	void MinTrayBtnEnable();
	void MinTrayBtnDisable();
	BOOL MinTrayBtnIsEnabled() const { return m_bMinTrayBtnEnabled; }

	void SetWindowText(LPCTSTR lpszString);

	static CThemeHelperST	m_themeHelper;

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
#if _MFC_VER>=0x0800
	afx_msg LRESULT OnNcHitTest(CPoint point);
#else
	afx_msg UINT OnNcHitTest(CPoint point);
#endif
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT _OnThemeChanged();
	DECLARE_MESSAGE_MAP()

private:
	void MinTrayBtnInit();
	void MinTrayBtnDraw();
	BOOL MinTrayBtnHitTest(CPoint point) const;
	void MinTrayBtnUpdatePosAndSize();

	void MinTrayBtnSetUp();
	void MinTrayBtnSetDown();

	const CPoint &MinTrayBtnGetPos() const { return m_MinTrayBtnPos; }
	const CSize &MinTrayBtnGetSize() const { return m_MinTrayBtnSize; }
	CRect MinTrayBtnGetRect() const { return CRect(MinTrayBtnGetPos(), MinTrayBtnGetSize()); }

	BOOL IsWindowsClassicStyle() const;
	TCHAR *GetVisualStylesXPColor();

	BOOL MinTrayBtnInitBitmap();

	BOOL (WINAPI *m_pfnTransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
	HMODULE		m_hMsImg32;
	CPoint		m_MinTrayBtnPos;
	CSize		m_MinTrayBtnSize;
	UINT_PTR	m_nMinTrayBtnTimerId;
	CBitmap		m_bmMinTrayBtnBitmap;
	BOOL		m_bMinTrayBtnVisible;
	BOOL		m_bMinTrayBtnEnabled;
	BOOL		m_bMinTrayBtnUp;
	BOOL		m_bMinTrayBtnCapture;
	BOOL		m_bMinTrayBtnActive;
	BOOL		m_bMinTrayBtnHitTest;
	BOOL		m_bMinTrayBtnWindowsClassicStyle;
};

#include "DialogMinTrayBtn.hpp"
