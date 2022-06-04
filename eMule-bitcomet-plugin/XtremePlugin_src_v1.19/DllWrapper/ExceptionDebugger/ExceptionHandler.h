// ExceptionHandler.h  Version 1.1
//
// Copyright ?1998 Bruce Dawson
//
// Author:       Bruce Dawson
//               brucedawson@cygnus-software.com
//
// Modified by:  Hans Dietrich
//               hdietrich2@hotmail.com
//
// A paper by the original author can be found at:
//     http://www.cygnus-software.com/papers/release_debugging.html
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WINDOWS_

	// We forward declare PEXCEPTION_POINTERS so that the function
	// prototype doesn't needlessly require windows.h.
	typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
	int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, LPCTSTR Message);
	extern LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
	extern void __cdecl CustomInvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
	extern void set_program_name(LPTSTR name);

	extern LPCTSTR g_sz_program_id;
	extern LPCTSTR g_sz_program_name;
extern LPTSTR g_sz_program_status;
extern LPTSTR g_sz_program_mode;
extern LPTSTR g_sz_socker_init_version;
extern LPTSTR g_sz_web_frame;
#define       g_debug_info_buffer_size 1024
	extern TCHAR  g_debug_info_buffer[];
	extern TCHAR  g_crash_info_buffer[];
	extern char   g_http_debug_info_buffer[];
extern UINT   g_uint_msg_id;
extern UINT   g_uint_cache_size;
extern UINT   g_uint_mem_block_size;

#endif // _WINDOWS_

/*
// Sample usage - put the code that used to be in main into HandledMain.
// To hook it in to an MFC app add ExceptionAttacher.cpp from the mfctest
// application into your project.
int main(int argc, char *argv[])
{
	int Result = -1;
	__try
	{
		Result = HandledMain(argc, argv);
	}
	__except(RecordExceptionInfo(GetExceptionInformation(), "main thread"))
	{
		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}
	return Result;
}
*/
