#pragma once
#include <GdiPlus.h>
using namespace Gdiplus;

#define _ARGB(a, r, g, b)   (((DWORD)(BYTE)(a)<<ALPHA_SHIFT) | (((DWORD)(BYTE)(r))<<RED_SHIFT) | ((DWORD)((BYTE)(g))<<GREEN_SHIFT) | ((DWORD)(BYTE)(b)<<BLUE_SHIFT))

ARGB ConvertToARGB(COLORREF color);

class CGDIPlus{
public:
	CGDIPlus();
	~CGDIPlus();
	void Init();
	bool IsInited() const{ return gdiplusToken != 0; }
private:
	ULONG_PTR           gdiplusToken;
};
extern CGDIPlus g_gdiplus;
