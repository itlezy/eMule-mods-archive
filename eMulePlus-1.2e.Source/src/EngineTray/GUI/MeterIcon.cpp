// MeterIcon.cpp														12.12.03
//------------------------------------------------------------------------------
#include "stdafx.h"

#define _MAXBLOCKS 5

BOOL DrawIconMeter(HDC hDC, int nMax, int nCur, int cx, int cy, 
					COLORREF crMeter, COLORREF crBorder);

HICON CreateMeterIcon(UINT nBaseIconID, 
				 int cx, int cy, 
				 int iMaxVal, int iCurVal,
				 COLORREF crMeter, COLORREF crBorder)
{
	EMULE_TRY

	ICONINFO iiNewIcon;
	::ZeroMemory(&iiNewIcon,sizeof(ICONINFO));
	iiNewIcon.fIcon = TRUE;

	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), 
									 MAKEINTRESOURCE(nBaseIconID), 
									 IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
	if(hIcon == NULL)
		return NULL;

	HWND hDesktopWnd = ::GetDesktopWindow();
	if(hDesktopWnd == NULL)
		return hIcon;
	HDC hDesktopDC = ::GetDC(hDesktopWnd);
	if(hDesktopDC == NULL)
		return hIcon;

	HDC hIconDC = ::CreateCompatibleDC(hDesktopDC);
	iiNewIcon.hbmColor = ::CreateCompatibleBitmap(hDesktopDC, cx, cy);
	HBITMAP hOldIconBMP = (HBITMAP)::SelectObject(hIconDC, iiNewIcon.hbmColor);

	::PatBlt(hIconDC, 0, 0, cx, cy, BLACKNESS);
	::DrawIconEx(hIconDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);
	DrawIconMeter(hIconDC, iMaxVal, iCurVal, cx, cy, crMeter, crBorder);
	
	iiNewIcon.hbmMask = ::CreateCompatibleBitmap(hDesktopDC, cx, cy);
	::SelectObject(hIconDC, iiNewIcon.hbmMask);
			
	::PatBlt(hIconDC, 0, 0, cx, cy, WHITENESS);
	::DrawIconEx(hIconDC, 0, 0, hIcon, cx, cy, 0, NULL, DI_NORMAL);
	DrawIconMeter(hIconDC, iMaxVal, iCurVal, cx, cy, 0, 0);

	::SelectObject(hIconDC, hOldIconBMP);
	::DestroyIcon(hIcon);

	hIcon = CreateIconIndirect(&iiNewIcon);

	//if(hIcon == NULL)
	//{
	//	LPVOID lpMsgBuf;
	//	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	//				  FORMAT_MESSAGE_FROM_SYSTEM	 |
	//				  FORMAT_MESSAGE_IGNORE_INSERTS,
	//				  NULL, GetLastError(),
	//				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	//				  (LPTSTR)&lpMsgBuf, 0, NULL);
	//	MessageBox(NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK|MB_ICONINFORMATION);
	//	LocalFree(lpMsgBuf);
	//}

	::DeleteObject(iiNewIcon.hbmColor);
	::DeleteObject(iiNewIcon.hbmMask);
	::DeleteDC(hIconDC);
	::ReleaseDC(hDesktopWnd, hDesktopDC);

	return hIcon;

	EMULE_CATCH
	return NULL;
}

BOOL DrawIconMeter(HDC hDC, int nMax, int nCur, int cx, int cy, 
					COLORREF crMeter, COLORREF crBorder)
{
	EMULE_TRY

	HPEN	hPen   = NULL, hOldPen   = NULL;
	HBRUSH	hBrush = NULL, hOldBrush = NULL;

	hPen = ::CreatePen(PS_SOLID,1,crBorder);
	hOldPen = (HPEN)::SelectObject(hDC, hPen);

	hBrush = ::CreateSolidBrush(crMeter);
	hOldBrush = (HBRUSH)::SelectObject(hDC, hBrush);
	
	if(nCur > 0)
	{
		int iYpos = cy - 4;
		int nBlockCnt = nCur / ((nMax == 0 ? 1 : nMax) / _MAXBLOCKS) + 1;
		if(nBlockCnt > _MAXBLOCKS)
			nBlockCnt = _MAXBLOCKS;
		for(int i = 0; i < nBlockCnt; i++)
		{
			::Rectangle(hDC, 0, iYpos, 4, iYpos+4);
			iYpos -= 3;
		}
	}
	
	if(hOldPen)
		::SelectObject(hDC,hOldPen);
	if(hOldBrush)
		::SelectObject(hDC, hOldBrush);
	if(hPen)
		::DeleteObject(hPen);
	if(hBrush)
		::DeleteObject(hBrush);

	return TRUE;
	
	EMULE_CATCH
	return FALSE;
}

