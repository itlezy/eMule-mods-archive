#pragma once

struct _EXCEPTION_POINTERS;

class CMiniDumper
{
public:
	static void Enable(LPCTSTR pszAppName, bool bShowErrors, LPCTSTR pszDumpDir);

private:
	static TCHAR m_szAppName[MAX_PATH];
	static TCHAR m_szDumpDir[MAX_PATH];

	static HMODULE GetDebugHelperDll(FARPROC* ppfnMiniDumpWriteDump, bool bShowErrors);
	static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo);
#if _MSC_VER >= 1400
	static void InvalidParameterHandler(LPCTSTR pszExpression, LPCTSTR pszFunction, LPCTSTR pszFile, unsigned int nLine, uintptr_t pReserved);
#endif
};

//extern CMiniDumper theCrashDumper; //Enig123:: Avi3k: improve code
