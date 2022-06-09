// TaskbarNotifier.cpp : implementation file
// By John O'Byrne
// 01 July 2008 - Corrected wrong window placement in Wine (which reports weird taskbar location)
// 08 February 2008 - replaced the window callback function AfxWndProc() to TaskbarWndProc() to fix the MSUL issue.
//                    (for more details see http://www.trigeminal.com/usenet/usenet031.asp?1033)
// 11 August 2002: - Timer precision is now determined automatically
//                 Complete change in the way the popup is showing (thanks to this,now the popup can be always on top, it shows even while watching a movie)
//                 The popup doesn't steal the focus anymore (by replacing ShowWindow(SW_SHOW) by ShowWindow(SW_SHOWNOACTIVATE))
//                 Thanks to Daniel Lohmann, update in the way the taskbar pos is determined (more flexible now)
// 17 July 2002: - Another big Change in the method for determining the pos of the taskbar (using the SHAppBarMessage function)
// 16 July 2002: - Thanks to the help of Daniel Lohmann, the Show Function timings work perfectly now ;)
// 15 July 2002: - Change in the method for determining the pos of the taskbar
//               (now handles the presence of quick launch or any other bar).
//               Remove the Handlers for WM_CREATE and WM_DESTROY
//               SetSkin is now called SetBitmap
// 14 July 2002: - Changed the GenerateRegion func by a new one (to correct a win98 bug)

#include "stdafx.h"
#include "TaskbarNotifier.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define IDT_HIDDEN			0
#define IDT_APPEARING		1
#define IDT_WAITING			2
#define IDT_DISAPPEARING	3
#define TASKBAR_X_TOLERANCE	20
#define TASKBAR_Y_TOLERANCE 20

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)	// System Hand cursor
#endif

static LRESULT CALLBACK TaskbarWndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

IMPLEMENT_DYNAMIC(CTaskbarNotifier, CWnd)
CTaskbarNotifier::CTaskbarNotifier() : m_strCaption(_T(""))
{
	m_pWndParent = NULL;
	m_bMouseIsOver = FALSE;
	m_bStarted = FALSE;
	m_hBitmapRegion = NULL;
	m_hCursor = NULL;
	m_crNormalTextColor = RGB(133, 146, 181);
	m_crSelectedTextColor = RGB(10, 36, 106);
	m_nBitmapHeight = 0;
	m_nBitmapWidth = 0;
	
	m_dwTimeToStay = 0;
	m_dwShowEvents = 0;
	m_dwHideEvents = 0;
	m_nCurrentPosX = 0;
	m_nCurrentPosY = 0;
	m_nCurrentWidth = 0;
	m_nCurrentHeight = 0;
	m_nIncrementShow = 0;
	m_nIncrementHide = 0;	
	m_nTaskbarPlacement = ABE_BOTTOM;
	m_nAnimStatus = IDT_HIDDEN;
	m_rcText.SetRect(0, 0, 0, 0);
	m_hCursor = ::LoadCursor(NULL, IDC_HAND);

	m_nActiveMessageType  = TBN_NULL;
	m_nMessageTypeClicked = TBN_NULL;
	
	// If running on NT, timer precision is 10 ms, if not timer precision is 50 ms
	OSVERSIONINFO	osvi;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		m_dwTimerPrecision = 10;
	else
		m_dwTimerPrecision = 50;

	SetTextDefaultFont(); // We use default GUI Font
}

CTaskbarNotifier::~CTaskbarNotifier()
{
	// No need to delete the HRGN,  SetWindowRgn() owns it after being called
}

int CTaskbarNotifier::Create(CWnd *pWndParent)
{
	ASSERT(pWndParent != NULL);
	m_pWndParent = pWndParent;

	WNDCLASSEX		wcx;

	wcx.cbSize = sizeof(wcx);
	wcx.lpfnWndProc = TaskbarWndProc;
	wcx.style = CS_DBLCLKS | CS_SAVEBITS;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = AfxGetInstanceHandle();
	wcx.hIcon = NULL;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = ::GetSysColorBrush(COLOR_WINDOW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = _T("TaskbarNotifierClass");
	wcx.hIconSm = NULL;

	RegisterClassEx(&wcx);

	return CreateEx(WS_EX_TOPMOST, _T("TaskbarNotifierClass"), NULL, WS_POPUP, 0, 0, 0, 0, pWndParent->m_hWnd, NULL);
}

void CTaskbarNotifier::SetTextFont(LPCTSTR szFont, int nSize, int nNormalStyle, int nSelectedStyle)
{
	LOGFONT		lf;

	m_fontNormal.DeleteObject();
	m_fontNormal.CreatePointFont(nSize, szFont);
	m_fontNormal.GetLogFont(&lf);

	// We  set the Font of the unselected ITEM
	if (nNormalStyle & TN_TEXT_BOLD)
		lf.lfWeight = FW_BOLD;
	else
		lf.lfWeight = FW_NORMAL;

	if (nNormalStyle & TN_TEXT_ITALIC)
		lf.lfItalic = TRUE;
	else
		lf.lfItalic = FALSE;

	if (nNormalStyle & TN_TEXT_UNDERLINE)
		lf.lfUnderline = TRUE;
	else
		lf.lfUnderline = FALSE;

	m_fontNormal.DeleteObject();
	m_fontNormal.CreateFontIndirect(&lf);

	// We set the Font of the selected ITEM
	if (nSelectedStyle & TN_TEXT_BOLD)
		lf.lfWeight = FW_BOLD;
	else
		lf.lfWeight = FW_NORMAL;

	if (nSelectedStyle & TN_TEXT_ITALIC)
		lf.lfItalic = TRUE;
	else
		lf.lfItalic = FALSE;

	if (nSelectedStyle & TN_TEXT_UNDERLINE)
		lf.lfUnderline = TRUE;
	else
		lf.lfUnderline = FALSE;

	m_fontSelected.DeleteObject();
	m_fontSelected.CreateFontIndirect(&lf);
}

void CTaskbarNotifier::SetTextDefaultFont()
{
	LOGFONT		lf;
	CFont		*pFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	pFont->GetLogFont(&lf);
	m_fontNormal.DeleteObject();
	m_fontSelected.DeleteObject();
	m_fontNormal.CreateFontIndirect(&lf);
	lf.lfUnderline = TRUE;
	m_fontSelected.CreateFontIndirect(&lf);
}

void CTaskbarNotifier::SetTextColor(COLORREF crNormalTextColor, COLORREF crSelectedTextColor)
{
	m_crNormalTextColor = crNormalTextColor;
	m_crSelectedTextColor = crSelectedTextColor;
	RedrawWindow(&m_rcText);
}

void CTaskbarNotifier::SetTextRect(RECT rcText)
{
	m_rcText = rcText;
}

BOOL CTaskbarNotifier::SetBitmap(UINT nBitmapID, int red, int green, int blue)
{
	BITMAP		bm;
	
	m_bitmapBackground.DeleteObject();
	if (m_bitmapBackground.LoadBitmap(nBitmapID) == NULL)
		return FALSE;

	m_bitmapBackground.GetBitmap(&bm);
	m_nBitmapWidth = bm.bmWidth;
	m_nBitmapHeight = bm.bmHeight;

	if (red != -1 && green != -1 && blue != -1)
	{
		// No need to delete the HRGN, SetWindowRgn() owns it after being called
		m_hBitmapRegion = CreateRgnFromBitmap(m_bitmapBackground, RGB(red, green, blue));
		SetWindowRgn(m_hBitmapRegion, true);
	}

	return TRUE;
}

BOOL CTaskbarNotifier::SetBitmap(CBitmap *pBitmap, int red, int green, int blue)
{
	BITMAP		bm;
	
	m_bitmapBackground.DeleteObject();
	if (m_bitmapBackground.Attach(pBitmap->Detach()) == NULL)
		return FALSE;

	m_bitmapBackground.GetBitmap(&bm);
	m_nBitmapWidth = bm.bmWidth;
	m_nBitmapHeight = bm.bmHeight;

	if (red != -1 && green != -1 && blue != -1)
	{
		// No need to delete the HRGN, SetWindowRgn() owns it after being called
		m_hBitmapRegion = CreateRgnFromBitmap(m_bitmapBackground, RGB(red, green, blue));
		SetWindowRgn(m_hBitmapRegion, true);
	}

	return TRUE;
}

BOOL CTaskbarNotifier::SetBitmap(LPCTSTR szFileName, int red, int green, int blue)
{
	BITMAP		bm;
	HBITMAP		hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	if (hBmp == NULL)
		return FALSE;

	m_bitmapBackground.DeleteObject();
	m_bitmapBackground.Attach(hBmp);
	m_bitmapBackground.GetBitmap(&bm);
	m_nBitmapWidth = bm.bmWidth;
	m_nBitmapHeight = bm.bmHeight;

	if (red != -1 && green != -1 && blue != -1)
	{
		// No need to delete the HRGN, SetWindowRgn() owns it after being called
		m_hBitmapRegion = CreateRgnFromBitmap(m_bitmapBackground, RGB(red, green, blue));
		SetWindowRgn(m_hBitmapRegion, true);
	}

	return TRUE;
}

void CTaskbarNotifier::Show(const CString &strCaption, int nMsgType, DWORD dwTimeToShow, DWORD dwTimeToStay, DWORD dwTimeToHide)
{
	int		iScreenWidth, iScreenHeight;
	UINT	nEvents, nBitmapSize;
	CRect	rcTaskbar;

	m_strCaption = strCaption;
	m_dwTimeToStay = dwTimeToStay;
	
	iScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	HWND	hWndTaskbar = ::FindWindow(_T("Shell_TrayWnd"), 0);

	::GetWindowRect(hWndTaskbar, &rcTaskbar);

	if (rcTaskbar.Width() >= rcTaskbar.Height())
	{
		// Taskbar is on top or on bottom
		m_nTaskbarPlacement = (((rcTaskbar.top + rcTaskbar.bottom) / 2) < (iScreenHeight >> 1)) ? ABE_TOP : ABE_BOTTOM;
	//	Correct weird taskbar position in Wine
		if (abs(iScreenWidth - rcTaskbar.right) > TASKBAR_X_TOLERANCE)
			rcTaskbar.right = iScreenWidth;
		nBitmapSize = m_nBitmapHeight;
	}
	else
	{
		// Taskbar is on left or on right
		m_nTaskbarPlacement = (((rcTaskbar.left + rcTaskbar.right) / 2) < (iScreenWidth >> 1)) ? ABE_LEFT : ABE_RIGHT;
	//	Correct weird taskbar position in Wine
		if (abs(iScreenHeight - rcTaskbar.bottom) > TASKBAR_Y_TOLERANCE)
			rcTaskbar.bottom = iScreenHeight;
		nBitmapSize = m_nBitmapWidth;
	}

	// We calculate the pixel increment and the timer value for the showing animation
	if (dwTimeToShow > m_dwTimerPrecision)
	{
		nEvents = min((dwTimeToShow / m_dwTimerPrecision) / 2, nBitmapSize);
		m_dwShowEvents = dwTimeToShow / nEvents;
		m_nIncrementShow = nBitmapSize / nEvents;
	}
	else
	{
		m_dwShowEvents = m_dwTimerPrecision;
		m_nIncrementShow = nBitmapSize;
	}

	// We calculate the pixel increment and the timer value for the hiding animation
	if (dwTimeToHide > m_dwTimerPrecision)
	{
		nEvents = min((dwTimeToHide / m_dwTimerPrecision / 2), nBitmapSize);
		m_dwHideEvents = dwTimeToHide / nEvents;
		m_nIncrementHide = nBitmapSize / nEvents;
	}
	else
	{
		m_dwShowEvents = m_dwTimerPrecision;
		m_nIncrementHide = nBitmapSize;
	}
	
	// Compute init values for the animation
	switch (m_nAnimStatus)
	{
		case IDT_HIDDEN:
		{
			if (m_nTaskbarPlacement == ABE_RIGHT)
			{
				m_nCurrentPosX = rcTaskbar.left;
				m_nCurrentPosY = rcTaskbar.bottom-m_nBitmapHeight;
				m_nCurrentWidth = 0;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			else if (m_nTaskbarPlacement == ABE_LEFT)
			{
				m_nCurrentPosX = rcTaskbar.right;
				m_nCurrentPosY = rcTaskbar.bottom-m_nBitmapHeight;
				m_nCurrentWidth = 0;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			else if (m_nTaskbarPlacement == ABE_TOP)
			{
				m_nCurrentPosX = rcTaskbar.right-m_nBitmapWidth;
				m_nCurrentPosY = rcTaskbar.bottom;
				m_nCurrentWidth = m_nBitmapWidth;
				m_nCurrentHeight = 0;
			}
			else //if (m_nTaskbarPlacement == ABE_BOTTOM)
			{
				// Taskbar is on the bottom or Invisible
				m_nCurrentPosX = rcTaskbar.right-m_nBitmapWidth;
				m_nCurrentPosY = rcTaskbar.top;
				m_nCurrentWidth = m_nBitmapWidth;
				m_nCurrentHeight = 0;
			}

			CWnd	*pWnd = GetForegroundWindow();

			ShowWindow(SW_SHOWNOACTIVATE);
			pWnd->SetForegroundWindow();
			m_nActiveMessageType = nMsgType;
			SetTimer(IDT_APPEARING, m_dwShowEvents, NULL);
			break;
		}
		case IDT_APPEARING:
			RedrawWindow(&m_rcText);
			break;

		case IDT_WAITING:
			RedrawWindow(&m_rcText);
			KillTimer(IDT_WAITING);
			SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
			break;

		case IDT_DISAPPEARING:
			KillTimer(IDT_DISAPPEARING);
			SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
			if (m_nTaskbarPlacement == ABE_RIGHT)
			{
				m_nCurrentPosX = rcTaskbar.left - m_nBitmapWidth;
				m_nCurrentWidth = m_nBitmapWidth;
			}
			else if (m_nTaskbarPlacement == ABE_LEFT)
			{
				m_nCurrentPosX = rcTaskbar.right;
				m_nCurrentWidth = m_nBitmapWidth;
			}
			else if (m_nTaskbarPlacement == ABE_TOP)
			{
				m_nCurrentPosY = rcTaskbar.bottom;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			else //if (m_nTaskbarPlacement == ABE_BOTTOM)
			{
				m_nCurrentPosY = rcTaskbar.top - m_nBitmapHeight;
				m_nCurrentHeight = m_nBitmapHeight;
			}
			
			SetWindowPos(&wndTopMost, m_nCurrentPosX, m_nCurrentPosY, m_nCurrentWidth, m_nCurrentHeight, SWP_NOACTIVATE);
			RedrawWindow(&m_rcText);
			break;
	}
}

void CTaskbarNotifier::Hide()
{
	switch (m_nAnimStatus)
	{
		case IDT_APPEARING:
		case IDT_WAITING:
		case IDT_DISAPPEARING:
			KillTimer(m_nAnimStatus);
			break;
	}
	MoveWindow(0, 0, 0, 0);
	ShowWindow(SW_HIDE);
	m_nAnimStatus = IDT_HIDDEN;
	m_nActiveMessageType = TBN_NULL;
}

HRGN CTaskbarNotifier::CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color)
{
	// this code is written by Davide Pizzolato

	if (hBmp == NULL)
		return NULL;

	CDC		*pDC = GetDC();

	if (pDC == NULL)
		return NULL;

	BITMAP		bm;
	CDC		dcBmp;

	GetObject(hBmp, sizeof(bm), &bm);	// get bitmap attributes

	dcBmp.CreateCompatibleDC(pDC);	//Creates a memory device context for the bitmap
	ReleaseDC(pDC);
	
	HGDIOBJ	hOldBitmap = dcBmp.SelectObject(hBmp); //selects the bitmap in the device context
	const	DWORD MAXBUF = 40;		// size of one block in RECTs

	// (i.e. MAXBUF * sizeof(RECT) in bytes)
	LPRECT	pRects;								
	DWORD	cBlocks = 0;			// number of allocated blocks

	INT		i, j;					// current position in mask image
	INT		first = 0;				// left position of current scan line
	// where mask was found
	bool	wasfirst = false;		// set when if mask was found in current scan line
	bool	ismask;					// set when current color is mask color

	// allocate memory for region data
	RGNDATAHEADER	*pRgnData = (RGNDATAHEADER*)new BYTE[sizeof(RGNDATAHEADER) + ++cBlocks * MAXBUF * sizeof(RECT)];
	
	memzero(pRgnData, sizeof(RGNDATAHEADER) + cBlocks * MAXBUF * sizeof(RECT));
	// fill it by default
	pRgnData->dwSize	= sizeof(RGNDATAHEADER);
	pRgnData->iType		= RDH_RECTANGLES;
	pRgnData->nCount	= 0;
	for (i = 0; i < bm.bmHeight; i++)
	{
		for (j = 0; j < bm.bmWidth; j++)
		{
			// get color
			ismask = (dcBmp.GetPixel(j, bm.bmHeight - i - 1) != color);
			// place part of scan line as RECT region if transparent color found after mask color or
			// mask color found at the end of mask image
			if (wasfirst && ((ismask && (j == (bm.bmWidth-1))) || (ismask ^ (j < bm.bmWidth))))
			{
				// get offset to RECT array if RGNDATA buffer
				pRects = (LPRECT)((LPBYTE)pRgnData + sizeof(RGNDATAHEADER));
				// save current RECT
				pRects[pRgnData->nCount++] = CRect(first, bm.bmHeight - i - 1, j + (j == (bm.bmWidth - 1)), bm.bmHeight - i);
				// if buffer full reallocate it
				if (pRgnData->nCount >= cBlocks * MAXBUF)
				{
					LPBYTE	pRgnDataNew = new BYTE[sizeof(RGNDATAHEADER) + ++cBlocks * MAXBUF * sizeof(RECT)];

					memcpy2(pRgnDataNew, pRgnData, sizeof(RGNDATAHEADER) + (cBlocks - 1) * MAXBUF * sizeof(RECT));
					delete[] pRgnData;
					pRgnData = (RGNDATAHEADER*)pRgnDataNew;
				}
				wasfirst = false;
			}
			else if (!wasfirst && ismask)
			{	// set wasfirst when mask is found
				first = j;
				wasfirst = true;
			}
		}
	}
	dcBmp.SelectObject(hOldBitmap); //restore default object
	dcBmp.DeleteDC();	//release the bitmap
	// create region
	/*  Under WinNT the ExtCreateRegion returns NULL (by Fable@aramszu.net) */
	//	HRGN hRgn = ExtCreateRegion( NULL, sizeof(RGNDATAHEADER) + pRgnData->nCount * sizeof(RECT), (LPRGNDATA)pRgnData );
	/* ExtCreateRegion replacement { */
	HRGN	hRgn = CreateRectRgn(0, 0, 0, 0), hr;

	ASSERT( hRgn != NULL );
	pRects = (LPRECT)((LPBYTE)pRgnData + sizeof(RGNDATAHEADER));
	for (i=0; i < (int)pRgnData->nCount; i++)
	{
		hr = CreateRectRgn(pRects[i].left, pRects[i].top, pRects[i].right, pRects[i].bottom);
		VERIFY(CombineRgn(hRgn, hRgn, hr, RGN_OR) != ERROR);
		if (hr != NULL)
			DeleteObject(hr);
	}
	ASSERT( hRgn != NULL );
	/* } ExtCreateRegion replacement */

	delete[] pRgnData;
	ReleaseDC(pDC);
	return hRgn;
}

int CTaskbarNotifier::GetMessageType()
{	
	return m_nMessageTypeClicked;
}


BEGIN_MESSAGE_MAP(CTaskbarNotifier, CWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CTaskbarNotifier message handlers

void CTaskbarNotifier::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT		tme;

	tme.cbSize      = sizeof(tme);
	tme.dwFlags     = TME_LEAVE | TME_HOVER;
	tme.hwndTrack   = m_hWnd;
	tme.dwHoverTime = 1;

	// We Tell Windows we want to receive WM_MOUSEHOVER and WM_MOUSELEAVE
	::_TrackMouseEvent(&tme);

	CWnd::OnMouseMove(nFlags, point);
}

void CTaskbarNotifier::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect		rClient;

	GetClientRect(&rClient);
	if (point.x >= rClient.right - 8 - 18 && point.x <= rClient.right - 8 &&
		point.y >= rClient.top + 12 && point.y <= rClient.top + 12 + 20)
	{
		KillTimer(IDT_WAITING);
		SetTimer(IDT_DISAPPEARING,m_dwHideEvents, NULL);
		return;
	}
	
	//	Store m_nActiveMessageType state before the timer expires
	m_nMessageTypeClicked = m_nActiveMessageType;
	// Notify the parent window that the Notifier popup was clicked
	m_pWndParent->PostMessage(WM_TASKBARNOTIFIERCLICKED, 0, 0);
	CWnd::OnLButtonUp(nFlags, point);
}

LRESULT CTaskbarNotifier::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam); NOPRM(lParam);
	if (m_nAnimStatus == IDT_WAITING)
		KillTimer(IDT_WAITING);

	if (m_bMouseIsOver == FALSE)
	{
		m_bMouseIsOver = TRUE;
		RedrawWindow(&m_rcText);
		if (m_nAnimStatus == IDT_DISAPPEARING)
		{
			KillTimer(IDT_DISAPPEARING);
			m_nAnimStatus = IDT_APPEARING;
			SetTimer(IDT_APPEARING, m_dwShowEvents, NULL);
		}
	}
	return 0;
}

LRESULT CTaskbarNotifier::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	NOPRM(wParam); NOPRM(lParam);
	if (m_bMouseIsOver == TRUE)
	{
		m_bMouseIsOver = FALSE;
		RedrawWindow(&m_rcText);
		if (m_nAnimStatus == IDT_WAITING)
			SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
	}
	return 0;
}

BOOL CTaskbarNotifier::OnEraseBkgnd(CDC* pDC)
{
	CDC			memDC;
	CBitmap		*pOldBitmap;
	
	memDC.CreateCompatibleDC(pDC);
	pOldBitmap = memDC.SelectObject(&m_bitmapBackground);
	pDC->BitBlt(0, 0, m_nCurrentWidth, m_nCurrentHeight, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);

	return TRUE;
}

void CTaskbarNotifier::OnPaint()
{
	CPaintDC	dc(this);
	CFont		*pOldFont;
			
	if (m_bMouseIsOver)
	{
		dc.SetTextColor(m_crSelectedTextColor);
		pOldFont = dc.SelectObject(&m_fontSelected);
	}
	else
	{
		dc.SetTextColor(m_crNormalTextColor);
		pOldFont = dc.SelectObject(&m_fontNormal);
	}

	dc.SetBkMode(TRANSPARENT); 
	dc.DrawText(m_strCaption, m_rcText, DT_CENTER | DT_VCENTER | DT_NOPREFIX |
		DT_WORDBREAK | DT_END_ELLIPSIS | DT_PATH_ELLIPSIS);

	dc.SelectObject(pOldFont);
}

BOOL CTaskbarNotifier::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CTaskbarNotifier::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
		case IDT_APPEARING:
			m_nAnimStatus = IDT_APPEARING;
			switch(m_nTaskbarPlacement)
			{
				case ABE_BOTTOM:
					if (m_nCurrentHeight < m_nBitmapHeight)
					{
						m_nCurrentPosY -= m_nIncrementShow;
						m_nCurrentHeight += m_nIncrementShow;
					}
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;
				case ABE_TOP:
					if (m_nCurrentHeight < m_nBitmapHeight)
						m_nCurrentHeight += m_nIncrementShow;
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;
				case ABE_LEFT:
					if (m_nCurrentWidth < m_nBitmapWidth)
						m_nCurrentWidth += m_nIncrementShow;
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;
				case ABE_RIGHT:
					if (m_nCurrentWidth < m_nBitmapWidth)
					{
						m_nCurrentPosX -= m_nIncrementShow;
						m_nCurrentWidth += m_nIncrementShow;
					}
					else
					{
						KillTimer(IDT_APPEARING);
						SetTimer(IDT_WAITING, m_dwTimeToStay, NULL);
						m_nAnimStatus = IDT_WAITING;
					}
					break;
			}
			SetWindowPos(&wndTopMost, m_nCurrentPosX, m_nCurrentPosY, m_nCurrentWidth, m_nCurrentHeight, SWP_NOACTIVATE);
			break;

		case IDT_WAITING:
			KillTimer(IDT_WAITING);
			SetTimer(IDT_DISAPPEARING, m_dwHideEvents,NULL);
			break;

		case IDT_DISAPPEARING:
			m_nAnimStatus = IDT_DISAPPEARING;
			switch(m_nTaskbarPlacement)
			{
				case ABE_BOTTOM:
					if (m_nCurrentHeight > 0)
					{
						m_nCurrentPosY += m_nIncrementHide;
						m_nCurrentHeight -= m_nIncrementHide;
					}
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;
				case ABE_TOP:
					if (m_nCurrentHeight > 0)
						m_nCurrentHeight -= m_nIncrementHide;
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;
				case ABE_LEFT:
					if (m_nCurrentWidth > 0)
						m_nCurrentWidth -= m_nIncrementHide;
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;
				case ABE_RIGHT:
					if (m_nCurrentWidth > 0)
					{
						m_nCurrentPosX += m_nIncrementHide;
						m_nCurrentWidth -= m_nIncrementHide;
					}
					else
					{
						KillTimer(IDT_DISAPPEARING);
						Hide();
					}
					break;
			}
			SetWindowPos(&wndTopMost, m_nCurrentPosX, m_nCurrentPosY, m_nCurrentWidth, m_nCurrentHeight, SWP_NOACTIVATE);
			break;
	}

	CWnd::OnTimer(nIDEvent);
}
