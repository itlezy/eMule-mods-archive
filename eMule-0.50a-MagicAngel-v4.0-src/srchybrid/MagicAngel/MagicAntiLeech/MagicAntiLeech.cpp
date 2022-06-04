//DLP = Dynamic Leecher Protection
//this code is part of Xtreme-Mod
//author: Xman
// changed for MagicDLP by sFrQlXeRt

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.




#include <atlstr.h>
#include "MagicAntiLeech.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BOOL WINAPI  DllMain (
					  HANDLE    hModule,
					  DWORD     dwFunction,
					  LPVOID    lpNot)
{
	switch (dwFunction)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}


void __declspec(dllexport)  TestFunc()
{
	::MessageBox(NULL,_T("Inside the DLL!"),_T("Nix"),0);
}

// ==> GPL Breaker Detection - sFrQlXeRt
LPCTSTR __declspec(dllexport) DLPCheckModstring_GPL(LPCTSTR modversion, LPCTSTR clientversion)
{
	if(modversion==NULL && clientversion==NULL)
		return NULL;

	if (
		// ==> Modstrings from NeoMule leechers.dat file - sFrQlXeRt
		StrStrI(modversion, _T("l!onet"))||
		StrStrI(modversion, _T("l!0net"))||
		StrStrI(modversion, _T("lionet"))||
		StrStrI(modversion, _T("li0net"))||
		StrStrI(modversion, _T("li()net"))||
		StrStrI(modversion, _T("L!()Net"))||
		StrStrI(modversion, _T("hardmule"))||
		StrStrI(modversion, _T("FreeAngel"))||
		StrStrI(modversion, _T("koikoi"))||
		StrStrI(modversion, _T("LHEMULE"))||
		StrStrI(modversion, _T("BIONIC"))||
		StrStrI(modversion, _T("I%27M WITH STUPID"))||
		StrStrI(modversion, _T("¦n¤ÍÁp·ù"))||
		StrStrI(modversion, _T("°­Å[³¡¶¤"))||
		StrStrI(modversion, _T("°­Å[Áp·ù"))||
		StrStrI(modversion, _T("Power Mule"))||
		StrStrI(modversion, _T("Kalitsch"))||
		StrStrI(modversion, _T("LSD.13b.XYJ.o3f"))||
		// <== Modstrings from NeoMule leechers.dat file - sFrQlXeRt
		// ==> more GPL Breaker - sFrQlXeRt
		StrStrI(modversion, _T("Plus Plus"))||
		StrStrI(modversion, _T("Warezfaw.com 2.0"))||
		StrStrI(modversion, _T("darkmule"))||
		StrStrI(modversion, _T("00de"))||
		StrStrI(modversion, _T("OOde"))||
		StrStrI(modversion, _T("00.de"))||
		StrStrI(modversion, _T("OO.de"))||
		StrStrI(modversion, _T("Keta"))||
		StrStrI(modversion, _T("rappi"))||
		StrStrI(modversion, _T("ZX v4."))||
		StrStrI(modversion, _T("ZX v5."))||
		StrStrI(modversion, _T("FXMule"))||
		StrStrI(modversion, _T("FXeMule"))||
		StrStrI(modversion, _T("????"))||
		StrStrI(modversion, _T("Zambor"))||
		StrStrI(modversion, _T("rabbit"))||
		StrStrI(modversion, _T("rabb_it"))||
		StrStrI(modversion, _T("Speedmule"))||
		StrStrI(modversion, _T("Administrator"))||
		StrStrI(modversion, _T("DarkDragon"))||
		StrStrI(modversion, _T("Exorzist"))||
		StrStrI(modversion, _T("x0Rz!$T"))||
		StrStrI(modversion, _T("€xORz!§T"))||
		StrStrI(modversion, _T("No Ratio"))||
		StrStrI(modversion, _T("DeathAngel"))
		// <== more GPL Breaker - sFrQlXeRt
		)
		return _T("Bad MODSTRING");

	return NULL;
}
LPCTSTR __declspec(dllexport) DLPCheckUsername_GPL(LPCTSTR username)
{
	if(username==NULL)
		return NULL;

	if (
		// ==> Usernames from NeoMule leechers.dat file - sFrQlXeRt
		StrStrI(username, _T("[lh.2y.net]"))||
		StrStrI(username, _T("[lh 2.2]"))||
		StrStrI(username, _T("[lionetwork]"))||
		StrStrI(username, _T("[lionheart"))||
		StrStrI(username, _T("li@network"))||
		StrStrI(username, _T("l!onetwork"))||
		StrStrI(username, _T("li()net"))||
		StrStrI(username, _T("l!0net"))||
		StrStrI(username, _T("$gam3r$"))||
		StrStrI(username, _T("g@m3r"))||
		StrStrI(username, _T("HARDMULE"))||
		StrStrI(username, _T("BIONIC"))||
		// <== Usernames from NeoMule leechers.dat file - sFrQlXeRt
		// ==> more GPL Breaker - sFrQlXeRt
		StrStrI(username, _T("emule-client"))||
		StrStrI(username, _T("FXeMule"))||
		StrStrI(username, _T("FXMule"))||
		StrStrI(username, _T("Emule FX"))||
		StrStrI(username, _T("OO.de"))||
		StrStrI(username, _T("00de"))||
		StrStrI(username, _T("OOde"))||
		StrStrI(username, _T("DarkMule"))||
		StrStrI(username, _T("Rappi"))||
		StrStrI(username, _T("Ketamine"))||
		StrStrI(username, _T("Boomerang"))||
		StrStrI(username, _T("HARDMULE"))||
		StrStrI(username, _T("rabb_it"))||
		StrStrI(username, _T("ZamBoR"))||
		StrStrI(username, _T("Disrael.Net"))||
		StrStrI(username, _T("UnKnOwN pOiSoN"))||
		StrStrI(username, _T("www.pruna.com"))||
		StrStrI(username, _T("[KOR"))||
		StrStrI(username,_T("emule-mods.biz"))
		// <== more GPL Breaker - sFrQlXeRt
		)
		return _T("Bad USERNAME");

	return NULL;
}// <== GPL Breaker Detection - sFrQlXeRt

// ==> Bad Mod Detection - sFrQlXeRt
LPCTSTR __declspec(dllexport) DLPCheckModstring_Bad(LPCTSTR modversion, LPCTSTR clientversion)
{
	if(modversion==NULL || clientversion==NULL)
		return NULL;

	if (
		// ==> Bad Mods - sFrQlXeRt
		StrStrI(modversion, _T("Neo Mule v2"))||
		StrStrI(modversion, _T("Neo Mule v3"))||
		StrStrI(modversion, _T("eChanblardNext"))||
		StrStrI(modversion, _T("eWombat"))
		// <== Bad Mods - sFrQlXeRt
		)
		return _T("Bad MODSTRING");

	return NULL;
}

// <== Bad Mod Detection - sFrQlXeRt

LPCTSTR __declspec(dllexport) DLPCheckModstring_Hard(LPCTSTR modversion, LPCTSTR clientversion)
{
	if(modversion==NULL || clientversion==NULL)
		return NULL;

	if (
		// ==> more Hard Leechers - sFrQlXeRt
		StrStrI(modversion, _T("warezfaw.com 3.0"))||
		StrStrI(modversion, _T("A I D E A D S L"))||
		StrStrI(modversion, _T("The Killer Bean"))
		// <== more Hard Leechers - sFrQlXeRt
		)
		return _T("Bad MODSTRING");

	return NULL;
}

LPCTSTR __declspec(dllexport) DLPCheckModstring_Soft(LPCTSTR modversion, LPCTSTR clientversion)
{
	if(modversion==NULL || clientversion==NULL)
		return NULL;

	if (
		// ==> more Soft Leechers - sFrQlXeRt
		StrStrI(modversion, _T("lovelace.10e X"))||
		StrStrI(modversion, _T("EastShare")) && StrStrI(clientversion, _T("0.29"))||
		StrStrI(modversion, _T("Xmas MoD"))||
		StrStrI(modversion, _T("NewWebCache"))
		// <== more Soft Leechers - sFrQlXeRt
		)
		return _T("Bad MODSTRING");


	return NULL;
}
// ==> We might need this later - sFrQlXeRt
LPCTSTR __declspec(dllexport) DLPCheckUsername_Hard(LPCTSTR username)
{
	if(username==NULL)
		return NULL;

	return NULL;
}
// <== We might need this later - sFrQlXeRt

LPCTSTR __declspec(dllexport) DLPCheckUsername_Soft(LPCTSTR username)
{
	if(username==NULL)
		return NULL;

	if(
		// ==> more Soft Leechers - sFrQlXeRt
		StrStrI(username, _T("[LSD.17"))||
		StrStrI(username, _T("[LSD.18"))||
		StrStrI(username, _T("[LSD.20"))
		// <== more Soft Leechers - sFrQlXeRt
		)
		return _T("Bad USERNAME");
	
	return NULL;
}

