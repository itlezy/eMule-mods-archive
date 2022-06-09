#if !defined(_DEFERPOS_H_2E1EF384_AC30_11D3_A458_000629B2F85_INCLUDED_)
#define _DEFERPOS_H_2E1EF384_AC30_11D3_A458_000629B2F85_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// This class wraps the BeginDeferWindowPos/DeferWindowPos/EndDeferWindowPos
// APIs using a "resource allocation is acquisition" idiom.

class CDeferPos
{
public:
	CDeferPos(int nWindows=1);
	~CDeferPos();

	BOOL MoveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint);
	BOOL SetWindowPos(HWND hWnd, HWND hWndAfter, int x, int y, int nWidth,
		int nHeight, UINT uFlags);
#ifdef	_MFC_VER
	BOOL MoveWindow(CWnd* pWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint);
	BOOL SetWindowPos(CWnd* pWnd, CWnd* pWndAfter, int x, int y, int nWidth,
		int nHeight, UINT uFlags);
#endif

private:
	HDWP m_hdwp;
};

#endif // !defined(_DEFERPOS_H_2E1EF384_AC30_11D3_A458_000629B2F85_INCLUDED_)
