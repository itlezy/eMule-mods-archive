// SplashScreenEx.cpp : header file
// by John O'Byrne 01/10/2002

#pragma once
#include "GDIPlusUtil.h"

#define CSS_FADEIN		0x0001
#define CSS_FADEOUT		0x0002
#define CSS_FADE		CSS_FADEIN | CSS_FADEOUT
#define CSS_SHADOW		0x0004
#define CSS_CENTERSCREEN	0x0008
#define CSS_CENTERAPP		0x0010
#define CSS_HIDEONCLICK		0x0020

#define CSS_TEXT_NORMAL		0x0000
#define CSS_TEXT_BOLD		0x0001
#define CSS_TEXT_ITALIC		0x0002
#define CSS_TEXT_UNDERLINE	0x0004

// CSplashScreenEx

class CSplashScreenEx : public CWnd
{
	DECLARE_DYNAMIC(CSplashScreenEx)

public:
	CSplashScreenEx();
	virtual ~CSplashScreenEx();

	BOOL Create(CWnd *pWndParent,DWORD dwStyle=CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW);
	void Show(bool start);
	void Hide();

	void SetText(LPCTSTR szText);  //Xman new slpash-screen arrangement
	void SetText2(LPCTSTR szText);  //Xman new slpash-screen arrangement
	void SetTextRect(CRect& rcText);
	
protected:
	CWnd *m_pWndParent;
	Gdiplus::Font m_myFont;
	Gdiplus::Font m_myFont2; //Xman new slpash-screen arrangement
	RectF m_rcText;
	RectF m_rcText2; //Xman new slpash-screen arrangement
	StringFormat format;
	LinearGradientBrush m_myBrush;
	LinearGradientBrush m_bgBrush;
	GraphicsPath gp;
	CBitmap bgbitmap;

	DWORD m_dwStyle;
	CString m_strText;
	CString m_strText2; //Xman new slpash-screen arrangement
	float pos;
	bool isStart;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy();
	void DrawWindow();
};


