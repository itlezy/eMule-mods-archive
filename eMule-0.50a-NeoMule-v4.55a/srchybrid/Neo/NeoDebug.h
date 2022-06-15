//this file is part of NeoMule
//Copyright (C)2006 David Xanatos ( Xanatos@Lycos.at / http://neomule.sourceforge.net )
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

#pragma once

// NEO: ND - [NeoDebug] -- Xanatos -->
#ifdef _DEBUG_NEO
CString GetErrorMessage(CException* e);
CString InterpreteErrorCode(unsigned int code);
CString InterpreteErrorInfo(struct _EXCEPTION_POINTERS *ep);

#define NEO_TRY try{

#define NEO_CATCH(extra) \
	}catch(CString &str){\
		try{	if(thePrefs.GetVerbose()) DebugLogError(LOG_STATUSBAR, _T("DEBUG-NEO: Exception in function: %s, file %s, line %ld; %s, ERROR: %s. Function Aborted!"),_T(__FUNCTION__),_T(__FILE__), __LINE__, extra,str);									}catch(...){} \
	}catch(CException *err){ \
		try{	if(thePrefs.GetVerbose()) DebugLogError(LOG_STATUSBAR, _T("DEBUG-NEO: Exception in function: %s, file %s, line %ld; %s, ERROR: %s. Function Aborted!"),_T(__FUNCTION__),_T(__FILE__), __LINE__, extra,GetErrorMessage(err)); err->Delete();	}catch(...){} \
	}catch(...){ \
		try{	if(thePrefs.GetVerbose()) DebugLogError(LOG_STATUSBAR, _T("DEBUG-NEO: Exception in function: %s, file %s, line %ld; %s, ERROR: unknown. Function Aborted!"),_T(__FUNCTION__),_T(__FILE__), __LINE__, extra);									}catch(...){} \
	}

#define NEO_CATCH_EX(extra) \
	}catch(CString &str){\
		try{ \
			CString Info; \
			Info.Format(_T("Exception in function: %s, file %s, line %ld; %s, ERROR: %s. Function Aborted!"),_T(__FUNCTION__),_T(__FILE__), __LINE__,extra,str); \
			if(thePrefs.GetVerbose()) \
				DebugLogError(LOG_STATUSBAR, _T("DEBUG-NEO: %s"), Info); \
			AfxMessageBox(Info,MB_OK | MB_ICONSTOP,NULL); \
		}catch(...){} \
	}catch(CException *err){ \
		try{ \
			CString Info; \
			Info.Format(_T("Exception in function: %s, file %s, line %ld; %s, ERROR: %s. Function Aborted!"),_T(__FUNCTION__),_T(__FILE__), __LINE__,extra, GetErrorMessage(err)); err->Delete(); \
			if(thePrefs.GetVerbose()) \
				DebugLogError(LOG_STATUSBAR, _T("DEBUG-NEO: %s"), Info); \
			AfxMessageBox(Info,MB_OK | MB_ICONSTOP,NULL); \
		}catch(...){} \
	}catch(...){ \
		try{ \
			CString Info; \
			Info.Format(_T("Exception in function: %s, file %s, line %ld; %s, ERROR: unknown. Function Aborted!"),_T(__FUNCTION__),_T(__FILE__), __LINE__,extra); \
			if(thePrefs.GetVerbose()) \
				DebugLogError(LOG_STATUSBAR, _T("DEBUG-NEO: %s"), Info); \
			AfxMessageBox(Info,MB_OK | MB_ICONSTOP,NULL); \
		}catch(...){} \
	}

#define NEO_CATCH_NULL }catch(...){}

#define NEO_DEBUG_BREAK(extra) \
	if (GetAsyncKeyState(VK_ESCAPE) < 0 && GetAsyncKeyState(VK_SHIFT) < 0){ \
		CString Info; \
		Info.Format(_T("Manual Neo Debug Break, in %s"),extra); \
		throw Info; \
	} \


#define NEO_DEBUG_ESC_BREAK if (GetAsyncKeyState(VK_ESCAPE) < 0 && GetAsyncKeyState(VK_SHIFT) < 0) RaiseException(0x00000000,NULL,NULL,NULL);

#else

#define NEO_TRY
#define NEO_CATCH(extra)
#define NEO_CATCH_EX(extra)
#define NEO_CATCH_NULL
#define NEO_DEBUG_BREAK(extra)
#define NEO_DEBUG_ESC_BREAK

#endif //_DEBUG_NEO
// NEO_ ND END - [NeoDebug] <-- Xanatos --