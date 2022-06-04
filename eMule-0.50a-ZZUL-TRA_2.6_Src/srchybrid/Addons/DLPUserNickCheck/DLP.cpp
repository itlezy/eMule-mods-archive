
#include "stdafx.h"
#include "DLP.h"
#include "Log.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDLP::CDLP(CString configdir_in)
{
	configdir=configdir_in;

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

	CString currentdll=configdir + _T("antiLeech.dll");

	if(dlpInstance==NULL)
	{
		dlpInstance=::LoadLibrary(currentdll);
		if(dlpInstance!=NULL)
		{
			GetDLPVersion = (GETDLPVERSION)GetProcAddress(dlpInstance,("GetDLPVersion"));
			DLPCheckUsername_Hard = (DLPCHECKUSERNAME_HARD)GetProcAddress(dlpInstance,("DLPCheckUsername_Hard"));
			DLPCheckUsername_Soft = (DLPCHECKUSERNAME_SOFT)GetProcAddress(dlpInstance,("DLPCheckUsername_Soft"));

			if( GetDLPVersion &&
				DLPCheckUsername_Hard &&
				DLPCheckUsername_Soft
				)
			{
				dlpavailable=true;
				AddLogLine(false,_T("Own Username Check loaded - DLP v%u"), GetDLPVersion());
			}
		}
	}
}
