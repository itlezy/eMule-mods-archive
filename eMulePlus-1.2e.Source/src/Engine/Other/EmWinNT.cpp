// EmWinNt.cpp: implementation of the CEmWinNt class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

#include "EmWinNT.h"

void CEmDynModule::Uninit()
{
	if (m_hDll)
	{
		if (!FreeLibrary(m_hDll))
			AddDebugLogLine("FreeLibrary failed");
		m_hDll = NULL;
	}
}

CEmWinNT CEmWinNT::s_stWinNT;

CEmWinNT::CEmWinNT()
{
	// first of all determine if we are running on NT system
	OSVERSIONINFO stVersionInfo;
	stVersionInfo.dwOSVersionInfoSize = sizeof(stVersionInfo);

	if(GetVersionEx(&stVersionInfo) && (stVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))
		if (m_hDll = LoadLibrary(_T("kernel32.dll")))
			// Obtain now pointers to desired functions
			if (
				!(m_pfnCreateIoCompletionPort = (HANDLE (WINAPI *)(HANDLE, HANDLE, DWORD, DWORD)) GetProcAddress(m_hDll, "CreateIoCompletionPort")) ||
				!(m_pfnGetQueuedCompletionStatus = (BOOL (WINAPI *)(HANDLE, PDWORD, PDWORD, OVERLAPPED**, DWORD)) GetProcAddress(m_hDll, "GetQueuedCompletionStatus")) ||
				!(m_pfnPostQueuedCompletionStatus = (BOOL (WINAPI *)(HANDLE, DWORD, DWORD, OVERLAPPED*)) GetProcAddress(m_hDll, "PostQueuedCompletionStatus")) ||
				!(m_pfnTryEnterCriticalSection = (BOOL (WINAPI *)(CRITICAL_SECTION*)) GetProcAddress(m_hDll, "TryEnterCriticalSection")))
				Uninit(); // one or more exports are missing
}

CEmMswSock::CEmMswSock()
{
	// first of all determine if we are running on NT system
	OSVERSIONINFO stVersionInfo;
	stVersionInfo.dwOSVersionInfoSize = sizeof(stVersionInfo);

	if(GetVersionEx(&stVersionInfo) && (stVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))
		if (m_hDll = LoadLibrary(_T("mswsock.dll")))
			// Obtain now pointers to desired functions
			if (
				!(m_pfnAcceptEx = (BOOL (WINAPI *)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, DWORD*, OVERLAPPED*)) GetProcAddress(m_hDll, "CreateIoCompletionPort")) ||
				!(m_pfnGetAcceptExSockaddrs = (void (WINAPI *)(PVOID, DWORD, DWORD, DWORD, SOCKADDR**, int*, SOCKADDR**, int*)) GetProcAddress(m_hDll, "GetQueuedCompletionStatus")))
				Uninit(); // one or more exports are missing
}
