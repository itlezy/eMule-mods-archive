/************************************************************************/
/*      Copyright (C) Alexey Kazakovsky 2002.  All rights reserved.     */
/*      Written by Alexey Kazakovsky (alex@lastbit.com)                 */
/*                                                                      */
/*      Free software: no warranty; use anywhere is ok; spread the      */
/*      sources; note any modifications; share variations and           */
/*      derivatives (including sending to alex@lastbit.com).            */
/*                                                                      */
/*                                                                      */
/*      This class CSysInfo makes easy to get a various system info:    */
/*      - OS version (e.g. Windows 2000 Terminal Server)                */
/*      - CPU type                                                      */
/*      - Total amount of physical memory and memory usage              */
/*      - IE version                                                    */
/*      - ADO version                                                   */
/*      - Does the current user have administrator rights ?             */
/*      - Network interface (NIC) list with their IP addresses          */
/*      - etc...                                                        */
/*                                                                      */
/*      Last Modified:           December, 19  2000                     */
/*      Unicode supported:       yes                                    */
/*      Target:                  Microsoft Visual C++ 6.0/7.0           */
/*                                                                      */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
//Updated:	28.02.2007 by WiZaRd
//
//- many codeparts rewritten and optimized, including some memleak fixes
//- removed obsolete definitions (included in VS defines)

#include "stdafx.h"
#include "SysInfo.h"
#include <process.h>
#include "emule.h"
#include "Log.h" //>>> taz::VistaFix [pP]

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Version.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CSYSINFO_VERSION "2.0"

#define W95_s L"Windows 95"
#define W98_s L"Windows 98"
#define WME_s L"Windows ME"
#define NT4_s L"Windows NT 4.0"
#define WKS_s L" Workstation"
#define WXP_s L"Windows XP"
#define XHE_s L" Home Edition"
#define XPR_s L" Professional Edition"
#define WVI_s L"Windows Vista"
#define W2K_s L"Windows 2000"
#define PRO_s L" Professional"
#define DOM_s L" Domain Controller"
#define SER_s L" Server"
#define ADV_s L" Advanced"
#define DTC_s L" DataCenter"
#define TER_s L" Terminal"
#define BOF_s L" BackOffice"

#define IE_CONFIGURATION_KEY	L"SOFTWARE\\Microsoft\\Internet Explorer"
#define CPU_CONFIGURATION_KEY	L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
#define ADO_CONFIGURATION_KEY	L"SOFTWARE\\Microsoft\\DataAccess"

#define MAX_IPADDRESSES 100

//>>> taz::VistaFix [pP]
CString GetRegKeyVista(const CString& sKey, const bool bLog = false)
{
	// get the WinVer from registry
	HKEY hKey = NULL;
#define VISTA_KEY	L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, VISTA_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwType;
		TCHAR szValue[100];
		DWORD dwSize = sizeof(szValue)*sizeof(TCHAR);
		if (RegQueryValueEx(hKey, sKey, NULL, &dwType, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
		{
			 if(dwType == REG_SZ || dwType == REG_EXPAND_SZ)
			 {
#ifdef _DEBUG
				if(bLog)
					theApp.QueueDebugLogLineEx(LOG_INFO, L"GetRegKeyVista: RegQueryValueEx (%s) returned '%s'", sKey, szValue);
#endif
				return (CString)szValue;
			}
			 if(bLog)
				 theApp.QueueDebugLogLineEx(LOG_ERROR, L"GetRegKeyVista: RegQueryValueEx (%s) returned wrong type %u", sKey, dwType);
		}
		else if(bLog)
			theApp.QueueDebugLogLineEx(LOG_ERROR, L"GetRegKeyVista: RegQueryValueEx (%s) failed to query key", sKey);
		RegCloseKey(hKey);
	}
#ifdef _DEBUG
	if(bLog)
		theApp.QueueDebugLogLineEx(LOG_ERROR, L"GetRegKeyVista: RegOpenKeyEx (%s) failed", VISTA_KEY);
#endif
	return L"";
}
//<<< taz::VistaFix [pP]

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSysInfo::CSysInfo()
{
	m_bWindowsNT = GetVersion() < 0x80000000;
	m_dwWinMajor=0;
	m_dwWinMinor=0;
	m_dwBuildNumber=0;
	m_dwServicePack=0;
	m_dwNumProcessors=0;
	m_dwCPUSpeed=0;

	m_dwMemoryLoad=0;
	m_dwAvailVirtual=0;
	m_dwTotalVirtual=0;
	m_dwAvailPageFile=0;
	m_dwTotalPageFile=0;
	m_dwAvailPhys=0;
	m_dwTotalPhys=0;
	m_bUserAdmin=FALSE;

	m_GetProcessMemoryInfo=NULL;
	m_bPsapiDll=FALSE;
	time(&m_StartupTime);

	Init();
}

CSysInfo::~CSysInfo()
{
	if (m_bPsapiDll) FreeModule(m_hmPsapiDll);
}

void CSysInfo::Init()
{
	// Init routines. It must be called before getting system information.
	m_hmPsapiDll = NULL;
	if (IsWindowsNT())
	{
		if((m_hmPsapiDll=::LoadLibrary(L"psapi.dll"))!=NULL)
		{
			m_bPsapiDll=TRUE;
			m_GetProcessMemoryInfo=(pGetProcessMemoryInfo)::GetProcAddress(m_hmPsapiDll, "GetProcessMemoryInfo");
		}
	}	

	DetectOSType();
	DetectNumProcessors();
	DetectCPUType();
	DetectMemory();
	DetectUserAdmin();
	DetectLoginUserName();
	DetectWorkingDir();
}

//
//  Detect a type of operation system
//
void CSysInfo::DetectOSType()
{
	BOOL bOSINFOEXWorking=FALSE;

	OSVERSIONINFO v;
	ZeroMemory(&v, sizeof(OSVERSIONINFO));
	v.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&v);

	m_dwWinMajor=v.dwMajorVersion;
	m_dwWinMinor=v.dwMinorVersion;
	m_dwBuildNumber=v.dwBuildNumber;

	if (!IsWindowsNT())
	{
		// Windows 9x/ME.
		if (m_dwWinMinor==0)  { m_sOSType=W95_s; };
		if (m_dwWinMinor==10) { m_sOSType=W98_s; };
		if (m_dwWinMinor==90) { m_sOSType=WME_s; };
		m_sOSType+=v.szCSDVersion;
		return;
	};

	// Get Service Pack number
	int n=0, j=-1; CString sSvcPack; TCHAR szNum[10];
	sSvcPack=v.szCSDVersion;
	for (n=1;n<=9;n++) {
		_stprintf(szNum, L"%d", n);
		j=sSvcPack.Find(szNum);
		if (j>=0) break;
	};
	if (j>=0) m_dwServicePack=n;

	// Check for OSVERSIONINFOEX-compatibility
	// It works only on Windows NT 4.0 Service Pack 6 or better
	if ((m_dwWinMajor>=5) || (m_dwServicePack>=6))
	{ bOSINFOEXWorking=TRUE; };

	if (m_dwWinMajor==4)
	{
		// Windows NT 4.0
		m_sOSType=NT4_s;
	}
//>>> taz::VistaFix [pP]
/*
	else
	{
		if (m_dwWinMajor>=5)
		{
			if (m_dwWinMinor==0)
				m_sOSType=W2K_s;  // Windows 2000
			else if (m_dwWinMinor==1)
				m_sOSType=WXP_s;  // Windows XP
			else if (m_dwWinMinor==1)
				m_sOSType=WVI_s;  // Windows Vista+Longhorn Server
		};
	};
*/
	else if (m_dwWinMajor==5)
	{
		if (m_dwWinMinor==0)
			m_sOSType=W2K_s;  // Windows 2000
		else if (m_dwWinMinor==1)
			m_sOSType=WXP_s; // Windows XP
	}
	// pp // Windows Vista
	else if (m_dwWinMajor>=6)
	{
		bOSINFOEXWorking = false; // skip extended info, not supported for Vista anyways 
		//WiZaRd: this statement ^^ is WRONG though the needed defines may be missing... they are supplied with V$2005 and higher
		//one could use "GetProductInfo" to get the missing data but for now I leave the code that taz added but with little fixes
		CString sProduct = GetRegKeyVista(L"ProductName");
		CString sService = GetRegKeyVista(L"CSDVersion");
		if (!sProduct.IsEmpty())
		{
			m_sOSType = sProduct;
			if(!sService.IsEmpty())
				m_sOSType.AppendFormat(L" %s", sService);
		}
		else
			m_sOSType.Format(L"Microsoft Windows %u.%u, unknown version", m_dwWinMajor, m_dwWinMinor);
	}
//<<< taz::VistaFix [pP]

	if (bOSINFOEXWorking)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		bOsVersionInfoEx=GetVersionEx((LPOSVERSIONINFO)&osvi);

		if (bOsVersionInfoEx)
		{
			if (osvi.wProductType==VER_NT_WORKSTATION)
			{
				if ((osvi.dwMajorVersion==5) && (osvi.dwMinorVersion>=1))
				{
					// Windows XP
					if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
					{ m_sOSType+=XHE_s; }
					else
					{ m_sOSType+=XPR_s; };
				};

				if ((osvi.dwMajorVersion==5) && (osvi.dwMinorVersion==0))
				{
					// Windows 2000 Professional
					m_sOSType+=PRO_s;
				};

				if (osvi.dwMajorVersion==4)
				{
					// Windows NT 4.0 Workstation
					m_sOSType+=WKS_s;
				};
			};

			if ((osvi.wProductType==VER_NT_SERVER) || (osvi.wProductType==VER_NT_DOMAIN_CONTROLLER))
			{
				if (osvi.dwMajorVersion==5)
				{
					if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) { m_sOSType+=ADV_s; };
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER) { m_sOSType+=DTC_s; };
					if (osvi.wSuiteMask & VER_SUITE_TERMINAL) { m_sOSType+=TER_s; };
					if (osvi.wSuiteMask & VER_SUITE_BACKOFFICE) { m_sOSType+=BOF_s; };
					m_sOSType+=SER_s;
				};
				if (osvi.dwMajorVersion==4 && osvi.dwMinorVersion==0) { m_sOSType+=SER_s; };
			};

			if (osvi.wProductType==VER_NT_DOMAIN_CONTROLLER)
			{ m_sOSType+=DOM_s; };
		};
	};

	if (m_dwServicePack>0)
	{
		m_sOSType += L" ";
		m_sOSType+=v.szCSDVersion;
	}
}

//
// Detect a number of processors
//
void CSysInfo::DetectNumProcessors()
{
	SYSTEM_INFO sinfo;
	GetSystemInfo(&sinfo);
	m_dwNumProcessors=sinfo.dwNumberOfProcessors;
}

//
// Detect CPU type
//
void CSysInfo::DetectCPUType()
{
	HKEY	NewKey;

	LONG	lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CPU_CONFIGURATION_KEY, 
		0, KEY_EXECUTE, &NewKey);

	if (lresult != ERROR_SUCCESS) 
		return;		// key not found


	TCHAR szKeyValue[100]; 
	memset(szKeyValue, 0, sizeof(szKeyValue));
	DWORD dwType = REG_SZ; 
	DWORD dwSize = 100;

	lresult = RegQueryValueEx(NewKey, L"Identifier", NULL, 
		&dwType, (LPBYTE)szKeyValue, &dwSize);

	if (lresult == ERROR_SUCCESS && dwSize != 0)
		m_sCPUIdentifier = szKeyValue; 

	memset(szKeyValue, 0, sizeof(szKeyValue));
	dwType = REG_SZ; 
	dwSize = 100;

	lresult = RegQueryValueEx(NewKey, L"VendorIdentifier", NULL, 
		&dwType, (LPBYTE)szKeyValue, &dwSize);

	if (lresult == ERROR_SUCCESS && dwSize != 0)
		m_sCPUVendorIdentifier = szKeyValue; 

	memset(szKeyValue, 0, sizeof(szKeyValue));
	dwType = REG_SZ; 
	dwSize = 100;

	lresult = RegQueryValueEx(NewKey, L"ProcessorNameString", 
		NULL, &dwType, (LPBYTE)szKeyValue, &dwSize);

	if (lresult == ERROR_SUCCESS && dwSize != 0)
		m_sCPUNameString = szKeyValue;

	DWORD dwData = 0; 
	dwType = REG_DWORD; 
	dwSize = sizeof(dwData);
	lresult = RegQueryValueEx(NewKey, L"~MHz", NULL, 
		&dwType, (LPBYTE)(&dwData), &dwSize);

	if (lresult == ERROR_SUCCESS && dwSize != 0)
		m_dwCPUSpeed = dwData;

	RegCloseKey(NewKey);
}

//
// Does the current user have administrator rights ?
//
void CSysInfo::DetectUserAdmin()
{
	if (!IsWindowsNT())
	{
		m_bUserAdmin=TRUE;
		return;
	}

	HANDLE hAccessToken       = NULL;
	PBYTE  pInfoBuffer        = NULL;
	DWORD  dwInfoBufferSize   = 1024;
	PTOKEN_GROUPS ptgGroups   = NULL;
	PSID   psidAdministrators = NULL;
	SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
	//BOOL   bResult = FALSE;

	__try
	{
		// init security token
		if( !AllocateAndInitializeSid( 
			&siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdministrators ) )
			__leave;

		// for Windows NT 4.0 only
		if( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hAccessToken ) )
			__leave;

		do
		{
			if( pInfoBuffer )
				delete[] pInfoBuffer; //>>> WiZaRd::Memleak FiX
			pInfoBuffer = new BYTE[dwInfoBufferSize];
			if( !pInfoBuffer )
				__leave;
			SetLastError( 0 );
			if( !GetTokenInformation( 
				hAccessToken, 
				TokenGroups, pInfoBuffer, 
				dwInfoBufferSize, &dwInfoBufferSize ) &&
				( ERROR_INSUFFICIENT_BUFFER != GetLastError() )
				)
				__leave;
			else
				ptgGroups = (PTOKEN_GROUPS)pInfoBuffer;
		}
		while( GetLastError() );

		for(UINT i = 0; i < ptgGroups->GroupCount; ++i)
		{
			if( EqualSid(psidAdministrators, ptgGroups->Groups[i].Sid) )
			{
				m_bUserAdmin=TRUE;
				__leave;
			}
		} 
	}

	__finally
	{
		if( hAccessToken )
			CloseHandle( hAccessToken );
		if( pInfoBuffer )
			delete[] pInfoBuffer; //>>> WiZaRd::Memleak FiX
		if( psidAdministrators )
			FreeSid( psidAdministrators );
	}
}

BOOL CSysInfo::GetFileInfo(LPCTSTR lpszFileName, DWORD *v_major, DWORD *v_minor, 
						   DWORD *v_build, DWORD *v_something, DWORD *size)
{
	*v_major=0; *v_minor=0; *v_build=0; *v_something=0; *size=0;
	if (lpszFileName==NULL) return FALSE;

	DWORD InfoSize;
	VS_FIXEDFILEINFO  fileinfo;
	VS_FIXEDFILEINFO  *fi;
	fi=&fileinfo;
	UINT size_fileinfo; size_fileinfo=sizeof(fileinfo);

	LPTSTR lpFileName; int i=0; i=_tcslen(lpszFileName);
	if (i>0)
	{
		// Convert LPCTSTR to LPTSTR
		lpFileName=new TCHAR[i+1];
		memset(lpFileName, 0, i+1);
		_tcscpy(lpFileName, lpszFileName);
	}
	else
		return FALSE;

	InfoSize=GetFileVersionInfoSize(lpFileName, 0);
	if (InfoSize>0)
	{
		LPVOID lpVoid;
		lpVoid=malloc(InfoSize);
		memset(lpVoid, 0, InfoSize);
		if (GetFileVersionInfo(lpFileName, NULL, InfoSize, lpVoid))
		{
			if (VerQueryValue(lpVoid, L"\\", (LPVOID *)&fi, &size_fileinfo))
			{	
				*v_major=((fi->dwFileVersionMS>>16) & 0x0000FFFF);
				*v_minor=(fi->dwFileVersionMS & 0x0000FFFF);
				*v_build=((fi->dwFileVersionLS>>16) & 0x0000FFFF);
				*v_something=(fi->dwFileVersionLS & 0x0000FFFF);
			}
		}
		free(lpVoid);
		delete[] lpFileName; //>>> WiZaRd::Memleak FiX
	}
	else
	{
		delete[] lpFileName; //>>> WiZaRd::Memleak FiX
		return FALSE;
	}

	// Get file size
	WIN32_FIND_DATA fdata; HANDLE hFile;

	hFile=FindFirstFile(lpszFileName, &fdata);
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		*size=fdata.nFileSizeLow;
		FindClose(hFile);
		return TRUE;
	}
	else
		return FALSE;
}

//
// Get file information in the explicit system directory (e.g. c:\winnt\system32)
//
BOOL CSysInfo::GetSystemFileInfo(LPCTSTR lpszFileName, DWORD *v_major, DWORD *v_minor, 
								 DWORD *v_build, DWORD *v_something, DWORD *size)
{
	*v_major=0; *v_minor=0; *v_build=0; *v_something=0; *size=0;

	CString str;
	TCHAR	szSysDirectory[100];
	int i=GetSystemDirectory(szSysDirectory, 50);
	if (i==0) return FALSE;
	str.Format(L"%s\\%s", szSysDirectory, lpszFileName);
	if (GetFileInfo((LPCTSTR)str, v_major, v_minor, v_build, v_something, size)) return TRUE;
	return FALSE;
}

void CSysInfo::DetectLoginUserName()
{
	DWORD dwSize = 100;
	LPTSTR pStr = m_sUserName.GetBuffer(150);
	GetUserName(pStr, &dwSize);
	m_sUserName.ReleaseBuffer();
}

void CSysInfo::DetectWorkingDir()
{
	LPTSTR pStr = m_sWorkingDir.GetBuffer(250);
	DWORD dwRes=::GetModuleFileName(NULL, pStr, 200);
	m_sWorkingDir.ReleaseBuffer();
	if (dwRes != 0)
	{
		int i=m_sWorkingDir.ReverseFind('\\');
		if (i != -1) 
			m_sWorkingDir = m_sWorkingDir.Left(i+1);
	}
}

//
// Detect the size of the current process
//
DWORD CSysInfo::GetProcessMemoryUsage()
{
	if (m_GetProcessMemoryInfo!=NULL)
	{
		// Get a handle to the process.
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, _getpid());
		if (hProcess)
		{
			m_GetProcessMemoryInfo(hProcess, &m_ProcMemCounters, sizeof(m_ProcMemCounters));
			CloseHandle(hProcess);
			DWORD f = (DWORD)((m_ProcMemCounters.WorkingSetSize)/1024);
			return f;
		}
		else
			return 0;
	}
	else
		return 0;
}

//
// Detect memory sizes
//
void CSysInfo::DetectMemory()
{
	if (IsWindowsNT() && m_dwWinMajor > 4)
	{
		// Windows 2000+. Use GlobalMemoryStatusEx to detect memory size more than 4GB
		MEMORYSTATUSEX MemStatEx;
		memset(&MemStatEx, 0, sizeof(MemStatEx));
		MemStatEx.dwLength = sizeof(MemStatEx);

		HMODULE hmKernelDll;
		m_pGlobalMemoryStatusEx = NULL;
		BOOL bSuccess = FALSE;
		if((hmKernelDll=::LoadLibrary(L"kernel32.dll"))!=NULL)
		{
			m_pGlobalMemoryStatusEx=(pGlobalMemoryStatusEx)::GetProcAddress(hmKernelDll, "GlobalMemoryStatusEx");
			if (m_pGlobalMemoryStatusEx!=NULL)
			{
				if ((*m_pGlobalMemoryStatusEx)(&MemStatEx))
				{
					m_dwMemoryLoad=MemStatEx.dwMemoryLoad;
					m_dwTotalPhys=MemStatEx.ullTotalPhys;
					m_dwAvailPhys=MemStatEx.ullAvailPhys;
					m_dwTotalPageFile=MemStatEx.ullTotalPageFile;
					m_dwAvailPageFile=MemStatEx.ullAvailPageFile;
					m_dwTotalVirtual=MemStatEx.ullTotalVirtual;
					m_dwAvailVirtual=MemStatEx.ullAvailVirtual;
					bSuccess=TRUE;
				};
			};
			::FreeLibrary(hmKernelDll);
		};
		if (bSuccess) 
			return;
	}

	MEMORYSTATUS MemStat;
	memset(&MemStat, 0, sizeof(MemStat));
	MemStat.dwLength=sizeof(MemStat);

	GlobalMemoryStatus(&MemStat);
	m_dwMemoryLoad=MemStat.dwMemoryLoad;
	m_dwTotalPhys=MemStat.dwTotalPhys;
	m_dwAvailPhys=MemStat.dwAvailPhys;
	m_dwTotalPageFile=MemStat.dwTotalPageFile;
	m_dwAvailPageFile=MemStat.dwAvailPageFile;
	m_dwTotalVirtual=MemStat.dwTotalVirtual;
	m_dwAvailVirtual=MemStat.dwAvailVirtual;
}

time_t CSysInfo::GetUpTime()
{
	time_t now;
	time(&now);
	return (now-m_StartupTime);
}