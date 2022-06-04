#include "stdafx.h"

#ifdef _WINDOWS_

#include "InterfaceExceptionDebugger.h"
#include "ExceptionHandler.h"

namespace Core_Common {

//void InterfaceExceptionDebugger::call_with_exception_debugger(const boost::function0<void>* pfunction)
//{
//	__try
//	{
//		(*pfunction)();
//	}
//	__except ( Core_Common::InterfaceExceptionDebugger::RecordExceptionInfo(GetExceptionInformation(), _T("") ) )
//	{
//		// Do nothing here - RecordExceptionInfo() has already done
//		// everything that is needed. Actually this code won't even
//		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
//		// the __except clause.
//		::TerminateProcess(GetCurrentProcess(), 100);
//	}
//}

LONG WINAPI InterfaceExceptionDebugger::CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
// Should be set in both of CMyApp::Run() and CMyApp::InitInstance()
// Only triggered if not caught by (__try, __except) block and no debugger present
{
#ifdef _DEBUG
	MessageBox(NULL, _T("Unhandled Exception!"), NULL, MB_OK);
#endif

	InterfaceExceptionDebugger::RecordExceptionInfo(pExInfo, _T("CustomUnhandledExceptionFilter()") );
	::TerminateProcess(GetCurrentProcess(), 100);
	return EXCEPTION_EXECUTE_HANDLER;
}

void __cdecl InterfaceExceptionDebugger::CustomInvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
{
	if ( ::IsDebuggerPresent() )
	{
		ASSERT(false);
	}
	throw NULL;
}

int __cdecl InterfaceExceptionDebugger::RecordExceptionInfo(PEXCEPTION_POINTERS data, LPCTSTR Message)
{
	if ( ::IsDebuggerPresent() )
	{
		ASSERT(false);
	}
#ifdef _WIN32_WCE
	return ::TerminateProcess(GetCurrentProcess(), 100);
#else
	return ::RecordExceptionInfo(data, Message);
#endif // _WIN32_WCE
}

void InterfaceExceptionDebugger::set_program_id(LPCTSTR id)
{
	g_sz_program_id = id;
}

void InterfaceExceptionDebugger::set_program_name(LPCTSTR name)
{
	g_sz_program_name = name;
}

void InterfaceExceptionDebugger::set_program_status(LPTSTR text)
{
	g_sz_program_status = text;
}

void InterfaceExceptionDebugger::set_program_mode(LPTSTR text)
{
	g_sz_program_mode = text;
}

void InterfaceExceptionDebugger::set_socker_init_version(LPTSTR text)
{
	g_sz_socker_init_version = text;
}

void InterfaceExceptionDebugger::set_web_frame(LPTSTR text)
{
	g_sz_web_frame = text;
}

void InterfaceExceptionDebugger::set_msg_id(UINT msg_id)
{
	g_uint_msg_id = msg_id;
}

void InterfaceExceptionDebugger::set_cache_size(UINT cache_size)
{
	g_uint_cache_size = cache_size;
}

void InterfaceExceptionDebugger::set_bt_mem_block_size(UINT mem_block_size)
{
	g_uint_mem_block_size = mem_block_size;
}

void InterfaceExceptionDebugger::set_debug_info(tstring info)
{
	if ( info.size() > g_debug_info_buffer_size )
		info.resize(g_debug_info_buffer_size);
	_tcscpy(g_debug_info_buffer, info.data() );
}

void InterfaceExceptionDebugger::set_crash_info(tstring info)
{
	if ( info.size() > g_debug_info_buffer_size )
		info.resize(g_debug_info_buffer_size);
	_tcscpy(g_crash_info_buffer, info.data() );
}

//void InterfaceExceptionDebugger::set_http_debug_info(string info)
//{
//	if ( info.size() > g_debug_info_buffer_size )
//		info.resize(g_debug_info_buffer_size);
//	strcpy(g_http_debug_info_buffer, info.data() );
//}

void InterfaceExceptionDebugger::set_unhandled_exception_filter()
// Should be called in both of CMyApp::Run() and CMyApp::InitInstance()
// Only triggered if not caught by (__try, __except) block and no debugger present
{
#ifndef _WIN32_WCE
	::SetUnhandledExceptionFilter(InterfaceExceptionDebugger::CustomUnhandledExceptionFilter);
#endif // _WIN32_WCE

#ifndef _WIN32_WCE
	::_set_invalid_parameter_handler(InterfaceExceptionDebugger::CustomInvalidParameterHandler);
#endif // _WIN32_WCE
}


} // namespace_end

#endif // _WINDOWS_
