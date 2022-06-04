//this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
//Copyright (C)2002-2007 Xtreme-Mod (emulextreme@yahoo.de)

//emule Xtreme is a modification of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
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

//
//
//	Author: Xman 
//  





#include "stdafx.h"
#include "DLP.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDLP::CDLP(CString appdir_in)
{
	appdir=appdir_in;

	dlpavailable=false;
	dlpInstance=NULL;
	Reload();
}

CDLP::~CDLP()
{
	if(dlpInstance!=NULL)
	{
		::FreeLibrary(dlpInstance);
	}
}

void CDLP::Reload()
{
	dlpavailable=false;
	bool waserror=false;

	CString newdll=appdir + _T("antiLeech.dll.new");
	CString olddll=appdir + _T("antiLeech.dll.old");
	CString currentdll=appdir + _T("antiLeech.dll");


	if(PathFileExists(newdll))
	{
		AddLogLine(false,_T("found new version of antiLeech.dll"));
		//new version exists, try to unload the old and load the new one
		if(dlpInstance!=NULL)
		{
			::FreeLibrary(dlpInstance);
			dlpInstance=NULL;
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
			AddLogLine(false,_T("error during copying the antiLeech.dll's, try to load the old one"));
	}

	if(dlpInstance==NULL)
	{
		dlpInstance=::LoadLibrary(currentdll);
		if(dlpInstance!=NULL)
		{
			//testfunc = (TESTFUNC)GetProcAddress(dlpInstance,("TestFunc"));
			GetDLPVersion = (GETDLPVERSION)GetProcAddress(dlpInstance,("GetDLPVersion"));
			DLPCheckModstring_Hard = (DLPCHECKMODSTRING_HARD)GetProcAddress(dlpInstance,("DLPCheckModstring_Hard"));
			DLPCheckModstring_Soft = (DLPCHECKMODSTRING_SOFT)GetProcAddress(dlpInstance,("DLPCheckModstring_Soft"));

			DLPCheckUsername_Hard = (DLPCHECKUSERNAME_HARD)GetProcAddress(dlpInstance,("DLPCheckUsername_Hard"));
			DLPCheckUsername_Soft = (DLPCHECKUSERNAME_SOFT)GetProcAddress(dlpInstance,("DLPCheckUsername_Soft"));

			DLPCheckNameAndHashAndMod = (DLPCHECKNAMEANDHASHANDMOD)GetProcAddress(dlpInstance,("DLPCheckNameAndHashAndMod"));

			DLPCheckMessageSpam = (DLPCHECKMESSAGESPAM)GetProcAddress(dlpInstance,("DLPCheckMessageSpam"));
			DLPCheckUserhash = (DLPCHECKUSERHASH)GetProcAddress(dlpInstance,("DLPCheckUserhash"));

			DLPCheckHelloTag = (DLPCHECKHELLOTAG)GetProcAddress(dlpInstance,("DLPCheckHelloTag"));
			DLPCheckInfoTag = (DLPCHECKINFOTAG)GetProcAddress(dlpInstance,("DLPCheckInfoTag"));
			if( GetDLPVersion &&
				DLPCheckModstring_Hard &&
				DLPCheckModstring_Soft &&
				DLPCheckUsername_Hard &&
				DLPCheckUsername_Soft &&
				DLPCheckNameAndHashAndMod &&
				DLPCheckHelloTag &&
				DLPCheckInfoTag &&
				DLPCheckMessageSpam &&
				DLPCheckUserhash
				)
			{
				dlpavailable=true;
				AddLogLine(false,_T("Dynamic Anti-Leecher Protection v %u loaded"), GetDLPVersion());
			}
			else
			{
				LogError(_T("failed to initialize the antiLeech.dll, please use an up tp date version of DLP"));
				::FreeLibrary(dlpInstance);
				dlpInstance=NULL;
			}
		}
		else
		{
			LogError(_T("failed to load the antiLeech.dll"));
			LogError(_T("ErrorCode: %u"), GetLastError());
		}
	}
	else
	{
		AddDebugLogLine(false,_T("no new version of antiLeech.dll found. kept the old one"));
		dlpavailable=true;
	}
}
