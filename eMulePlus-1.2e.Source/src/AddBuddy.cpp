//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//-----------------------------------------------------------------------------
// (p) 2003 by FoRcHa  ... (based on Gipsysoft's BuddyButton)

#include "stdafx.h"
#include "AddBuddy.h"


static const TCHAR g_szOldProc[] = _T("BuddyOldProc");
static const TCHAR g_szData[] = _T("BuddyData");

class CData
{
public:
	UINT m_uBuddyWidth;
	UINT m_uBuddyHeight;
	HWND m_hwndBuddy;
	UINT m_uStyle;
	BOOL m_bVisible;
};

static LRESULT CALLBACK SubClassedProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC WndProc = reinterpret_cast<WNDPROC>(::GetProp(hwnd, g_szOldProc));
	CData *pData = reinterpret_cast<CData*>(::GetProp(hwnd, g_szData));

	ASSERT(WndProc);

	switch(message)
	{
		case WM_NCDESTROY:
		{
			SetWindowLong(hwnd, GWL_WNDPROC, (long)WndProc);
			RemoveProp(hwnd, g_szOldProc);
			RemoveProp(hwnd, g_szData);

			delete pData;
			break;
		}
		case WM_NCHITTEST:
		{
			LRESULT lr = CallWindowProc(WndProc, hwnd, message, wParam, lParam);
			if(lr == HTNOWHERE)
			{
				lr = HTTRANSPARENT;
			}
			return lr;
		}
		case WM_NCCALCSIZE:
		{
			LRESULT lr = CallWindowProc(WndProc, hwnd, message, wParam, lParam);
			LPNCCALCSIZE_PARAMS lpnccs = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam);

			switch(pData->m_uStyle)
			{
				case BDS_LEFT:
					lpnccs->rgrc[0].left += pData->m_uBuddyWidth;
					break;
				case BDS_RIGHT:
					lpnccs->rgrc[0].right -= pData->m_uBuddyWidth;
					break;
				case BDS_TOP:
					lpnccs->rgrc[0].top += pData->m_uBuddyHeight;
					break;
				case BDS_BOTTOM:
					lpnccs->rgrc[0].bottom -= pData->m_uBuddyHeight;
					break;
				default:
					break;
			}

			return lr;
		}
		case WM_SIZE:
		{
			CRect rc;
			::GetClientRect(hwnd, rc);

			switch(pData->m_uStyle)
			{
				case BDS_LEFT:
					rc.right = rc.left;
					rc.left = rc.left - pData->m_uBuddyWidth;
					break;
				case BDS_RIGHT:
					rc.left = rc.right;
					rc.right = rc.left + pData->m_uBuddyWidth;
					break;
				case BDS_TOP:
					rc.bottom = rc.top;
					rc.top = rc.top - pData->m_uBuddyHeight;
					break;
				case BDS_BOTTOM:
					rc.top = rc.bottom;
					rc.bottom = rc.bottom + pData->m_uBuddyHeight;
					break;
			}

			::MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&rc, 2);
			::SetWindowPos(pData->m_hwndBuddy, NULL, rc.left, rc.top, 
					rc.Width(), rc.Height(), SWP_NOZORDER);
			break;
		}
		case WM_SHOWWINDOW:
		{
			if(static_cast<BOOL>(wParam) != pData->m_bVisible)
			{
				pData->m_bVisible = wParam;
				::ShowWindow(pData->m_hwndBuddy, wParam ? SW_SHOW : SW_HIDE);
			}
			break;
		}
	}

	return CallWindowProc(WndProc, hwnd, message, wParam, lParam);
}

BOOL AddBuddy(HWND hwndTarget, HWND hwndBuddy, UINT uStyle)
{
	if(uStyle == BDS_LEFT || uStyle == BDS_RIGHT || uStyle == BDS_TOP || uStyle == BDS_BOTTOM)
	{
		if(::IsWindow(hwndTarget) && ::IsWindow(hwndBuddy))
		{
			FARPROC lpfnWndProc = reinterpret_cast<FARPROC>(SetWindowLong(hwndTarget, GWL_WNDPROC, (long)SubClassedProc));
			ASSERT(lpfnWndProc != NULL);
			VERIFY(::SetProp(hwndTarget, g_szOldProc, reinterpret_cast<HANDLE>(lpfnWndProc)));
			
			CData *pData = new CData;
			pData->m_uStyle = uStyle;

			CRect rcBuddy;
			::GetWindowRect(hwndBuddy, rcBuddy);

			pData->m_uBuddyWidth = rcBuddy.Width();
			pData->m_uBuddyHeight = rcBuddy.Height();
			pData->m_hwndBuddy = hwndBuddy;
			pData->m_bVisible = -1;

			VERIFY(::SetProp(hwndTarget, g_szData, reinterpret_cast<HANDLE>(pData)));

			::SetWindowPos(hwndTarget, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
			
			return TRUE;
		}
		else
		{
			SetLastErrorEx(ERROR_INVALID_WINDOW_HANDLE, SLE_ERROR);
		}
	}
	else
	{
		SetLastErrorEx(ERROR_INVALID_DATA, SLE_ERROR);
	}

	return FALSE;
}
