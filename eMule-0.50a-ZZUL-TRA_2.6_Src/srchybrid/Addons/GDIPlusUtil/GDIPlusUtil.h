#pragma once
#include <GdiPlus.h>
using namespace Gdiplus;

#define _ARGB(a, rgb)   (((a)<<24) | (rgb))

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
