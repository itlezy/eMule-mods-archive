#pragma once
#include "map_inc.h"

struct VolumeInfo{
	VolumeInfo():Name(NULL), FileSystem(NULL), Flags(0), FreeSpace((uint64)-1){}
	LPTSTR Name;
	LPTSTR FileSystem;
	uint64 FreeSpace;
	DWORD Flags;
	//bool IsSupportReparsePoints() const{return (Flags & FILE_SUPPORTS_REPARSE_POINTS) != 0;};
	bool IsSupportSparseFiles() const{return (Flags & FILE_SUPPORTS_SPARSE_FILES) != 0;};
	bool IsSupportNamedStream() const{return (Flags & FILE_NAMED_STREAMS) != 0;};
	bool IsFAT() const{return _tcsnicmp(FileSystem, _T("FAT"), 3) == 0;};
	bool IsNTFS() const{return _tcscmp(FileSystem, _T("NTFS")) == 0;};
};

struct PathAndVolume 
{
	LPTSTR realpath;
	VolumeInfo*volume;
};

typedef BOOL (WINAPI *t_GetVolumePathNamesForVolumeName)(LPCWSTR, LPWCH, DWORD, PDWORD);

class CVolumeInfo
{
public:
	CVolumeInfo();
	~CVolumeInfo();
	VolumeInfo* GetVolumeInfoByPath(LPCTSTR pszFilePath);
	LPCTSTR GetRealPath(LPCTSTR pszFilePath);
	void ClearCache(/*size_t iDrive*/);
	void ResetFreeDiskSpace();
	//uint64 GetFreeTempSpace();
	VolumeInfo* GetFreeDiskSpace(LPCTSTR pDirectory);
	static uint64 _GetFreeDiskSpace(LPCTSTR pDirectory);
protected:
	static void GetPrivilege();
	bool _GetRealPath(LPTSTR pszFilePath, LPTSTR pszNewPath, size_t dwLen, size_t count = 0) const;
	LPTSTR _GetVolumeName(LPCTSTR pszFilePath) const;
	PathAndVolume& GetPathAndVolume(LPCTSTR pszFilePath);
	//void GetRootDir(LPTSTR pszFilePath);
	static t_GetVolumePathNamesForVolumeName _pfnGetVolumePathNamesForVolumeName;
	MAP<LPTSTR, VolumeInfo*, LPTSTR_Pred> m_mapVolumeInfo;
	MAP<LPTSTR, PathAndVolume, LPTSTR_Pred> m_mapPath;
	static bool hasPrivilege;
};
extern CVolumeInfo g_VolumeInfo;
