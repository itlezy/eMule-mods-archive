// SysInfo.h: interface for the CSysInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSINFO_H__89E72643_E342_4856_A11D_768096354EB9__INCLUDED_)
#define AFX_SYSINFO_H__89E72643_E342_4856_A11D_768096354EB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <iphlpapi.h>
#include <iptypes.h>
#include <iprtrmib.h>

#include "mibaccess.h"

typedef DWORD (WINAPI *pGetAdaptersInfo)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);
typedef DWORD (WINAPI *pGetIfEntry)(PMIB_IFROW pIfRow);

#define CSYSINFO_VERSION "2.0"

#define W95_s "Windows 95"
#define W98_s "Windows 98"
#define WME_s "Windows ME"
#define NT4_s "Windows NT 4.0"
#define WKS_s " Workstation"
#define WXP_s "Windows XP"
#define XHE_s " Home Edition"
#define XPR_s " Professional Edition"
#define W2K_s "Windows 2000"
#define PRO_s " Professional"
#define DOM_s " Domain Controller"
#define SER_s " Server"
#define ADV_s " Advanced"
#define DTC_s " DataCenter"
#define TER_s " Terminal"
#define BOF_s " BackOffice"

#define MY_VER_SERVER_NT						0x80000000
#define MY_VER_WORKSTATION_NT					0x40000000
#define MY_VER_SUITE_SMALLBUSINESS				0x00000001
#define MY_VER_SUITE_ENTERPRISE					0x00000002
#define MY_VER_SUITE_BACKOFFICE					0x00000004
#define MY_VER_SUITE_COMMUNICATIONS				0x00000008
#define MY_VER_SUITE_TERMINAL					0x00000010
#define MY_VER_SUITE_SMALLBUSINESS_RESTRICTED	0x00000020
#define MY_VER_SUITE_EMBEDDEDNT					0x00000040
#define MY_VER_SUITE_DATACENTER					0x00000080
#define MY_VER_SUITE_SINGLEUSERTS				0x00000100
#define MY_VER_SUITE_PERSONAL					0x00000200
#define MY_VER_SUITE_BLADE						0x00000400

#define MY_VER_NT_WORKSTATION					0x0000001
#define MY_VER_NT_DOMAIN_CONTROLLER				0x0000002
#define MY_VER_NT_SERVER						0x0000003


typedef struct _MYOSVERSIONINFOEXA {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	TCHAR szCSDVersion[128];
	WORD  wServicePackMajor;
	WORD  wServicePackMinor;
	WORD  wSuiteMask;
	BYTE  wProductType;
	BYTE  wReserved;
} MYOSVERSIONINFOEXA, *MYPOSVERSIONINFOEXA, *MYLPOSVERSIONINFOEXA;

typedef MYOSVERSIONINFOEXA MYOSVERSIONINFOEX;
typedef MYPOSVERSIONINFOEXA MYPOSVERSIONINFOEX;
typedef MYLPOSVERSIONINFOEXA MYLPOSVERSIONINFOEX;

typedef struct _MYMEMORYSTATUSEX {
	DWORD dwLength; 
	DWORD dwMemoryLoad; 
	DWORDLONG ullTotalPhys; 
	DWORDLONG ullAvailPhys; 
	DWORDLONG ullTotalPageFile; 
	DWORDLONG ullAvailPageFile; 
	DWORDLONG ullTotalVirtual; 
	DWORDLONG ullAvailVirtual; 
	DWORDLONG ullAvailExtendedVirtual;
} MYMEMORYSTATUSEX, *LPMYMEMORYSTATUSEX;

typedef BOOL (WINAPI *pGlobalMemoryStatusEx)(LPMYMEMORYSTATUSEX lpBuffer);

#define IE_CONFIGURATION_KEY "SOFTWARE\\Microsoft\\Internet Explorer"
#define CPU_CONFIGURATION_KEY "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
#define ADO_CONFIGURATION_KEY "SOFTWARE\\Microsoft\\DataAccess"

#define MAX_IPADDRESSES 100

// The set of operational states of the network interface.
#define MIB_IFOPERSTATUS_UP 1
#define MIB_IFOPERSTATUS_DOWN 2
#define MIB_IFOPERSTATUS_TESTING 3

#define MIB_IFTYPE_ETHERNET_CSMACD 6
#define MIB_IFTYPE_PPP 23
#define MIB_IFTYPE_LOOPBACK 24

typedef struct _PROCESS_MEMORY_COUNTERS {
	DWORD cb;
	DWORD PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;

typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;
typedef	BOOL (WINAPI *pGetProcessMemoryInfo)(HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD);

class CSysInfo  
{
public:
	time_t GetUpTime();
	time_t m_StartupTime;

	LPCTSTR GetWorkingDir() { return (LPCTSTR)m_sWorkingDir; };
	LPCTSTR GetLoginUserName() { return (LPCTSTR)m_sUserName; };
	BOOL GetSystemFileInfo(LPCTSTR lpszFileName, DWORD *v_major, DWORD *v_minor, DWORD *v_build, DWORD *v_something, DWORD *size);
	BOOL GetFileInfo(LPCTSTR lpszFileName, DWORD *v_major, DWORD *v_minor, DWORD *v_build, DWORD *v_something, DWORD *size);
	BOOL IsWindowsNT() { return m_bWindowsNT; };
	BOOL IsUserAdmin() { return m_bUserAdmin; };

	DWORD m_dwMIBIPArray[MAX_IPADDRESSES];
	DWORD GetMIBAdapterMaskDword(int i);
	DWORD GetMIBAdapterIPDword(int i);
	int GetMIBAdapterType(int i);
	DWORD GetMIBAdapterOperStatus(int i);
	DWORD GetMIBAdapterSpeed(int i);
	int GetMIBNumAdapters() { return m_nMIBAdapters; }
	void GetMIBAdapterMaskStr(int i, LPTSTR lpMaskStr);
	void GetMIBAdapterIPStr(int i, LPTSTR lpIPStr);
	LPCSTR GetMIBAdapterDescr(int i);
	void DetectAdaptersMIB();
	void SetShowLoopback(BOOL bStatus) { m_bMIBShowLoopback=bStatus; };
	BOOL IsShowLoopback() { return m_bMIBShowLoopback; };
	DWORD GetMIBIP(int index);
	int GetMIBAddresses() { return m_nMIBAddresses; };
	int MIBRefreshAddresses();

	BOOL IsIphlpapiCompat() { return m_bIphlpapiCompat; };
	BOOL GetAPIAdapterSpeedStatus(int i, DWORD *pdwSpeed, DWORD *pdwStatus);
	DWORD GetAPIAdapterIndex(int i);
	LPCSTR GetAPIAdapterMaskStr(int i,int indexIP);
	LPCSTR GetAPIAdapterIPStr(int i, int indexIP);
	int GetAPINumAddresses(int i);
	LPCSTR GetAPIAdapterName(int i);
	LPCSTR GetAPIAdapterDescr(int i);
	int GetAPINumAdapters() { return m_nNumAdaptersIphlpapi; };

	DWORD GetADOSomething() { return m_dwADOSomething; };
	DWORD GetADOBuild() { return m_dwADOBuild; };
	DWORD GetADOMinor() { return m_dwADOMinor; };
	DWORD GetADOMajor() { return m_dwADOMajor; };

	LPCTSTR GetCPUNameString() { return (LPCTSTR)m_sCPUNameString; };
	LPCTSTR GetCPUVendorIdentifier() { return (LPCTSTR)m_sCPUVendorIdentifier; };
	LPCTSTR GetCPUIdentifier() { return (LPCTSTR)m_sCPUIdentifier; };
	DWORD GetCPUSpeed() { return m_dwCPUSpeed; };
	DWORD GetNumProcessors() { return m_dwNumProcessors; };

	DWORD GetIESomething() { return m_dwIESomething; };
	DWORD GetIEBuild() { return m_dwIEBuild; };
	DWORD GetIEMinor() { return m_dwIEMinor; };
	DWORD GetIEMajor() { return m_dwIEMajor; };

	DWORD GetProcessMemoryUsage();
	uint32 GetProcessMemoryUsageInt(); // changed - Stulle
	DWORD GetMemoryLoad() { return m_dwMemoryLoad; };
	DWORD64 GetAvailVirtual() { return m_dwAvailVirtual; };
	DWORD64 GetTotalVirtual() { return m_dwTotalVirtual; };
	DWORD64 GetAvailPageFile() { return m_dwAvailPageFile; };
	DWORD64 GetTotalPageFile() { return m_dwTotalPageFile; };
	DWORD64 GetAvailPhys() { return m_dwAvailPhys; };
	DWORD64 GetTotalPhys() { return m_dwTotalPhys; };

	DWORD GetWinMajor() { return m_dwWinMajor; };
	DWORD GetWinMinor() { return m_dwWinMinor; };
	DWORD GetBuildNumber() { return m_dwBuildNumber; };
	DWORD GetServicePack() { return m_dwServicePack; };
	LPCTSTR GetOSType() { return (LPCTSTR)m_sOSType; };

	void Init();
	CSysInfo();
	virtual ~CSysInfo();

protected:
	// Misc variables
	CString m_sWorkingDir;
	CString m_sUserName;
	BOOL m_bUserAdmin;

	// NIC list with MIB
	BOOL m_bMIBAllocated;
	tSTRUCTNICINFO *m_pMIBAdapters;
	int m_nMIBAdapters;
	int m_nMIBAddresses;
	DWORD m_dwMIBIPArray_tmp[MAX_IPADDRESSES];
	BOOL  m_bMIBShowLoopback;
	MibII	m_mib;
	BOOL	m_bMibDll;

	// NIC list with Iphlpapi
	BOOL m_bIphlpapiCompat;
	int m_nNumAdaptersIphlpapi;
	pGetIfEntry m_GetIfEntry;
	pGetAdaptersInfo m_GetAdaptersInfo;
	BOOL m_bIphlpapiAllocated;
	BOOL m_bIphlpapiObtained;
	BOOL	m_bIphlpapiDll;
	HMODULE	m_hmIphlpapiDll;
	PIP_ADAPTER_INFO m_pai;

	// Windows version and CPU type
	CString m_sCPUNameString;
	CString m_sCPUIdentifier;
	CString m_sCPUVendorIdentifier;
	CString	m_sOSType;
	BOOL  m_bWindowsNT;
	DWORD m_dwCPUSpeed;
	DWORD m_dwNumProcessors;
	DWORD m_dwServicePack;
	DWORD m_dwBuildNumber;
	DWORD m_dwWinMinor;
	DWORD m_dwWinMajor;

	// Memory counters
	BOOL m_bPsapiUsed;
	BOOL	m_bPsapiDll;
	HMODULE m_hmPsapiDll;
	PROCESS_MEMORY_COUNTERS m_ProcMemCounters;
	pGetProcessMemoryInfo m_GetProcessMemoryInfo;
	pGlobalMemoryStatusEx m_pGlobalMemoryStatusEx;
	DWORD m_dwMemoryLoad;
	DWORD64 m_dwAvailVirtual;
	DWORD64 m_dwTotalVirtual;
	DWORD64 m_dwAvailPageFile;
	DWORD64 m_dwTotalPageFile;
	DWORD64 m_dwAvailPhys;
	DWORD64 m_dwTotalPhys;

	// IE version
	DWORD m_dwIESomething;
	DWORD m_dwIEBuild;
	DWORD m_dwIEMinor;
	DWORD m_dwIEMajor;

	// ADO version
	DWORD m_dwADOSomething;
	DWORD m_dwADOBuild;
	DWORD m_dwADOMinor;
	DWORD m_dwADOMajor;

	void DetectWorkingDir();
	void DetectLoginUserName();
	void DetectUserAdmin();
	void DetectAdaptersIphlpapi();
	void DetectADOVersion();
	void DetectCPUType();
	void DetectIEVersion();
	void DetectMemory();
	void DetectNumProcessors();
	void DetectOSType();
};

#endif // !defined(AFX_SYSINFO_H__89E72643_E342_4856_A11D_768096354EB9__INCLUDED_)