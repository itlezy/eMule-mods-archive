#define MAGICDLPVERSION 5


#pragma once

class CantiLeech 
{
public:
	BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD,LPVOID);
	DWORD	__declspec(dllexport) GetMagicDLPVersion(){return MAGICDLPVERSION;}

	LPCTSTR __declspec(dllexport) DLPCheckModstring_GPL(LPCTSTR modversion, LPCTSTR clientversion);
	LPCTSTR __declspec(dllexport) DLPCheckUsername_GPL(LPCTSTR username);

	LPCTSTR __declspec(dllexport) DLPCheckModstring_Bad(LPCTSTR modversion, LPCTSTR clientversion); // => Bad Mod Detection - sFrQlXeRt

	LPCTSTR __declspec(dllexport) DLPCheckModstring_Hard(LPCTSTR modversion, LPCTSTR clientversion);
	LPCTSTR __declspec(dllexport) DLPCheckModstring_Soft(LPCTSTR modversion, LPCTSTR clientversion);
	LPCTSTR __declspec(dllexport) DLPCheckUsername_Hard(LPCTSTR username);
	LPCTSTR __declspec(dllexport) DLPCheckUsername_Soft(LPCTSTR username);

	void __declspec(dllexport)  TestFunc();	
};

//<<< new tags from eMule 0.04x
#define CT_UNKNOWNx3E			0x3E // => Snake - sFrQlXeRt
//>>> eWombat [SNAFU_V3]

