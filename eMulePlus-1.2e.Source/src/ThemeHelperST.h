//
//	Class:		CThemeHelperST
//
//	Compiler:	Visual C++
//	Tested on:	Visual C++ 6.0
//
//	Version:	See GetVersionC() or GetVersionI()
//
//	Created:	09/January/2002
//	Updated:	31/October/2002
//
//	Author:		Davide Calabro'		davide_calabro@yahoo.com
//									http://www.softechsoftware.it
//
//	Note:		Based on the CVisualStylesXP code 
//				published by David Yuheng Zhao (yuheng_zhao@yahoo.com)
//
//	Disclaimer
//	----------
//	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//	RISK OF USING THIS SOFTWARE.
//
//	Terms of use
//	------------
//	THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//	IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//	ARE GENTLY ASKED TO DONATE 5$ (FIVE U.S. DOLLARS) TO THE AUTHOR:
//
//		Davide Calabro'
//		P.O. Box 65
//		21019 Somma Lombardo (VA)
//		Italy
//
#pragma once

#include <uxtheme.h>
#include <tmschema.h>

class CThemeHelperST
{
public:
	CThemeHelperST();
	virtual ~CThemeHelperST();

	HTHEME	OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT	CloseThemeData(HTHEME hTheme);
	HRESULT	DrawThemeBackground(HTHEME hTheme, HWND hWnd, HDC hdc, int iPartId, int iStateId,
		const RECT *pRect, const RECT *pClipRect);
	HRESULT	DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
		LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
	BOOL	IsThemeActive();
	BOOL	IsAppThemed();
	HRESULT	GetCurrentThemeName(LPWSTR pszThemeFileName, int cchMaxNameChars, 
		LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars);
	HRESULT	GetThemeTextExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
		LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, LPCRECT pBoundingRect, LPRECT pExtentRect);

private:
	HMODULE	m_hDll;

	typedef HTHEME (WINAPI *PFNOPENTHEMEDATA)(HWND, LPCWSTR);
	static HTHEME WINAPI OpenThemeDataFail(HWND, LPCWSTR) { return NULL; }

	typedef HRESULT (WINAPI *PFNCLOSETHEMEDATA)(HTHEME);
	static HRESULT WINAPI CloseThemeDataFail(HTHEME) { return E_FAIL; }

	typedef HRESULT (WINAPI *PFNDRAWTHEMEBACKGROUND)(HTHEME, HDC, int, int, const RECT*,  const RECT*);
	static HRESULT WINAPI DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT*, const RECT*) { return E_FAIL; }

	typedef HRESULT (WINAPI *PFNDRAWTHEMETEXT)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*);
	static HRESULT WINAPI DrawThemeTextFail(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*) { return E_FAIL; }

	typedef BOOL (WINAPI *PFNISTHEMEACTIVE)();
	static BOOL WINAPI IsThemeActiveFail() { return FALSE; }

	typedef BOOL (WINAPI *PFNISAPPTHEMED)();
	static BOOL WINAPI IsAppThemedFail() { return FALSE; }

	typedef HRESULT (WINAPI *PFNGETCURRENTTHEMENAME)(LPWSTR, int, LPWSTR, int, LPWSTR, int);
	static HRESULT WINAPI GetCurrentThemeNameFail(LPWSTR, int, LPWSTR, int, LPWSTR, int) { return E_FAIL; }

	typedef HRESULT (WINAPI *PFNDRAWTHEMEPARENTBACKGROUND)(HWND, HDC, RECT*);
	static HRESULT WINAPI DrawThemeParentBackgroundFail(HWND, HDC, RECT*) { return E_FAIL; }

	typedef HRESULT (WINAPI *PFNGETTHEMETEXTEXTENT)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPCRECT, LPRECT);
	static HRESULT WINAPI GetThemeTextExtentFail(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPCRECT, LPRECT) { return E_FAIL; }

	PFNOPENTHEMEDATA				m_pfnOpenThemeData;
	PFNCLOSETHEMEDATA				m_pfnCloseThemeData;
	PFNDRAWTHEMEBACKGROUND			m_pfnDrawThemeBackground;
	PFNDRAWTHEMETEXT				m_pfnDrawThemeText;
	PFNISTHEMEACTIVE				m_pfnIsThemeActive;
	PFNISAPPTHEMED					m_pfnIsAppThemed;
	PFNGETCURRENTTHEMENAME			m_pfnGetCurrentThemeName;
	PFNDRAWTHEMEPARENTBACKGROUND	m_pfnDrawThemeParentBackground;
	PFNGETTHEMETEXTEXTENT			m_pfnGetThemeTextExtent;
};
