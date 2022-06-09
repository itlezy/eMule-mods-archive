#include "stdafx.h"
#include "ThemeHelperST.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CThemeHelperST::CThemeHelperST()
{
//	Load DLL. This call will fails (and return NULL)
//	if we are not running under Windows XP (or greater)
	m_hDll = ::LoadLibrary(_T("UxTheme.dll"));

	typedef	void (*ThemeCBProc)(void);
	static const struct
	{
		ThemeCBProc	*pOut;
		LPCSTR		szFnNm;
		ThemeCBProc	pIn;
	} s_stProcAddress[] =
	{
		{(ThemeCBProc*)&m_pfnOpenThemeData, "OpenThemeData", (ThemeCBProc)OpenThemeDataFail},
		{(ThemeCBProc*)&m_pfnCloseThemeData, "CloseThemeData", (ThemeCBProc)CloseThemeDataFail},
		{(ThemeCBProc*)&m_pfnDrawThemeBackground, "DrawThemeBackground", (ThemeCBProc)DrawThemeBackgroundFail},
		{(ThemeCBProc*)&m_pfnDrawThemeText, "DrawThemeText", (ThemeCBProc)DrawThemeTextFail},
		{(ThemeCBProc*)&m_pfnIsThemeActive, "IsThemeActive", (ThemeCBProc)IsThemeActiveFail},
		{(ThemeCBProc*)&m_pfnIsAppThemed, "IsAppThemed", (ThemeCBProc)IsAppThemedFail},
		{(ThemeCBProc*)&m_pfnGetCurrentThemeName, "GetCurrentThemeName", (ThemeCBProc)GetCurrentThemeNameFail},
		{(ThemeCBProc*)&m_pfnDrawThemeParentBackground, "DrawThemeParentBackground", (ThemeCBProc)DrawThemeParentBackgroundFail},
		{(ThemeCBProc*)&m_pfnGetThemeTextExtent, "GetThemeTextExtent", (ThemeCBProc)GetThemeTextExtentFail}
	};

	for (unsigned ui = 0; ui < ARRSIZE(s_stProcAddress); ui++)
		*(s_stProcAddress[ui].pOut) = (m_hDll != NULL) ?
			reinterpret_cast<ThemeCBProc>(GetProcAddress(m_hDll, s_stProcAddress[ui].szFnNm)) : s_stProcAddress[ui].pIn;
}

CThemeHelperST::~CThemeHelperST()
{
//	Unload DLL (if any)
	if (m_hDll != NULL)
		::FreeLibrary(m_hDll);
	m_hDll = NULL;
}

HTHEME CThemeHelperST::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	return (*m_pfnOpenThemeData)(hwnd, pszClassList);
}

HRESULT CThemeHelperST::CloseThemeData(HTHEME hTheme)
{
	return (*m_pfnCloseThemeData)(hTheme);
}

HRESULT CThemeHelperST::GetCurrentThemeName(LPWSTR pszThemeFileName, int cchMaxNameChars, 
	LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars)
{
	return (*m_pfnGetCurrentThemeName)(pszThemeFileName, cchMaxNameChars, pszColorBuff, cchMaxColorChars, pszSizeBuff, cchMaxSizeChars);
}

HRESULT CThemeHelperST::DrawThemeBackground(HTHEME hTheme, HWND hWnd, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect)
{
	if (hWnd != NULL)
		(*m_pfnDrawThemeParentBackground)(hWnd, hdc, const_cast<RECT*>(pRect));

	return (*m_pfnDrawThemeBackground)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

HRESULT CThemeHelperST::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect)
{
	return (*m_pfnDrawThemeText)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
}

BOOL CThemeHelperST::IsAppThemed()
{
	return (*m_pfnIsAppThemed)();
}

BOOL CThemeHelperST::IsThemeActive()
{
	return (*m_pfnIsThemeActive)();
}

HRESULT CThemeHelperST::GetThemeTextExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
		LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, LPCRECT pBoundingRect, LPRECT pExtentRect)
{
	return (*m_pfnGetThemeTextExtent)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, pBoundingRect, pExtentRect);
}
