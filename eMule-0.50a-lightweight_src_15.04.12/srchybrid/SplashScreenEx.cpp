// SplashScreenEx.cpp : implementation file
// by John O'Byrne 01/10/2002

//Xman small modifications to work with unicode and emule

#include "stdafx.h"
#include "SplashScreenEx.h"
#include "preferences.h"
#include "otherfunctions.h"
#include "emule.h"
/*
#ifndef AW_HIDE
#define AW_HIDE 0x00010000
#define AW_BLEND 0x00080000
#endif
*/
#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW   0x00020000
#endif

// CSplashScreenEx
#define RCWIDTH	200
#define MINWIDTH	217
//#define TEXTWIDTH	(sizeof(_T("eMule v") + theApp.m_strCurVersionLong)/sizeof(TCHAR)-1)*200 
#define TEXTWIDTH (sizeof(_T("eMule v0.50a"))/sizeof(TCHAR)-1)*17 

#define WINHEIGHT 55
#if 1
#define WINWIDTH TEXTWIDTH
#else
#define WINWIDTH MINWIDTH
#endif

IMPLEMENT_DYNAMIC(CSplashScreenEx, CWnd)
CSplashScreenEx::CSplashScreenEx()
: m_myFont(_T("Tahoma"), 13, FontStyleBold) 
, m_myFont2(_T("Tahoma"), 10)
, m_myBrush(
	Point(0, 0),
	Point(0, 18),
	Color(255, 80, 255, 255),
	Color(255, 0, 130, 255))
, m_rcText((WINWIDTH - TEXTWIDTH)/2, 0, TEXTWIDTH, 18) 
, m_rcText2((WINWIDTH - RCWIDTH)/2, WINHEIGHT - 32, RCWIDTH, 19)
, m_bgBrush(
	Point(0, 0),
	Point(0, WINHEIGHT),
	Color(128, 255, 255, 255),
	Color(192, 255, 255, 255))
{
	m_pWndParent=NULL;
	isStart=false;
	m_dwStyle = 0;
	format.SetAlignment(StringAlignmentCenter);
	m_strText2.Empty();
	gp.AddArc(0, 0, 10, 10, 180, 90);
	gp.AddArc(WINWIDTH - 10 - 2, 0, 10, 10, 270, 90);
	gp.AddArc(WINWIDTH - 10 - 2, WINHEIGHT - 10 - 2, 10, 10, 0, 90);
	gp.AddArc(0, WINHEIGHT - 10 - 2, 10, 10, 90, 90);
	gp.CloseAllFigures();
}

CSplashScreenEx::~CSplashScreenEx()
{
}

BOOL CSplashScreenEx::Create(CWnd *pWndParent,DWORD dwStyle)
{
	if(pWndParent)
		m_pWndParent = pWndParent;
	else
		m_pWndParent = this;
	m_dwStyle = dwStyle;

	WNDCLASSEX wcx; 

	wcx.cbSize = sizeof(wcx);
	wcx.lpfnWndProc = AfxWndProc;
	wcx.style = CS_DBLCLKS|CS_SAVEBITS;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = AfxGetInstanceHandle();
	wcx.hIcon = NULL;
	wcx.hCursor = LoadCursor(NULL,IDC_ARROW);
	wcx.hbrBackground=::GetSysColorBrush(COLOR_WINDOW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = _T("SplashScreenExClass");
	wcx.hIconSm = NULL;

	if (m_dwStyle & CSS_SHADOW)
		wcx.style|=CS_DROPSHADOW;

	static ATOM classAtom = RegisterClassEx(&wcx);

	// didn't work? try not using dropshadow (may not be supported)

	if (classAtom==NULL)
	{
		if (m_dwStyle & CSS_SHADOW)
		{
			wcx.style &= ~CS_DROPSHADOW;
			classAtom = RegisterClassEx(&wcx);
		}
		else
			return FALSE;
	}

	if (!CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,_T("SplashScreenExClass"),NULL,WS_POPUP,0,0,0,0,m_pWndParent->m_hWnd,NULL))
		return FALSE;

	return TRUE;
}

//Xman new slpash-screen arrangement
void CSplashScreenEx::SetText(LPCTSTR szText)
{
	m_strText2=szText;
	if(isStart)
		pos+=20.0f;
	else
		pos-=18.0f;
	DrawWindow();
}
void CSplashScreenEx::SetText2(LPCTSTR szText)
{
	m_strText2=szText;
	DrawWindow();
}
//Xman end

void CSplashScreenEx::Show(bool start)
{
	isStart=start;
	pos=(isStart) ? 0.0f : m_rcText2.Width;

	DrawWindow();

	SetWindowPos(NULL,(GetSystemMetrics(SM_CXFULLSCREEN)-WINWIDTH)/2,(GetSystemMetrics(SM_CYFULLSCREEN)-WINHEIGHT)/2,WINWIDTH,WINHEIGHT,SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);

	ShowWindow(SW_SHOW);
}

void CSplashScreenEx::Hide()
{
	ShowWindow(SW_HIDE);
	//DestroyWindow(); //Xman to use with CSS_HIDEONCLICK, emule will destroy it!
}

BEGIN_MESSAGE_MAP(CSplashScreenEx, CWnd)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CSplashScreenEx::DrawWindow()
{
	HDC hdc = ::GetDC(HWND_DESKTOP);
	CDC* pdc = CDC::FromHandle(hdc);
	CDC dc;
	dc.CreateCompatibleDC(pdc);
	CBitmap bgbitmap;
	bgbitmap.CreateCompatibleBitmap(pdc, WINWIDTH, WINWIDTH);
	dc.SelectObject(bgbitmap);
	Graphics g(dc.GetSafeHdc());
	g.SetCompositingMode(CompositingModeSourceCopy);
	SolidBrush brush(Color(255, 255, 255, 255));
	Pen pen(&brush);
	g.DrawPath(&pen, &gp);
	g.FillPath(&m_bgBrush, &gp);

	// Draw Text
	g.SetTextRenderingHint(TextRenderingHintAntiAlias);
	g.DrawString(_T("eMule v") + theApp.m_strCurVersionLong, -1, &m_myFont, m_rcText, &format, &m_myBrush);
	g.DrawString(m_strText2, -1, &m_myFont2, m_rcText2, &format, &m_myBrush);
	g.FillRectangle(&m_myBrush, m_rcText2.X, m_rcText2.GetBottom(), pos, 8.0);

	POINT ptSrc = {0,0};
	SIZE szWindow = {WINWIDTH, WINHEIGHT};
	BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255,AC_SRC_ALPHA };
	UpdateLayeredWindow(NULL, NULL, &szWindow, &dc,&ptSrc, 0, &blendPixelFunction, ULW_ALPHA);
	::ReleaseDC(HWND_DESKTOP, hdc);
}

// CSplashScreenEx message handlers
BOOL CSplashScreenEx::PreTranslateMessage(MSG* pMsg)
{
	// If a key is pressed, Hide the Splash Screen and destroy it
	if (m_dwStyle & CSS_HIDEONCLICK)
	{
		if (pMsg->message == WM_KEYDOWN ||
			pMsg->message == WM_SYSKEYDOWN ||
			pMsg->message == WM_LBUTTONDOWN ||
			pMsg->message == WM_RBUTTONDOWN ||
			pMsg->message == WM_MBUTTONDOWN ||
			pMsg->message == WM_NCLBUTTONDOWN ||
			pMsg->message == WM_NCRBUTTONDOWN ||
			pMsg->message == WM_NCMBUTTONDOWN)
		{
			Hide();
			return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CSplashScreenEx::OnTimer(UINT_PTR /*nIDEvent*/)
{
	KillTimer(0);
	Hide();
}

void CSplashScreenEx::PostNcDestroy()
{
	CWnd::PostNcDestroy();
	//delete this; //Xman modified: emule destroys manually
}