#include "stdafx.h"
#include "GDIPlusUtil.h"
#include "OtherFunctions.h"
#include "Preferences.h"

CGDIPlus g_gdiplus;

ARGB ConvertToARGB(COLORREF color){
	return _rotr(_byteswap_ulong(color), 8);
}

CGDIPlus::CGDIPlus():gdiplusToken(0){
}

CGDIPlus::~CGDIPlus(){
	if(gdiplusToken)
		GdiplusShutdown(gdiplusToken);
}

void CGDIPlus::Init(){
	if(thePrefs.GetWindowsVersion() == _WINVER_2K_){
		// NOTE: Do *NOT* forget to specify /DELAYLOAD:gdiplus.dll as link parameter.
		HMODULE hLib = LoadLibrary(_T("gdiplus.dll"));
		if (hLib == NULL)
			return;
		bool bGdiPlusInstalled = GetProcAddress(hLib, "GdiplusStartup") != NULL;
		FreeLibrary(hLib);
		if(!bGdiPlusInstalled)
			return;
	}
	GdiplusStartupInput gdiplusStartupInput;
	// Initialize GDI+.
	VERIFY(GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Ok);
}