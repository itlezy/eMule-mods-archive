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


 
#include "stdafx.h"
#include "MagicDLP.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMagicDLP::CMagicDLP(CString appdir_in)
{
	appdir=appdir_in;

	Magicdlpavailable=false;
	MagicdlpInstance=NULL;
	Reload();
}

CMagicDLP::~CMagicDLP()
{
	if(MagicdlpInstance!=NULL)
	{
		::FreeLibrary(MagicdlpInstance);
	}
}

void CMagicDLP::Reload()
{
	Magicdlpavailable=false;
	bool waserror=false;

	CString newdll=appdir + _T("MagicAntiLeech.dll.new");
	CString olddll=appdir + _T("MagicAntiLeech.dll.old");
	CString currentdll=appdir + _T("MagicAntiLeech.dll");


	if(PathFileExists(newdll))
	{
		AddLogLine(false,_T("found new version of MagicAntiLeech.dll"));
		//new version exists, try to unload the old and load the new one
		if(MagicdlpInstance!=NULL)
		{
			::FreeLibrary(MagicdlpInstance);
			MagicdlpInstance=NULL;
		}
		if(PathFileExists(currentdll))
		{
			if(PathFileExists(olddll))
			{
				if(_tremove(olddll)!=0)
					waserror=true;
			}
			if(waserror==false)
				if(_trename(currentdll,olddll)!=0)
					waserror=true;
		}
		if(waserror==false)
		{
			if(_trename(newdll,currentdll)!=0)
				waserror=true;
		}
		if(waserror)
			AddLogLine(false,_T("error during copying the MagicAntiLeech.dll's, try to load the old one"));
	}

	if(MagicdlpInstance==NULL)
	{
		MagicdlpInstance=::LoadLibrary(currentdll);
		if(MagicdlpInstance!=NULL)
		{
			//testfunc = (TESTFUNC)GetProcAddress(dlpInstance,("TestFunc"));
			GetMagicDLPVersion = (GETMAGICDLPVERSION)GetProcAddress(MagicdlpInstance,("GetMagicDLPVersion"));

			DLPCheckModstring_GPL = (DLPCHECKMODSTRING_GPL)GetProcAddress(MagicdlpInstance,("DLPCheckModstring_GPL"));
			DLPCheckUsername_GPL = (DLPCHECKUSERNAME_GPL)GetProcAddress(MagicdlpInstance,("DLPCheckUsername_GPL"));

			DLPCheckModstring_Hard = (DLPCHECKMODSTRING_HARD)GetProcAddress(MagicdlpInstance,("DLPCheckModstring_Hard"));
			DLPCheckModstring_Soft = (DLPCHECKMODSTRING_SOFT)GetProcAddress(MagicdlpInstance,("DLPCheckModstring_Soft"));

			DLPCheckUsername_Hard = (DLPCHECKUSERNAME_HARD)GetProcAddress(MagicdlpInstance,("DLPCheckUsername_Hard"));
			DLPCheckUsername_Soft = (DLPCHECKUSERNAME_SOFT)GetProcAddress(MagicdlpInstance,("DLPCheckUsername_Soft"));

			DLPCheckModstring_Bad = (DLPCHECKMODSTRING_BAD)GetProcAddress(MagicdlpInstance,("DLPCheckModstring_Bad"));
			if( GetMagicDLPVersion &&
				DLPCheckModstring_GPL &&
				DLPCheckUsername_GPL &&
				DLPCheckModstring_Hard &&
				DLPCheckModstring_Soft &&
				DLPCheckUsername_Hard &&
				DLPCheckUsername_Soft &&
				DLPCheckModstring_Bad
				)
			{
				Magicdlpavailable=true;
				AddLogLine(false,_T("Magic Dynamic Anti-Leecher Protection v%u loaded"), GetMagicDLPVersion());
			}
			else
			{
				LogError(_T("failed to initialize the MagicAntiLeech.dll"));
				::FreeLibrary(MagicdlpInstance);
				MagicdlpInstance=NULL;
			}
		}
		else
		{
			LogError(_T("failed to load the MagicAntiLeech.dll"));
			LogError(_T("ErrorCode: %u"), GetLastError());
		}
	}
	else
	{
		AddDebugLogLine(false,_T("no new version of MagicAntiLeech.dll found. kept the old one"));
		Magicdlpavailable=true;
	}
}
