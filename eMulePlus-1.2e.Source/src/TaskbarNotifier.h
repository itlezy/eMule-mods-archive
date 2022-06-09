// CTaskbarNotifier Header file
// By John O'Byrne - 15 July 2002
// Modified by kei-kun
#pragma once

#define WM_TASKBARNOTIFIERCLICKED	WM_USER+123
#define TN_TEXT_NORMAL			0x0000
#define TN_TEXT_BOLD			0x0001
#define TN_TEXT_ITALIC			0x0002
#define TN_TEXT_UNDERLINE		0x0004

//START - enkeyDEV(kei-kun) -TaskbarNotifier-
#define TBN_NULL				0
#define TBN_CHAT				1
#define TBN_DLOAD				2
#define TBN_LOG					3
#define TBN_DLOAD_ADD			4
#define TBN_IMPORTANTEVENT		5
#define TBN_SCHEDULER			6
#define TBN_WEBSERVER			7
#define TBN_SERVER				8
//END - enkeyDEV(kei-kun) -TaskbarNotifier-

class CTaskbarNotifier : public CWnd
{
	DECLARE_DYNAMIC(CTaskbarNotifier)

public:
	CTaskbarNotifier();
	virtual ~CTaskbarNotifier();

	int Create(CWnd *pWndParent);
	void Show(const CString &strCaption, int nMsgType, DWORD dwTimeToShow=500, DWORD dwTimeToStay=5000, DWORD dwTimeToHide=200); // enkeyDEV(kei-kun) -TaskbarNotifier-
	void Hide();
	int GetMessageType();
	BOOL SetBitmap(UINT nBitmapID, int red = -1, int green = -1, int blue = -1);
	BOOL SetBitmap(LPCTSTR pcFileName, int red = -1, int green = -1, int blue = -1);
	BOOL SetBitmap(CBitmap *pBitmap, int red, int green, int blue);
	void SetTextFont(LPCTSTR szFont, int nSize, int nNormalStyle, int nSelectedStyle);
	void SetTextDefaultFont();
	void SetTextColor(COLORREF crNormalTextColor, COLORREF crSelectedTextColor);
	void SetTextRect(RECT rcText);

	CWnd *m_pWndParent;
	
	CFont m_fontNormal;
	CFont m_fontSelected;
	COLORREF m_crNormalTextColor;
	COLORREF m_crSelectedTextColor;
	HCURSOR m_hCursor;
	
	CBitmap m_bitmapBackground;
	HRGN m_hBitmapRegion;
	int m_nBitmapWidth;
	int m_nBitmapHeight;

	CString m_strCaption;
	CRect m_rcText;
	BOOL m_bMouseIsOver;
	BOOL m_bStarted;

	int m_nAnimStatus;
	int m_nTaskbarPlacement;
	DWORD m_dwTimerPrecision;
 	DWORD m_dwTimeToStay;
	DWORD m_dwShowEvents;
	DWORD m_dwHideEvents;
	int m_nCurrentPosX;
	int m_nCurrentPosY;
	int m_nCurrentWidth;
	int m_nCurrentHeight;
	int m_nIncrementShow;
	int m_nIncrementHide;
	int m_nActiveMessageType;  //<<--enkeyDEV(kei-kun) -TaskbarNotifier-
	int m_nMessageTypeClicked; //<<--enkeyDEV(kei-kun) -TaskbarNotifier-
	
protected:
	HRGN CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT nIDEvent);
};
