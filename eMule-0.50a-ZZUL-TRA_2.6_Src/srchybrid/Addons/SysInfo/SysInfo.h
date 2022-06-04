// SysInfo.h: interface for the CSysInfo class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "CPU.h"

typedef BOOL (WINAPI *pGlobalMemoryStatusEx)(LPMEMORYSTATUSEX lpBuffer);

// The set of operational states of the network interface.
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

class CSysInfo : public CPU
{
private:
	CPU cpu;
	int sys;
	TKTime upTime;
public:
	int GetCpuUsage()		{return cpu.GetUsage( &sys, &upTime );};
	time_t GetUpTime();
	time_t m_StartupTime;

	LPCTSTR GetWorkingDir() const	{return (LPCTSTR)m_sWorkingDir;};
	LPCTSTR GetLoginUserName() const{return (LPCTSTR)m_sUserName;};
	BOOL GetSystemFileInfo(LPCTSTR lpszFileName, DWORD *v_major, DWORD *v_minor, DWORD *v_build, DWORD *v_something, DWORD *size);
	BOOL GetFileInfo(LPCTSTR lpszFileName, DWORD *v_major, DWORD *v_minor, DWORD *v_build, DWORD *v_something, DWORD *size);
	BOOL IsWindowsNT() const {return m_bWindowsNT;};
	BOOL IsUserAdmin() const {return m_bUserAdmin;};

	LPCTSTR GetCPUNameString() const	{return (LPCTSTR)m_sCPUNameString;};
	LPCTSTR GetCPUVendorIdentifier() const {return (LPCTSTR)m_sCPUVendorIdentifier;};
	LPCTSTR GetCPUIdentifier() const	{return (LPCTSTR)m_sCPUIdentifier;};
	DWORD GetCPUSpeed() const		{return m_dwCPUSpeed;};
	DWORD GetNumProcessors() const	{return m_dwNumProcessors;};

	DWORD GetProcessMemoryUsage();
	DWORD GetMemoryLoad() const		{return m_dwMemoryLoad;};
	DWORD64 GetAvailVirtual() const {return m_dwAvailVirtual;};
	DWORD64 GetTotalVirtual() const {return m_dwTotalVirtual;};
	DWORD64 GetAvailPageFile() const{return m_dwAvailPageFile;};
	DWORD64 GetTotalPageFile() const{return m_dwTotalPageFile;};
	DWORD64 GetAvailPhys() const	{return m_dwAvailPhys;};
	DWORD64 GetTotalPhys() const	{return m_dwTotalPhys;};

	DWORD GetWinMajor()	const		{return m_dwWinMajor;};
	DWORD GetWinMinor() const		{return m_dwWinMinor;};
	DWORD GetBuildNumber() const	{return m_dwBuildNumber;};
	DWORD GetServicePack() const	{return m_dwServicePack;};
	LPCTSTR GetOSType() const		{return (LPCTSTR)m_sOSType;};

	void Init();
	CSysInfo();
	virtual ~CSysInfo();

protected:
	// Misc variables
	CString m_sWorkingDir;
	CString m_sUserName;
	BOOL m_bUserAdmin;

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

	void DetectWorkingDir();
	void DetectLoginUserName();
	void DetectUserAdmin();
	void DetectCPUType();
	void DetectMemory();
	void DetectNumProcessors();
	void DetectOSType();
};