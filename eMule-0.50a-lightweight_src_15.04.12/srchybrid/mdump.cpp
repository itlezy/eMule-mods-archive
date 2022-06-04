//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "stdafx.h"
#include <dbghelp.h>
#include "mdump.h"

#if _MSC_VER >= 1400 && !defined(STATUS_INVALID_CRUNTIME_PARAMETER)
#define STATUS_INVALID_CRUNTIME_PARAMETER ((DWORD)0xC0000417L)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

//CMiniDumper theCrashDumper;	//Enig123:: Avi3k: improve code
TCHAR CMiniDumper::m_szAppName[MAX_PATH] = {0};
TCHAR CMiniDumper::m_szDumpDir[MAX_PATH] = {0};

void CMiniDumper::Enable(LPCTSTR pszAppName, bool bShowErrors, LPCTSTR pszDumpDir)
{
	// if this assert fires then you have two instances of CMiniDumper which is not allowed
	ASSERT( m_szAppName[0] == _T('\0') );
	_tcsncpy(m_szAppName, pszAppName, _countof(m_szAppName) - 1);
	m_szAppName[_countof(m_szAppName) - 1] = _T('\0');

	// eMule may not have the permission to create a DMP file in the directory where the "emule.exe" is located.
	// Need to pre-determine a valid directory.
	_tcsncpy(m_szDumpDir, pszDumpDir, _countof(m_szDumpDir) - 1);
	m_szDumpDir[_countof(m_szDumpDir) - 1] = _T('\0');
	PathAddBackslash(m_szDumpDir);

	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = NULL;
	HMODULE hDbgHelpDll = GetDebugHelperDll((FARPROC*)&pfnMiniDumpWriteDump, bShowErrors);
	if (hDbgHelpDll)
	{
		if (pfnMiniDumpWriteDump){
			SetUnhandledExceptionFilter(TopLevelFilter);
#if _MSC_VER >= 1400
			_set_invalid_parameter_handler(InvalidParameterHandler);
#endif
		}
		FreeLibrary(hDbgHelpDll);
		hDbgHelpDll = NULL;
		pfnMiniDumpWriteDump = NULL;
	}
}

#ifndef _WIN64
#define PLATFORM _T("x86")
#else
#define PLATFORM _T("x64")
#endif

#define DBGHELP_HINT _T("You can get the required DBGHELP.DLL by downloading the \"User Mode Process Dumper\" from \"Microsoft Download Center\".\r\n\r\n") \
	_T("Extract the \"User Mode Process Dumper\" and locate the \"") PLATFORM _T("\" folder. Copy the DBGHELP.DLL from the \"") PLATFORM _T("\" folder into your eMule installation folder and/or into your Windows system32/SysWOW64 folder.")

HMODULE CMiniDumper::GetDebugHelperDll(FARPROC* ppfnMiniDumpWriteDump, bool bShowErrors)
{
	*ppfnMiniDumpWriteDump = NULL;
	HMODULE hDll = LoadLibrary(_T("DBGHELP.DLL"));
	if (hDll == NULL)
	{
		if (bShowErrors) {
			// Do *NOT* localize that string (in fact, do not use MFC to load it)!
			MessageBox(NULL, _T("DBGHELP.DLL not found. Please install a DBGHELP.DLL.\r\n\r\n") DBGHELP_HINT, m_szAppName, MB_ICONSTOP | MB_OK);
		}
	}
	else
	{
		*ppfnMiniDumpWriteDump = GetProcAddress(hDll, "MiniDumpWriteDump");
		if (*ppfnMiniDumpWriteDump == NULL)
		{
			if (bShowErrors) {
				// Do *NOT* localize that string (in fact, do not use MFC to load it)!
				MessageBox(NULL, _T("DBGHELP.DLL found is too old. Please upgrade to version 5.1 (or later) of DBGHELP.DLL.\r\n\r\n") DBGHELP_HINT, m_szAppName, MB_ICONSTOP | MB_OK);
			}
		}
	}
	return hDll;
}

LONG CMiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	LONG lRetValue = EXCEPTION_CONTINUE_SEARCH;
	TCHAR szResult[MAX_PATH + 1024] = {0};
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = NULL;
	HMODULE hDll = GetDebugHelperDll((FARPROC*)&pfnMiniDumpWriteDump, true);
	if (hDll)
	{
		if (pfnMiniDumpWriteDump)
		{
			// Ask user if they want to save a dump file
			// Do *NOT* localize that string (in fact, do not use MFC to load it)!
			if (MessageBox(NULL, _T("eMule crashed :-(\r\n\r\nA diagnostic file can be created which will help the author to resolve this problem. This file will be saved on your Disk (and not sent).\r\n\r\nDo you want to create this file now?"), m_szAppName, MB_ICONSTOP | MB_YESNO) == IDYES)
			{
				// Create full path for DUMP file
				TCHAR szDumpPath[MAX_PATH];
				_tcsncpy(szDumpPath, m_szDumpDir, _countof(szDumpPath) - 1);
				szDumpPath[_countof(szDumpPath) - 1] = _T('\0');
				size_t uDumpPathLen = _tcslen(szDumpPath);

				TCHAR szBaseName[MAX_PATH];
				_tcsncpy(szBaseName, m_szAppName, _countof(szBaseName) - 1);
				szBaseName[_countof(szBaseName) - 1] = _T('\0');
				size_t uBaseNameLen = _tcslen(szBaseName);

				time_t tNow = time(NULL);
				_tcsftime(szBaseName + uBaseNameLen, _countof(szBaseName) - uBaseNameLen, _T("_%Y%m%d-%H%M%S"), localtime(&tNow));
				szBaseName[_countof(szBaseName) - 1] = _T('\0');

				// Replace spaces and dots in file name.
				LPTSTR psz = szBaseName;
				while (*psz != _T('\0')) {
					if (*psz == _T('.'))
						*psz = _T('-');
					else if (*psz == _T(' '))
						*psz = _T('_');
					psz++;
				}
				if (uDumpPathLen < _countof(szDumpPath) - 1) {
					_tcsncat(szDumpPath, szBaseName, _countof(szDumpPath) - uDumpPathLen - 1);
					szDumpPath[_countof(szDumpPath) - 1] = _T('\0');
					uDumpPathLen = _tcslen(szDumpPath);
					if (uDumpPathLen < _countof(szDumpPath) - 1) {
						_tcsncat(szDumpPath, _T(".dmp"), _countof(szDumpPath) - uDumpPathLen - 1);
						szDumpPath[_countof(szDumpPath) - 1] = _T('\0');
					}
				}

				HANDLE hFile = CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo = {0};
					ExInfo.ThreadId = GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					BOOL bOK = (*pfnMiniDumpWriteDump)(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
					if (bOK)
					{
						// Do *NOT* localize that string (in fact, do not use MFC to load it)!
						_sntprintf(szResult, _countof(szResult) - 1, _T("Saved dump file to \"%s\".\r\n\r\nPlease send this file together with a detailed bug report to the developer!\r\n\r\nThank you for helping to improve eMule."), szDumpPath); 
						lRetValue = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						// Do *NOT* localize that string (in fact, do not use MFC to load it)!
						_sntprintf(szResult, _countof(szResult) - 1, _T("Failed to save dump file to \"%s\".\r\n\r\nError: %u"), szDumpPath, GetLastError());
					}
					CloseHandle(hFile);
				}
				else
				{
					// Do *NOT* localize that string (in fact, do not use MFC to load it)!
					_sntprintf(szResult, _countof(szResult) - 1, _T("Failed to create dump file \"%s\".\r\n\r\nError: %u"), szDumpPath, GetLastError());
				}
				szResult[_countof(szResult) - 1] = _T('\0');
			}
		}
		FreeLibrary(hDll);
		hDll = NULL;
		pfnMiniDumpWriteDump = NULL;
	}

	if (szResult[0] != _T('\0'))
		MessageBox(NULL, szResult, m_szAppName, MB_ICONINFORMATION | MB_OK);

#ifndef _DEBUG
	// Exit the process only in release builds, so that in debug builds the exception is passed to a possible
	// installed debugger
	ExitProcess(0);
#else
	return lRetValue;
#endif
}

#if _MSC_VER >= 1400
#ifdef __cplusplus
extern "C"{
#endif
void * _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)
void * _AddressOfReturnAddress(void);
#pragma intrinsic(_AddressOfReturnAddress)
#if defined(_WIN64) && !defined(HAVE_WIN7_SDK_H)
PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
	IN ULONG64 ControlPc,
	OUT PULONG64 ImageBase,
	IN OUT PVOID HistoryTable OPTIONAL
	);

PVOID
RtlVirtualUnwind (
	IN ULONG HandlerType,
	IN ULONG64 ImageBase,
	IN ULONG64 ControlPc,
	IN PRUNTIME_FUNCTION FunctionEntry,
	IN OUT PCONTEXT ContextRecord,
	OUT PVOID *HandlerData,
	OUT PULONG64 EstablisherFrame,
	IN OUT PVOID ContextPointers OPTIONAL
	);

#endif
#define UNW_FLAG_NHANDLER 0x00
#ifdef __cplusplus
}
#endif

void CMiniDumper::InvalidParameterHandler(LPCTSTR pszExpression, LPCTSTR pszFunction, LPCTSTR pszFile, unsigned int nLine, uintptr_t pReserved){
	_EXCEPTION_RECORD ExceptionRecord;
	memset(&ExceptionRecord, 0, sizeof(ExceptionRecord));
	_CONTEXT Context;
	_EXCEPTION_POINTERS ExceptionInfo = {&ExceptionRecord, &Context};

#ifndef _WIN64
	__asm {
		mov dword ptr [Context.Eax], eax
		mov dword ptr [Context.Ecx], ecx
		mov dword ptr [Context.Edx], edx
		mov dword ptr [Context.Ebx], ebx
		mov dword ptr [Context.Esi], esi
		mov dword ptr [Context.Edi], edi
		mov word ptr [Context.SegSs], ss
		mov word ptr [Context.SegCs], cs
		mov word ptr [Context.SegDs], ds
		mov word ptr [Context.SegEs], es
		mov word ptr [Context.SegFs], fs
		mov word ptr [Context.SegGs], gs
		pushfd
		pop [Context.EFlags]
	}

	Context.ContextFlags = CONTEXT_CONTROL;
#pragma warning(disable:4311)
	Context.Eip = (ULONG)_ReturnAddress();
	Context.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(default:4311)
	Context.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);
#else
	ULONG64 ControlPc;
	ULONG64 EstablisherFrame;
	PRUNTIME_FUNCTION FunctionEntry;
	PVOID HandlerData;
	ULONG64 ImageBase;

	RtlCaptureContext(&Context);
	ControlPc = Context.Rip;
	FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, NULL);
	if (FunctionEntry != NULL) {
		RtlVirtualUnwind(UNW_FLAG_NHANDLER, ImageBase, ControlPc, FunctionEntry, &Context, &HandlerData, &EstablisherFrame, NULL);
	} else {
		Context.Rip = (ULONGLONG) _ReturnAddress();
		Context.Rsp = (ULONGLONG) _AddressOfReturnAddress();
	}
#endif

    ExceptionRecord.ExceptionCode = STATUS_INVALID_CRUNTIME_PARAMETER;
    ExceptionRecord.ExceptionFlags    = EXCEPTION_NONCONTINUABLE;
    ExceptionRecord.ExceptionAddress = _ReturnAddress();

#ifdef DEBUG
	CString msg;
	msg.Format(L"Invalid parameter detected in function %s."
				L" File: %s Line: %d\nExpression: %s\n", pszFunction, pszFile, nLine, pszExpression);
	AfxMessageBox(msg);
#endif

    UnhandledExceptionFilter(&ExceptionInfo);

    TerminateProcess(GetCurrentProcess(), STATUS_INVALID_CRUNTIME_PARAMETER);
}
#endif
