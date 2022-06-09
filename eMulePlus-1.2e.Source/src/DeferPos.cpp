#include "stdafx.h"
#include "DeferPos.h"

// Constructor
// This sets up the RAIA idiom by calling BeginDeferWindowPos. The number of windows
// can be passed as an argument to optimize memory management, although the API will
// grow the memory if needed at run time.

CDeferPos::CDeferPos(int nWindows)
{
	m_hdwp = BeginDeferWindowPos(nWindows);
}

// Destructor
// This concludes the RAIA idiom by ensuring EndDeferWindowPos is called.

CDeferPos::~CDeferPos()
{
	EndDeferWindowPos(m_hdwp);
}

// MoveWindow
// Emulates a call to ::MoveWindow but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.

BOOL CDeferPos::MoveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight,
						  BOOL bRepaint)
{
	UINT uFlags = SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER;
	if (!bRepaint)
		uFlags |= SWP_NOREDRAW;
	return SetWindowPos(hWnd, 0, x, y, nWidth, nHeight, uFlags);
}

// SetWindowPos
// Emulates a call to ::SetWindowPos but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.

BOOL CDeferPos::SetWindowPos(HWND hWnd, HWND hWndAfter, int x, int y, int nWidth,
							int nHeight, UINT uFlags)
{
	if (m_hdwp != 0)
	{
		m_hdwp = DeferWindowPos(m_hdwp, hWnd, hWndAfter, x, y, nWidth, nHeight,
			uFlags);
	}
	return m_hdwp != 0;
}

// MFC versions of the above.

#ifdef	_MFC_VER
BOOL CDeferPos::MoveWindow(CWnd* pWnd, int x, int y, int nWidth, int nHeight,
						   BOOL bRepaint)
{
	return MoveWindow(pWnd->GetSafeHwnd(), x, y, nWidth, nHeight, bRepaint);
}

BOOL CDeferPos::SetWindowPos(CWnd* pWnd, CWnd* pWndAfter, int x, int y, int nWidth,
							 int nHeight, UINT uFlags)
{
	return SetWindowPos(pWnd->GetSafeHwnd(), pWndAfter->GetSafeHwnd(), x, y, nWidth,
		nHeight, uFlags);
}
#endif
