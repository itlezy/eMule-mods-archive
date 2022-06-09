// TitleMenu.h: interface for the CTitleMenu class.
// Based on the code of Per Fikse(1999/06/16) on codeguru.earthweb.com
// Author: Arthur Westerman
// Bug reports by : Brian Pearson 
//////////////////////////////////////////////////////////////////////
#pragma once
#include "map_inc.h"
/*
#if (WINVER < 0x0500)
typedef struct tagMENUINFO
{
    DWORD   cbSize;
    DWORD   fMask;
    DWORD   dwStyle;
    UINT    cyMax;
    HBRUSH  hbrBack;
    DWORD   dwContextHelpID;
    ULONG_PTR dwMenuData;
}   MENUINFO, FAR *LPMENUINFO;
typedef MENUINFO CONST FAR *LPCMENUINFO;
#endif

typedef BOOL (WINAPI* TSetMenuInfo)(
  HMENU hmenu,       // handle to menu
  LPCMENUINFO lpcmi  // menu information
);
typedef BOOL (WINAPI* TGetMenuInfo)(
  HMENU hmenu,            // handle to menu
  LPCMENUINFO lpcmi       // menu information
);
*/
typedef UINT (WINAPI* LPFNGRADIENTFILL)(HDC, CONST PTRIVERTEX, DWORD, CONST PVOID, DWORD, DWORD); // Avi3k: fix & improve code

class CTitleMenu : public CMenu
{
public:
	CTitleMenu();
	virtual ~CTitleMenu();

	BOOL CreateMenu();
	BOOL DestroyMenu();
	void DeleteIcons();

	void EnableIcons();
	void AddMenuTitle(LPCTSTR lpszTitle, bool bIsIconMenu = false);
	BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL, LPCTSTR lpszIconName = NULL);
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL, LPCTSTR lpszIconName = NULL);

	long GetColor() { return m_clLeft; }
	void SetColor(long cl) { m_clLeft = cl; }

	long GetGradientColor() { return m_clRight; }
	void SetGradientColor(long cl) { m_clRight = cl; }

	long GetTextColor() { return m_clText; }
	void SetTextColor(long cl) { m_clText = cl; }

	long GetEdge() { return m_uEdgeFlags; }
	void SetEdge(bool shown, UINT remove = 0, UINT add = 0)	{ m_bDrawEdge = shown; (m_uEdgeFlags ^= remove) |= add; }

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	CString m_strTitle;
	long m_clRight;
	long m_clLeft;
	long m_clText;
	bool m_bDrawEdge;
	bool m_bIconMenu;
	UINT m_uEdgeFlags;
	CImageList m_ImageList;
#ifdef REPLACE_ATLMAP
	unordered_map<UINT_PTR, int> m_mapMenuIdToIconIdx;
#else
	CAtlMap<UINT_PTR, int> m_mapMenuIdToIconIdx;
#endif
	CStringToIntMap m_mapIconNameToIconIdx;
	CStringToHBITMAPMap m_mapIconNameToBitmap;

	// Avi3k: fix & improve code
	//typedef UINT (WINAPI* LPFNGRADIENTFILL)(HDC, CONST PTRIVERTEX, DWORD, CONST PVOID, DWORD, DWORD);
	static LPFNGRADIENTFILL m_pfnGradientFill;
	//HINSTANCE m_hLibMsimg32;
	// end Avi3k: fix & improve code

/*	static bool m_bInitializedAPI;
	static bool LoadAPI();
	static void FreeAPI();
	static TSetMenuInfo SetMenuInfo;
	static TGetMenuInfo GetMenuInfo;*/
	void DrawMonoIcon(int nIconPos, CPoint nDrawPos, CDC *dc);
	void SetMenuBitmap(UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, LPCTSTR lpszIconName);
};
