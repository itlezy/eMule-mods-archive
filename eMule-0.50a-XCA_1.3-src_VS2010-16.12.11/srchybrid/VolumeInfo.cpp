#include "stdafx.h"
#include "VolumeInfo.h"
#include <winioctl.h>
#ifdef _DEBUG
#include "log.h"
#endif
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CVolumeInfo g_VolumeInfo;
t_GetVolumePathNamesForVolumeName CVolumeInfo::_pfnGetVolumePathNamesForVolumeName;
bool CVolumeInfo::hasPrivilege = false;

typedef struct _REPARSE_DATA_BUFFER {
	DWORD ReparseTag;
	USHORT  ReparseDataLength;
	USHORT  Reserved;
	USHORT  SubstituteNameOffset;
	USHORT  SubstituteNameLength;
	USHORT  PrintNameOffset;
	USHORT  PrintNameLength;
	union {
		struct {
			ULONG  Flags;
			WCHAR  ReparseTarget[1];
		} SymbolicLinkReparseBuffer;
		struct {
			WCHAR ReparseTarget[1];
		} MountPointReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

CVolumeInfo::CVolumeInfo(){
	GetPrivilege();
	(FARPROC&)_pfnGetVolumePathNamesForVolumeName = GetProcAddress(GetModuleHandle(_T("kernel32")), _TWINAPI("GetVolumePathNamesForVolumeNameW"));
}

void CVolumeInfo::GetPrivilege(){
	// Obtain backup/restore privilege in case we don't have it
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	if(!OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return;
	if(!LookupPrivilegeValue(NULL, SE_BACKUP_NAME,&tp.Privileges[0].Luid)){
		CloseHandle(hToken);
		return;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if(AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
		hasPrivilege = true;
	CloseHandle(hToken);
}
/*
void CVolumeInfo::GetRootDir(LPTSTR pszFilePath){
	size_t len = _tcslen(pszFilePath);
	if(len > 2)
		VERIFY(PathStripToRoot(pszFilePath));
	// Need to add a trailing backslash in case of a network share
	if (len != 0 && pszFilePath[len - 1] != _T('\\')) {
		pszFilePath[len] = _T('\\');
		pszFilePath[len + 1] = 0;
	}
}
*/

LPTSTR CVolumeInfo::_GetVolumeName(LPCTSTR pszFilePath) const{
	LPCTSTR newpath = pszFilePath;
	LPTSTR volname = NULL;
	if(newpath[2] == _T('?')){// not mounted partition
#define GUIDVOLLEN 49
		ASSERT(_tcslen(newpath) == GUIDVOLLEN);
		volname = new TCHAR[GUIDVOLLEN + 1];
		_tcsncpy(volname, newpath, GUIDVOLLEN);
		volname[GUIDVOLLEN] = 0;
	}
	else{
		size_t len = _tcslen(pszFilePath);
		bool addBackslash = false;
		if(len == 2){
			addBackslash = true;
			++len;
		}
		else if(len > 2 && pszFilePath[0] == _T('\\')){// Need to add a trailing backslash in case of a network share
			if(pszFilePath[len - 1] != _T('\\')){
				addBackslash = true;
				++len;
			}
		}
		else
			len = 3;

		volname = new TCHAR[len+1];
		_tcsncpy(volname, pszFilePath, len);
		if(addBackslash)
			volname[len - 1] = _T('\\');
		volname[len] = 0;
	}
	return volname;
}

LPCTSTR CVolumeInfo::GetRealPath(LPCTSTR pszFilePath){
	return GetPathAndVolume(pszFilePath).realpath;
}

VolumeInfo* CVolumeInfo::GetVolumeInfoByPath(LPCTSTR pszFilePath)
{
	PathAndVolume&pnv = GetPathAndVolume(pszFilePath);
	if(pnv.volume != NULL)
		return pnv.volume;
	LPTSTR volname = _GetVolumeName(pnv.realpath);
	MAP<LPTSTR, VolumeInfo*, LPTSTR_Pred>::const_iterator it = m_mapVolumeInfo.find(volname);
	if(it != m_mapVolumeInfo.end()){
		delete [] volname;
		pnv.volume = it->second;
		return it->second;
	}

	// 'GetVolumeInformation' may cause a noticable delay - depending on the type of volume 
	// which is queried. As we are using that function for almost every file (for compensating
	// the NTFS file time issues), we need to cash this information.
	//
	// The cash gets cleared when the user manually hits the 'Reload' button in the 'Shared
	// Files' window and when Windows broadcasts a message about that a volume was mounted/unmounted.
	//
	DWORD dwMaximumComponentLength = 0;
	TCHAR szFileSystemNameBuffer[MAX_PATH + 1];
	VolumeInfo*vi = new VolumeInfo();
	vi->Name = volname;
	if (!GetVolumeInformation(volname, NULL, 0, NULL, &dwMaximumComponentLength, &vi->Flags, szFileSystemNameBuffer, _countof(szFileSystemNameBuffer)))
		vi->FileSystem = NULL;
	else
		vi->FileSystem = _tcsdup(szFileSystemNameBuffer);
	pnv.volume = vi;
	return m_mapVolumeInfo[vi->Name] = vi;
}

PathAndVolume& CVolumeInfo::GetPathAndVolume(LPCTSTR pszFilePath){
	TCHAR oldPath[MAX_PATH + 1];
	_tcsncpy(oldPath, pszFilePath, MAX_PATH);
	oldPath[_countof(oldPath) - 1] = 0;
	_tcslwr(oldPath);
	TCHAR* lastchar = oldPath +_tcslen(oldPath) - 1;
	if(*lastchar == _T('\\'))
		*lastchar = 0;
	if(!PathIsDirectory(pszFilePath)){
		lastchar = _tcsrchr(oldPath,_T('\\'));
		ASSERT(lastchar != NULL);
		*lastchar = 0;
	}
	MAP<LPTSTR, PathAndVolume, LPTSTR_Pred>::iterator it = m_mapPath.find(oldPath);
	if(it != m_mapPath.end())
		return it->second;
	TCHAR*poldPath = _tcsdup(oldPath);
	PathAndVolume pnv;
	pnv.volume = NULL;
	if(hasPrivilege && !PathIsUNC(pszFilePath)){
		TCHAR newPath[MAX_PATH + 1];
		newPath[0] = 0;
		if(_GetRealPath(oldPath, newPath, MAX_PATH + 1)){
			TCHAR*pnewpath = _tcsdup(newPath);
			pnv.realpath = pnewpath;
#ifdef _DEBUG
			AddLogLine(false, _T("CVolumeInfo::GetRealPath %s -> %s"), poldPath, pnewpath);
#endif
			return m_mapPath[poldPath] = pnv;
		}
	}
	pnv.realpath = poldPath;
	return m_mapPath[poldPath] = pnv;
}

void CVolumeInfo::ResetFreeDiskSpace(){
	for (MAP<LPTSTR, VolumeInfo*, LPTSTR_Pred>::const_iterator it = m_mapVolumeInfo.begin(); it != m_mapVolumeInfo.end(); ++it)
		it->second->FreeSpace = (uint64)-1;
}

uint64 CVolumeInfo::GetFreeTempSpace(){
	ResetFreeDiskSpace();
	for (size_t i=0;i<thePrefs.tempdir.GetCount();i++)
		GetFreeDiskSpace(thePrefs.GetTempDir(i));
	uint64 totalspace = 0;
	for (MAP<LPTSTR, VolumeInfo*, LPTSTR_Pred>::const_iterator it = m_mapVolumeInfo.begin(); it!= m_mapVolumeInfo.end(); ++it){
		uint64 curspace = it->second->FreeSpace;
		if(curspace != (uint64)-1)
			totalspace += curspace;
	}
	return totalspace;
}

VolumeInfo* CVolumeInfo::GetFreeDiskSpace(LPCTSTR pDirectory){
	VolumeInfo*vi = GetVolumeInfoByPath(pDirectory);
	if(vi->FreeSpace == (uint64)-1)
		vi->FreeSpace =  _GetFreeDiskSpace(pDirectory);
	return vi;
}

uint64 CVolumeInfo::_GetFreeDiskSpace(LPCTSTR pDirectory){
	ULARGE_INTEGER nFreeDiskSpace;
	ULARGE_INTEGER dummy;
	if ( GetDiskFreeSpaceEx(pDirectory, &nFreeDiskSpace, &dummy, &dummy)==TRUE)
		return nFreeDiskSpace.QuadPart;
	ASSERT(0);
	return 0;
	/*}
	else 
	{
		TCHAR cDrive[MAX_PATH];
		const TCHAR *p = _tcschr(pDirectory, _T('\\'));
		if (p)
		{
			size_t uChars = p - pDirectory;
			if (uChars >= _countof(cDrive))
				return 0;
			memcpy(cDrive, pDirectory, uChars * sizeof(TCHAR));
			cDrive[uChars] = _T('\0');
		}
		else
		{
			if (_tcslen(pDirectory) >= _countof(cDrive))
				return 0;
			_tcscpy_s(cDrive, pDirectory);
		}
		DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwDummy;
		if (GetDiskFreeSpace(cDrive, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwDummy))
			return (ULONGLONG)dwFreeClusters * (ULONGLONG)dwSectPerClust * (ULONGLONG)dwBytesPerSect;
		return 0;
	}*/
}

bool CVolumeInfo::_GetRealPath(LPTSTR pszFilePath, LPTSTR pszNewPath, size_t dwLen, size_t count) const{
	if(count > 3)
		return false;
	DWORD dwAttr = GetFileAttributes(pszFilePath);
	bool ret = false;
	if((dwAttr != -1) &&
		(dwAttr & FILE_ATTRIBUTE_DIRECTORY) &&
		(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)){
		HANDLE hDir = CreateFile(pszFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
		char buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
		DWORD dwRet=0;
		if (hDir != INVALID_HANDLE_VALUE && DeviceIoControl(hDir, FSCTL_GET_REPARSE_POINT, NULL, 0, &buf, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwRet, NULL)){
			CloseHandle(hDir);
			REPARSE_DATA_BUFFER*pbuf=(REPARSE_DATA_BUFFER*)&buf;
			if(pbuf->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT){
				if(_tcsnicmp(pbuf->MountPointReparseBuffer.ReparseTarget, _T("\\??\\Volume"), 10)){
					//"\??\X:\"
					ASSERT(pbuf->MountPointReparseBuffer.ReparseTarget[5] == _T(':'));
					_tcsncpy(pszNewPath, pbuf->MountPointReparseBuffer.ReparseTarget+4, dwLen - 1);
					pszNewPath[dwLen - 1] = 0;
					_GetRealPath(pszNewPath, pszNewPath, dwLen, count+1);
					_tcslwr(pszNewPath);
					return true;
				}
				//"\??\Volume{GUID}\"
				pbuf->MountPointReparseBuffer.ReparseTarget[1] = _T('\\');
				if(_pfnGetVolumePathNamesForVolumeName && _pfnGetVolumePathNamesForVolumeName(pbuf->MountPointReparseBuffer.ReparseTarget, pszNewPath, dwLen, NULL)){
					if(_tcslen(pszNewPath) <= 3){
						ASSERT(pszNewPath[0] >= _T('A') && pszNewPath[0] <= _T('Z'));
						pszNewPath[0] += _T('a') - _T('A');
						pszNewPath[2] = 0;
					}
				}
				_tcsncpy(pszNewPath, pbuf->MountPointReparseBuffer.ReparseTarget, dwLen - 1);
				pszNewPath[dwLen - 1] = 0;
				return true;
			}
			else if(pbuf->ReparseTag == IO_REPARSE_TAG_SYMLINK){
				//"X:\Path\??\X:\Path"
				LPCTSTR pathend = _tcsstr(pbuf->SymbolicLinkReparseBuffer.ReparseTarget, _T("\\??\\"));
				if(pathend){
					ASSERT(pbuf->SymbolicLinkReparseBuffer.ReparseTarget[1] == _T(':'));
					if(pbuf->SymbolicLinkReparseBuffer.ReparseTarget + dwLen > pathend + 1)
						dwLen = pathend - pbuf->SymbolicLinkReparseBuffer.ReparseTarget + 1;
					_tcsncpy(pszNewPath, pbuf->SymbolicLinkReparseBuffer.ReparseTarget, dwLen - 1 );
					pszNewPath[dwLen - 1] = 0;
					_GetRealPath(pszNewPath, pszNewPath, dwLen, count+1);
					_tcslwr(pszNewPath);
					return true;
				}

			}
		}
	}
	TCHAR* pdirname = _tcsrchr(pszFilePath+2,_T('\\'));
	if(pdirname){
		TCHAR* dirname = _tcsdup(pdirname);
		*pdirname = 0;
		ret |= _GetRealPath(pszFilePath, pszNewPath, dwLen, count);
		size_t len = _tcslen(pszNewPath);
		if(len + _tcslen(dirname) + 1>dwLen){
			free(dirname);
			return false;
		}
		_tcscpy(pszNewPath+len, dirname);
		free(dirname);
	}
	else
		_tcscpy(pszNewPath, pszFilePath);
	return ret;
}

void CVolumeInfo::ClearCache(size_t iDrive)
{
	if (iDrive == (size_t)-1){
		for(MAP<LPTSTR, VolumeInfo*, LPTSTR_Pred>::const_iterator it=m_mapVolumeInfo.begin();it!=m_mapVolumeInfo.end();++it )
		{
			delete [] it->first;
			free(it->second->FileSystem);
			free(it->second);
		}
		m_mapVolumeInfo.clear();
		for(MAP<LPTSTR, PathAndVolume, LPTSTR_Pred>::const_iterator it=m_mapPath.begin();it!=m_mapPath.end();++it )
		{
			if(it->first != it->second.realpath)
				free(it->second.realpath);
			free(it->first);
		}
		m_mapPath.clear();
	}
	else
	{
		TCHAR szRoot[MAX_PATH];
		szRoot[0] = 0;
		PathBuildRoot(szRoot, iDrive);
		if (szRoot[0] != 0)
		{
			ASSERT(szRoot[_tcslen(szRoot) - 1] == _T('\\') );
			_tcslwr(szRoot);
			MAP<LPTSTR, VolumeInfo*, LPTSTR_Pred>::iterator it = m_mapVolumeInfo.find(szRoot);
			if(it != m_mapVolumeInfo.end()){
				delete [] it->first;
				free(it->second->FileSystem);
				free(it->second);
				m_mapVolumeInfo.erase(it);
			}
		}
	}
}

CVolumeInfo::~CVolumeInfo(){
	ClearCache((size_t)-1);
}
