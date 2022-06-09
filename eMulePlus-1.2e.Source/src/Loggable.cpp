//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "emule.h"
#else
	#define GetResString(id)  _T("NOT IMPLEMENTED YET")
#endif //NEW_SOCKETS_ENGINE
#include "Loggable.h"

void CLoggable::AddLogLine(int iMode, UINT nID, ...)
{
	va_list argptr;
	va_start(argptr, nID);
	AddLogText(iMode, GetResString(nID), argptr);
	va_end(argptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoggable::AddLogLine(int iMode, LPCTSTR line, ...)
{
	ASSERT(line != NULL);

	va_list argptr;
	va_start(argptr, line);
	AddLogText(iMode, line, argptr);
	va_end(argptr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoggable::AddLogText(int iMode, LPCTSTR line, va_list argptr)
{
	ASSERT(line != NULL);

	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	if (((iMode & LOG_FL_DBG) != 0) && !g_App.m_pPrefs->GetVerbose())
		return;
#endif

	CString sBuffer;

	sBuffer.FormatV(line, argptr);

#ifndef NEW_SOCKETS_ENGINE
	#ifdef _DEBUG
		//SyruS (0.29c) view the log messages while shutting down at least in the debugger
		if (g_App.m_app_state != g_App.APP_STATE_RUNNING)
			TRACE("App Log: %s\n", sBuffer);
	#endif

	if (g_App.m_pMDlg)
		g_App.m_pMDlg->AddLogText(iMode, sBuffer);
#else
	g_stEngine.AddLog((iMode & LOG_FL_DBG) ? LOG_DEBUG : LOG_WARNING, sBuffer);
#endif //NEW_SOCKETS_ENGINE

#ifdef _DEBUG
	if ((iMode & LOG_FL_DBG) != 0)
		::OutputDebugString(sBuffer + _T("\n"));
#endif

	EMULE_CATCH2
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
