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

#include "stdafx.h"
#include "SysInfo.h"
#include <process.h>

// --------> Vista sensing fix
#include "Log.h"
// <------- Vista sensing fix


#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Version.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSysInfo::CSysInfo()
{
	if (GetVersion()<0x80000000) {
		m_bWindowsNT=TRUE;
	} else { 
		m_bWindowsNT=FALSE;
	};

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

	m_dwIESomething=0;
	m_dwIEBuild=0;
	m_dwIEMinor=0;
	m_dwIEMajor=0;

	m_dwADOMajor=0;
	m_dwADOMinor=0;
	m_dwADOBuild=0;
	m_dwADOSomething=0;

	m_bUserAdmin=FALSE;

	m_GetProcessMemoryInfo=NULL;
	m_bPsapiDll=FALSE;
	time(&m_StartupTime);
}

CSysInfo::~CSysInfo()
{
	if (m_bPsapiDll) FreeModule(m_hmPsapiDll);
}

void CSysInfo::Init()
{
	// Init routines. It must be called before getting system information.

	if (IsWindowsNT())
	{
		if((m_hmPsapiDll=::LoadLibrary(_T("psapi.dll")))!=NULL)
		{
			m_bPsapiDll=TRUE;
			m_GetProcessMemoryInfo=(pGetProcessMemoryInfo)::GetProcAddress(m_hmPsapiDll,"GetProcessMemoryInfo");
		};
	};

	DetectOSType();
	DetectNumProcessors();
	DetectCPUType();
	DetectMemory();
	DetectIEVersion();
	DetectADOVersion();

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
	ZeroMemory(&v,sizeof(OSVERSIONINFO));
	v.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx((LPOSVERSIONINFO)&v);

	m_dwWinMajor=v.dwMajorVersion;
	m_dwWinMinor=v.dwMinorVersion;
	m_dwBuildNumber=v.dwBuildNumber;

	if (!IsWindowsNT())
	{
		// Windows 9x/ME.
		if (m_dwWinMinor==0)  { m_sOSType=_T(W95_s); };
		if (m_dwWinMinor==10) { m_sOSType=_T(W98_s); };
		if (m_dwWinMinor==90) { m_sOSType=_T(WME_s); };
		m_sOSType+=v.szCSDVersion;
		return;
	};

	// Get Service Pack number
	int n=0,j=-1; CString sSvcPack; TCHAR szNum[10];
	sSvcPack=v.szCSDVersion;
	for (n=1;n<=9;n++) {
		_stprintf(szNum,_T("%d"),n);
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
		m_sOSType=_T(NT4_s);
	}
	else
// -------> Vista sensing fix
// pp vista fix
/*
	{
		if (m_dwWinMajor>=5)
		{
			if (m_dwWinMinor==0) { m_sOSType=_T(W2K_s); };  // Windows 2000
			if (m_dwWinMinor==1) { m_sOSType=_T(WXP_s); };  // Windows XP
		};
	};
*/
	if (m_dwWinMajor==5)
	{
		if (m_dwWinMinor==0)
			m_sOSType=_T(W2K_s);  // Windows 2000
		else if (m_dwWinMinor==1)
			m_sOSType=_T(WXP_s); // Windows XP
	}
	// pp // Windows Vista
	else if (m_dwWinMajor>=6)
        {
		bOSINFOEXWorking=false;
		CString strProduct, strService;
		strProduct = GetRegKeyVista(L"ProductName");
		strService = GetRegKeyVista(L"CSDVersion");
		if (!strProduct.IsEmpty())
			//m_sOSType.Format(L"%s %s", strProduct, strService.IsEmpty()?L"":strService);
	            m_sOSType.Format(L"%s",strProduct);
                 else
			m_sOSType.Format(L"Microsoft Windows %u.%u, unknown version", m_dwWinMajor, m_dwWinMinor);
		}
//<< pP: VistaFix
// <------- Vista sensing fix

	if (bOSINFOEXWorking)
	{
		MYOSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;
		ZeroMemory(&osvi,sizeof(MYOSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(MYOSVERSIONINFOEX);
		bOsVersionInfoEx=GetVersionEx((LPOSVERSIONINFO)&osvi);

		if (bOsVersionInfoEx)
		{
			if (osvi.wProductType==MY_VER_NT_WORKSTATION)
			{
				if ((osvi.dwMajorVersion==5) && (osvi.dwMinorVersion==1))
				{
					// Windows XP
					if (osvi.wSuiteMask & MY_VER_SUITE_PERSONAL)
					{ m_sOSType+=_T(XHE_s); }
					else
					{ m_sOSType+=_T(XPR_s); };
				};

				if ((osvi.dwMajorVersion==5) && (osvi.dwMinorVersion==0))
				{
					// Windows 2000 Professional
					m_sOSType+=_T(PRO_s);
				};

				if (osvi.dwMajorVersion==4)
				{
					// Windows NT 4.0 Workstation
					m_sOSType+=_T(WKS_s);
				};
			};

			if ((osvi.wProductType==MY_VER_NT_SERVER) || (osvi.wProductType==MY_VER_NT_DOMAIN_CONTROLLER))
			{
				if (osvi.dwMajorVersion==5)
				{
					if (osvi.wSuiteMask & MY_VER_SUITE_ENTERPRISE) { m_sOSType+=_T(ADV_s); };
					if (osvi.wSuiteMask & MY_VER_SUITE_DATACENTER) { m_sOSType+=_T(DTC_s); };
					if (osvi.wSuiteMask & MY_VER_SUITE_TERMINAL) { m_sOSType+=_T(TER_s); };
					if (osvi.wSuiteMask & MY_VER_SUITE_BACKOFFICE) { m_sOSType+=_T(BOF_s); };
					m_sOSType+=_T(SER_s);
				};
				if (osvi.dwMajorVersion==4 && osvi.dwMinorVersion==0) { m_sOSType+=_T(SER_s); };
			};

			if (osvi.wProductType==MY_VER_NT_DOMAIN_CONTROLLER)
			{ m_sOSType+=_T(DOM_s); };
		};
	};

	if (m_dwServicePack>0)
	{
		m_sOSType+=_T(" ");
		m_sOSType+=v.szCSDVersion;
	};
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
// Detect Internet Explorer version
//
void CSysInfo::DetectIEVersion()
{
	LONG	lresult;
	HKEY	NewKey;

	lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T(IE_CONFIGURATION_KEY),
		0,KEY_EXECUTE,&NewKey);

	if (ERROR_SUCCESS!=lresult) return;		// key not found

	TCHAR szKeyValue[100]; memset(szKeyValue,0,100);
	DWORD dwType=REG_SZ; DWORD dwSize=100;

	lresult=RegQueryValueEx(NewKey,_T("Version"),NULL,&dwType,
		(LPBYTE)szKeyValue,&dwSize);

	RegCloseKey(NewKey);

	if (dwSize>0) {
		_stscanf(szKeyValue,_T("%d.%d.%d.%d"),&m_dwIEMajor,&m_dwIEMinor,&m_dwIEBuild,&m_dwIESomething);
	};
}

//
// Detect CPU type
//
void CSysInfo::DetectCPUType()
{
	LONG	lresult;
	HKEY	NewKey;

	lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T(CPU_CONFIGURATION_KEY),
		0,KEY_EXECUTE,&NewKey);

	if (ERROR_SUCCESS != lresult) return;		// key not found

	TCHAR szKeyValue[100]; memset(szKeyValue,0,100);
	DWORD dwType=REG_SZ; DWORD dwSize=100;

	lresult=RegQueryValueEx(NewKey,_T("Identifier"),NULL,
		&dwType,(LPBYTE)szKeyValue,&dwSize);

	if ((lresult==ERROR_SUCCESS) && (dwSize>0)) 
	{ m_sCPUIdentifier=szKeyValue; };

	memset(szKeyValue,0,100); dwType=REG_SZ; dwSize=100;

	lresult=RegQueryValueEx(NewKey,_T("VendorIdentifier"),NULL,
		&dwType,(LPBYTE)szKeyValue,&dwSize);

	if ((lresult==ERROR_SUCCESS) && (dwSize>0))
	{ m_sCPUVendorIdentifier=szKeyValue; };

	memset(szKeyValue,0,100); dwType=REG_SZ; dwSize=100;

	lresult=RegQueryValueEx(NewKey,_T("ProcessorNameString"),
		NULL,&dwType,(LPBYTE)szKeyValue,&dwSize);

	if ((lresult==ERROR_SUCCESS) && (dwSize>0))
	{ m_sCPUNameString=szKeyValue; };

	DWORD dwData=0; dwType=REG_DWORD; dwSize=sizeof(dwData);
	lresult=RegQueryValueEx(NewKey,_T("~MHz"),NULL,
		&dwType,(LPBYTE)(&dwData),&dwSize);

	if ((lresult==ERROR_SUCCESS) && (dwSize>0))
	{ m_dwCPUSpeed=dwData; };

	RegCloseKey(NewKey);
}

//
// Detect ADO version
//
void CSysInfo::DetectADOVersion()
{
	LONG	lresult;
	HKEY	NewKey;

	lresult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,_T(ADO_CONFIGURATION_KEY),
		0,KEY_EXECUTE,&NewKey);

	if (ERROR_SUCCESS != lresult) return;		// key not found

	TCHAR szKeyValue[100]; memset(szKeyValue,0,100);
	DWORD dwType=REG_SZ; DWORD dwSize=100;

	lresult=RegQueryValueEx(NewKey,_T("FullInstallVer"),
		NULL,&dwType,(LPBYTE)szKeyValue,&dwSize);

	if (ERROR_SUCCESS!=lresult)
	{
		memset(szKeyValue,0,100); dwType=REG_SZ; dwSize=100;

		lresult=RegQueryValueEx(NewKey,_T("Version"),
			NULL,&dwType,(LPBYTE)szKeyValue,&dwSize);
	};

	if (dwSize>0) {
		_stscanf(szKeyValue,_T("%d.%d.%d.%d"),&m_dwADOMajor,&m_dwADOMinor,&m_dwADOBuild,&m_dwADOSomething);
	};

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
	};

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
			DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &psidAdministrators ) )
			__leave;

		// for Windows NT 4.0 only
		if( !OpenProcessToken( GetCurrentProcess(),TOKEN_QUERY,&hAccessToken ) )
			__leave;

		do
		{
			if( pInfoBuffer )
				delete[] pInfoBuffer; // Memleak FiX [WiZaRd] - Stulle
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

		for( UINT i = 0; i < ptgGroups->GroupCount; i++ )
		{
			if( EqualSid(psidAdministrators,ptgGroups->Groups[i].Sid) )
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
			delete[] pInfoBuffer; // Memleak FiX [WiZaRd] - Stulle
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
		memset(lpFileName,0,i+1);
		_tcscpy(lpFileName,lpszFileName);
	}
	else
		return FALSE;

	InfoSize=GetFileVersionInfoSize(lpFileName,0);
	if (InfoSize>0)
	{
		LPVOID lpVoid;
		lpVoid=malloc(InfoSize);
		memset(lpVoid,0,InfoSize);
		if (GetFileVersionInfo(lpFileName,NULL,InfoSize,lpVoid))
		{
			if (VerQueryValue(lpVoid,_T("\\"),(LPVOID *)&fi,&size_fileinfo))
			{	
				*v_major=((fi->dwFileVersionMS>>16) & 0x0000FFFF);
				*v_minor=(fi->dwFileVersionMS & 0x0000FFFF);
				*v_build=((fi->dwFileVersionLS>>16) & 0x0000FFFF);
				*v_something=(fi->dwFileVersionLS & 0x0000FFFF);
			};
		};
		free(lpVoid);
		delete[] lpFileName; // Memleak FiX [WiZaRd] - Stulle
	}
	else
	{
		delete[] lpFileName; // Memleak FiX [WiZaRd] - Stulle
		return FALSE;
	};

	// Get file size
	WIN32_FIND_DATA fdata; HANDLE hFile;

	hFile=FindFirstFile(lpszFileName,&fdata);
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
	int i=GetSystemDirectory(szSysDirectory,50);
	if (i==0) return FALSE;
	str.Format(_T("%s\\%s"),szSysDirectory,lpszFileName);
	if (GetFileInfo((LPCTSTR)str,v_major,v_minor,v_build,v_something,size)) return TRUE;
	return FALSE;
}

void CSysInfo::DetectLoginUserName()
{
	LPTSTR pStr; DWORD dwSize=100;

	pStr=m_sUserName.GetBuffer(150);
	GetUserName(pStr,&dwSize);
	m_sUserName.ReleaseBuffer();
}

void CSysInfo::DetectWorkingDir()
{
	LPTSTR pStr;

	pStr=m_sWorkingDir.GetBuffer(250);
	DWORD dwRes=::GetModuleFileName(NULL,pStr,200);
	m_sWorkingDir.ReleaseBuffer();
	if (dwRes>0)
	{
		int i=m_sWorkingDir.ReverseFind('\\');
		if (i>0) m_sWorkingDir=m_sWorkingDir.Left(i+1);
	};
}

//
// Detect the size of the current process
//
DWORD CSysInfo::GetProcessMemoryUsage()
{
	if (m_GetProcessMemoryInfo!=NULL)
	{
		// Get a handle to the process.
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,_getpid()); //VS2008 getpid -> _getpid
		if (hProcess)
		{
			m_GetProcessMemoryInfo(hProcess,&m_ProcMemCounters,sizeof(m_ProcMemCounters));
			CloseHandle(hProcess);
			DWORD f=(DWORD)(m_ProcMemCounters.WorkingSetSize/1024);
			return f;
		}
		else
			return 0;
	}
	else
		return 0;
}

// ==> changed - Stulle
uint32 CSysInfo::GetProcessMemoryUsageInt()
{
	if (m_GetProcessMemoryInfo!=NULL)
	{
		// Get a handle to the process.
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,_getpid());//vs2008
		if (hProcess)
		{
			m_GetProcessMemoryInfo(hProcess,&m_ProcMemCounters,sizeof(m_ProcMemCounters));
			CloseHandle(hProcess);
			uint32 f=m_ProcMemCounters.WorkingSetSize;
			return f;
		}
		else
			return 0;
	}
	else
		return 0;
}
// <== changed - Stulle

//
// Detect memory sizes
//
void CSysInfo::DetectMemory()
{
	BOOL bSuccess=FALSE;

	if (IsWindowsNT() & (m_dwWinMajor>=5))
	{
		// Windows 2000+. Use GlobalMemoryStatusEx to detect memory size more than 4GB
		MYMEMORYSTATUSEX MemStatEx;
		MemStatEx.dwLength=0;
		MemStatEx.dwMemoryLoad=0;
		MemStatEx.ullTotalPhys=0;
		MemStatEx.ullAvailPhys=0;
		MemStatEx.ullTotalPageFile=0;
		MemStatEx.ullAvailPageFile=0;
		MemStatEx.ullTotalVirtual=0;
		MemStatEx.ullAvailVirtual=0;
		MemStatEx.dwLength=sizeof(MemStatEx);

		HMODULE hmKernelDll;
		m_pGlobalMemoryStatusEx=NULL;
		if((hmKernelDll=::LoadLibrary(_T("kernel32.dll")))!=NULL)
		{
			m_pGlobalMemoryStatusEx=(pGlobalMemoryStatusEx)::GetProcAddress(hmKernelDll,"GlobalMemoryStatusEx");
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
	}

	if (bSuccess) return;

	MEMORYSTATUS MemStat;

	MemStat.dwMemoryLoad=0;
	MemStat.dwTotalPhys=0;
	MemStat.dwAvailPhys=0;
	MemStat.dwTotalPageFile=0;
	MemStat.dwAvailPageFile=0;
	MemStat.dwTotalVirtual=0;
	MemStat.dwAvailVirtual=0;
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

//>> pP: VistaFix
CString CSysInfo::GetRegKeyVista(CString sKey)
{
	// get the WinVer from registry
	int long lnResult;
	HKEY hKey=NULL;
	LPCTSTR lpSubKey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
	lnResult=RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey);

	if(!hKey)
        {
		//AddLogLine(0, L"RegOpenKeyEx failed (%s)", sKey);
		return L"";
	}

	DWORD dwType;
	TCHAR szValue[100];
	DWORD dwSize = sizeof(szValue)*sizeof(TCHAR);
	LONG lRet = RegQueryValueEx(hKey, sKey, NULL, &dwType, (LPBYTE) szValue, &dwSize);

	if ((lRet == ERROR_SUCCESS) && ( (dwType == REG_SZ) || (dwType == REG_EXPAND_SZ)))
	{
		//AddLogLine(0, L"RegQueryValueEx (%s) returned '%s'", sKey, szValue);
		return (CString)szValue;
	}
	//AddLogLine(0, L"RegQueryValueEx failed (%s)", sKey);
	return L"";
}
//<< pP: VistaFix
