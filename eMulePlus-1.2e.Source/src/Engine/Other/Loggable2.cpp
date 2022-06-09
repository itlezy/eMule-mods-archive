// Server.cpp: implementation of the server classes
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loggable2.h"
#ifdef SUPPORT_CLIENT_PEER
	#include "../Database/TaskProcessorDB.h"
#endif //SUPPORT_CLIENT_PEER


void CLoggable2::AddLog(EnumLogType eType, LPCTSTR szLine, ...)
{
	va_list argptr;
	va_start(argptr, szLine);
	if(szLine)
	{
		// Make string
		CString sBuffer;
		sBuffer.FormatV(szLine, argptr);

		CPreciseTime tmNow = CPreciseTime::GetCurrentTime();

		// Output
		if(eType >= LOG_ERROR)
			TRACE(_T("***** "));
		TRACE(CString(tmNow.FullTime) + _T("  ") + sBuffer + _T("\n"));

		if(eType >= LOG_ERROR && g_stEngine.AlertOnErrors())
			ASSERT(FALSE);

#ifdef SUPPORT_CLIENT_PEER
		CTask_AddToLog *pTask = new CTask_AddToLog(eType, tmNow, sBuffer);
		g_stEngine.DB.Push(pTask);
#endif //SUPPORT_CLIENT_PEER
	}
	va_end(argptr);
}
