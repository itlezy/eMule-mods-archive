// SizeableDlg.cpp: implementation of the CSizeableDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "msgviewer.h"
#include "SizeableDlg.h"

#include <afxtempl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

struct WND_SIZE_DATA {
	int cx_now;
	int cy_now;

	CRect stFirst;

	inline WND_SIZE_DATA() { ZeroMemory(this, sizeof(WND_SIZE_DATA)); }

	// copy contructor
	inline WND_SIZE_DATA(const WND_SIZE_DATA& stOther) : 
		cx_now(stOther.cx_now),
		cy_now(stOther.cy_now),
		stFirst(stOther.stFirst) {}

};

void CSizeableDlg::OnSizeControl(CWnd &stWnd, int cx, int cy, UINT nLocks)
{
	static CMap <CWnd*, CWnd*, WND_SIZE_DATA, WND_SIZE_DATA&> mapWnds;

	if (!IsWindow(stWnd))
		return;

	// check if this window was already measured by us
	WND_SIZE_DATA stSD;
	if (mapWnds.Lookup(&stWnd, stSD))
	{
		CRect stRect = stSD.stFirst;

		if (nLocks & 2)
		{
			stRect.right += cx - stSD.cx_now;
			if (nLocks & 1)
				stRect.left += cx - stSD.cx_now;
		}

		if (nLocks & 8)
		{
			stRect.bottom += cy - stSD.cy_now;
			if (nLocks & 4)
				stRect.top += cy - stSD.cy_now;
		}

		stWnd.MoveWindow(&stRect);
	} else
	{
		// this is a new window
		WND_SIZE_DATA stSD;
		stSD.cx_now = cx;
		stSD.cy_now = cy;

		stWnd.GetWindowRect(&stSD.stFirst);
		ScreenToClient(&stSD.stFirst);

		mapWnds.SetAt(&stWnd, stSD);
	}
}

void CSizeableDlg::OnSizeControl(UINT nDlgCtrlID, int cx, int cy, UINT nLocks)
{
	CWnd* pWnd = GetDlgItem(nDlgCtrlID);
	if (pWnd)
		OnSizeControl(*pWnd, cx, cy, nLocks);
}
