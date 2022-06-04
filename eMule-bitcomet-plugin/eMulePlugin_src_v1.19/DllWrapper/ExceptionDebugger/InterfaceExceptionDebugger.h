#pragma once

// ´íÎó±¨¸æÄ£¿é
// e.g.
// boost::function<BOOL(void)> func = boost::bind(&CSimpleBTApp::InitInstanceInternal,this);
// return Core_Common::InterfaceExceptionDebugger::call_with_exception_debugger( &func, FALSE );

// load dbghelp.dll dynamically
//#pragma comment(lib, "Dbghelp.lib")	

#ifdef _WINDOWS_

#include <string>
#ifdef  _UNICODE
	typedef std::wstring		tstring;
#else
	typedef std::string			tstring;
#endif


namespace Core_Common {

class InterfaceExceptionDebugger
{
public:
	static void set_program_id(LPCTSTR id);
	static void set_program_name(LPCTSTR name);
	static void set_program_status(LPTSTR text);
	static void set_program_mode(LPTSTR text);
	static void set_socker_init_version(LPTSTR text);
	static void set_web_frame(LPTSTR text);
	static void set_cache_size(UINT cache_size);
	static void set_bt_mem_block_size(UINT mem_block_size);
	static void set_msg_id(UINT msg_id);
	static void set_debug_info(tstring info);
	static void set_crash_info(tstring info);
	//static void set_http_debug_info(string info);

	static void set_unhandled_exception_filter();
	//static void call_with_exception_debugger(const boost::function0<void>* pfunction);
	//template<class _ReturnType>
	//static _ReturnType call_with_exception_debugger(const boost::function<_ReturnType(void)>* pfunction, _ReturnType return_default);

	typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
	static int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, LPCTSTR Message);
private:
	static void __cdecl CustomInvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
	static LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
};

//template<class _ReturnType>
//_ReturnType InterfaceExceptionDebugger::call_with_exception_debugger(const boost::function<_ReturnType(void)>* pfunction, _ReturnType return_default)
//{
//	__try
//	{
//		return (*pfunction)();
//	}
//	__except ( Core_Common::InterfaceExceptionDebugger::RecordExceptionInfo(GetExceptionInformation(), _T("") ) )
//	{
//		// Do nothing here - RecordExceptionInfo() has already done
//		// everything that is needed. Actually this code won't even
//		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
//		// the __except clause.
//		::TerminateProcess(GetCurrentProcess(), 100);
//		return return_default;
//	}
//}

} // namespace_end

#endif // _WINDOWS_
