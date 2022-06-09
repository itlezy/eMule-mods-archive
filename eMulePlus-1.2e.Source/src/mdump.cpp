#include "stdafx.h"
#include <dbghelp.h>
#include "mdump.h"
#include "otherfunctions.h"
#include "emule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

LPCTSTR	MiniDumper::m_szKeepDumpFile = NULL;
LPCTSTR	MiniDumper::m_szCantUseDBGHelp = NULL;
LPCTSTR	MiniDumper::m_szDownloadDBGHelp = NULL;
LPCTSTR	MiniDumper::m_szSavedDump = NULL;
LPCTSTR	MiniDumper::m_szFailedToSave = NULL;
bool	MiniDumper::m_bWritten;

MiniDumper::MiniDumper()
{
	m_bWritten = false;

	::SetUnhandledExceptionFilter(TopLevelFilter);
}

void MiniDumper::LoadStrings()
{
//	NB! _tcsdup can return NULL in case of memory lack
	m_szKeepDumpFile = _tcsdup(GetResString(IDS_DUMP_KEEPDUMPFILE));
	m_szCantUseDBGHelp = _tcsdup(GetResString(IDS_DUMP_CANTUSEDBGHELP));
	m_szDownloadDBGHelp = _tcsdup(GetResString(IDS_DUMP_DOWNLOADDBG));
	m_szSavedDump = _tcsdup(GetResString(IDS_DUMP_SAVEDDUMP));
	m_szFailedToSave = _tcsdup(GetResString(IDS_DUMP_FAILEDTOSAVE));
}

MiniDumper::~MiniDumper()
{
	if (m_szKeepDumpFile != NULL)
		free((void*)m_szKeepDumpFile);
	if (m_szCantUseDBGHelp != NULL)
		free((void*)m_szCantUseDBGHelp);
	if (m_szDownloadDBGHelp != NULL)
		free((void*)m_szDownloadDBGHelp);
	if (m_szSavedDump != NULL)
		free((void*)m_szSavedDump);
	if (m_szFailedToSave != NULL)
		free((void*)m_szFailedToSave);
}

void CloseAfterTimeout(void*)
{
	Sleep(60 * 1000);	// wait minute and terminate
	TerminateProcess(GetCurrentProcess(), 0);
}

LONG MiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	g_App.m_app_state = g_App.APP_STATE_SHUTTINGDOWN;

	// Start timeout thread
	_beginthread(CloseAfterTimeout, 0, NULL);

	// If already written, wait for process to terminate
	if(m_bWritten)
	{
		Sleep(60 * 1000);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	TCHAR szDbgHelpPath[_MAX_PATH], *pcTmpPtr;
	unsigned	uiSz = (sizeof(szDbgHelpPath) - sizeof(_T("DBGHELP.DLL"))) / sizeof(TCHAR);

	szDbgHelpPath[uiSz] = _T('\0');	//if name is longer than buffer, terminating null won't be added
	if (GetModuleFileName(NULL, szDbgHelpPath, uiSz))
	{
		pcTmpPtr = _tcsrchr(szDbgHelpPath, _T('\\'));
		if (pcTmpPtr != NULL)
		{
			_tcscpy(pcTmpPtr + 1, _T("DBGHELP.DLL"));
			hDll = ::LoadLibrary(szDbgHelpPath);
		}
	}

	if (hDll == NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
	}

	LPCTSTR szResult = NULL;

	if (hDll != NULL)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			TCHAR		szDumpPath[_MAX_PATH], szScratch[0x500];

			// work out a good place for the dump file
			uiSz = (sizeof(szDumpPath) - sizeof(CLIENT_NAME_WITH_VER _T(".dmp"))) / sizeof(TCHAR);
			szDumpPath[uiSz] = _T('\0');	//if name is longer than buffer, terminating null won't be added
			if ((GetModuleFileName(0, szDumpPath, uiSz) == 0) || ((pcTmpPtr = _tcsrchr(szDumpPath, _T('\\'))) == NULL))
				pcTmpPtr = szDumpPath - 1;

			_tcscpy(pcTmpPtr + 1, CLIENT_NAME_WITH_VER _T(".dmp"));

			// save dump
			// create the file
			HANDLE hFile = CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL, NULL );

			int nStatus = 0, iGetLastErr = 0;
			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = FALSE;

				// write the dump
				BOOL bOK = (pDump)(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
				if (bOK)
					nStatus = 1;
				else
				{
					nStatus = 2;
					iGetLastErr = GetLastError();
				}
				CloseHandle(hFile);
			}
			else
			{
				nStatus = 3;
				iGetLastErr = GetLastError();
			}

			m_bWritten = true;
		// Ask a user if he wants to keep a dump file
			if ( ::MessageBox( NULL, (m_szKeepDumpFile != NULL) ? m_szKeepDumpFile :
				CLIENT_NAME _T(" crashed.\nA diagnostic file has been created which will help the authors to resolve this problem.\nDo you want to keep this file?"),
				CLIENT_NAME_WITH_VER, MB_YESNO | MB_ICONSTOP ) == IDYES )
			{
				switch(nStatus)
				{
				case 1:
					_stprintf( szScratch, (m_szSavedDump != NULL) ? m_szSavedDump :
						_T("Saved dump file to '%s'\n\nPlease upload this file at http://emuleplus.info/upload/ (recommended)\nor send it by e-mail at eMulePlus@yahoo.com.sg\nThank you for helping us improving ") CLIENT_NAME,
						szDumpPath );
					szResult = szScratch;
					break;
				case 2:
				case 3:
					_stprintf( szScratch, (m_szFailedToSave != NULL) ? m_szFailedToSave :
						_T("Failed to save dump file '%s' (error %d)"), szDumpPath, iGetLastErr );
					szResult = szScratch;
					break;
				}
			}
			else
				DeleteFile(szDumpPath);
		}
		else
			szResult = (m_szCantUseDBGHelp != NULL) ? m_szCantUseDBGHelp :
				_T("Could not use DBGHELP.DLL located on your system.\nPlease download the latest version (see http://emuleplus.info/)\nand place it in ") CLIENT_NAME _T(" directory.");
	}
	else
		szResult = (m_szDownloadDBGHelp != NULL) ? m_szDownloadDBGHelp :
			CLIENT_NAME _T(" crashed.\n\nTo help us find this bug, please download DBGHELP.DLL\nfrom microsoft's site and install it in your system.\n\nMore details available on http://emuleplus.info/");

	if (szResult)
		MessageBox(NULL, szResult, CLIENT_NAME_WITH_VER, MB_OK | MB_ICONSTOP);
	ExitProcess(0);
}
