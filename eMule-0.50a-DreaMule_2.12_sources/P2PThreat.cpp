/** @file		P2PThreat.cpp
 *  @brief	Detect bad apps (virus/worms) that could threaten file sharing
 *  @author	netfinity
 */

#include "StdAfx.h"
#include "emule.h"
#include "OtherFunctions.h"
#include "P2PThreat.h"
 
CP2PThreat	theP2PThreat;
 
/**
 *  @brief	Internal list of bad software
 */
static struct Threat
{
	TCHAR*		threat_name;
	TCHAR*		file_name;
	TCHAR*		install_path;
	TCHAR*		run_registry_key_name;
	TCHAR*		run_registry_key_value;
	TCHAR*		created_dir;
} Threats[] = {
	{_T("W32.Tibick Worm"), _T("svcnet.exe"), _T("%System%"), _T("System Restore"), _T("svcnet.exe"), _T("%Windir%\\msview")},
	{_T("W32.Tibick Worm"), _T("svcnet.exe"), _T("%System%"), _T("shellapi32"), _T("svcnet"), _T("%Windir%\\msview")},
	{_T("W32.HLLW.Reur Worm"), NULL, _T("%System%"), _T("########"), _T("%Sysdir%\\########.exe"), NULL},
	{_T("W32.HLLW.Cayam@mm Worm"), _T("Msfind32.exe"), _T("%Windir%"), _T("MSFind32"), _T("c:\\windows\\msfind32.exe"), NULL},
	{_T("W32.Saros@mm Worm"), _T("NonYou.exe"), _T("%System%"), _T("nldr32\\default"), _T("WINDOWS\\system32\\NonYou.exe"), NULL},
	{_T("W32.Netsky.S@mm Worm"), _T("EasyAV.exe"), _T("%Windir%"), _T("EasyAV"), _T("%Windir%\\EasyAV.exe"), NULL},
	{_T("W32.Netsky.T@mm Worm"), _T("EasyAV.exe"), _T("%Windir%"), _T("EasyAV"), _T("%Windir%\\EasyAV.exe"), NULL},
	{_T("W32.Netsky.U@mm Worm"), _T("SymAV.exe"), _T("%Windir%"), _T("SymAV"), _T("%Windir%\\SymAV.exe"), NULL},
	{_T("W32.Netsky.V@mm Worm"), _T("Kasperskyaveng.exe"), _T("%Windir%"), _T("KasperskyAVEng"), _T("%Windir%\\Kasperskyaveng.exe"), NULL},
	{_T("W32.Netsky.Q@mm Worm"), _T("SysMonXP.exe"), _T("%Windir%"), _T("SysMonXP"), _T("%Windir%\\SysMonXP.exe"), NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL}
};

/**
 *  @brief	Detect if a bad app (virus/worm), is present, that could threaten file sharing
 */
bool
CP2PThreat::IsMachineInfected()
{
	TCHAR	buffer[MAX_PATH];
	TCHAR	system_dir[MAX_PATH];
	TCHAR	windows_dir[MAX_PATH];
	bool	bInfected = false;
	//FILE*	fout;
	//fout = fopen("P2PThreat_debug.txt","wb");
	// Get system paths
	GetSystemDirectory(system_dir,MAX_PATH);
	GetWindowsDirectory(windows_dir,MAX_PATH);
	// Save path
	GetCurrentDirectory(MAX_PATH, buffer);
	int i;
	for (i = 0; Threats[i].threat_name != NULL; ++i)
	{
		//fprintf(fout,"%d:\n",i);
		bInfected = true;
		// Check if bad file is present on disk
		if (Threats[i].file_name != NULL && Threats[i].install_path != NULL)
		{
			CString	install_path = Threats[i].install_path;
			//install_path.Preallocate(MAX_PATH);
			//TCHAR install_path[MAX_PATH];
			//PathUnExpandEnvStringsW(Threats[i].install_path, install_path.GetBuffer(), MAX_PATH);
			//install_path.ReleaseBuffer();
			install_path.Replace(_T("%System%"), system_dir);
			install_path.Replace(_T("%Windir%"), windows_dir);
			if (SetCurrentDirectory(install_path))
			{
				HANDLE hFile; 
 
				hFile = CreateFile(Threats[i].file_name,    // file to open
						GENERIC_READ,          // open for reading
						FILE_SHARE_READ,       // share for reading
						NULL,                  // default security
						OPEN_EXISTING,         // existing file only
						FILE_ATTRIBUTE_NORMAL, // normal file
						NULL);                 // no attr. template
 
				if (hFile == INVALID_HANDLE_VALUE)
				{
					if (GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						bInfected = false;
						//fprintf(fout,"No suspicious file detected!\n");
					}
				}
				else
					CloseHandle(hFile);
			}
			else
			{
				bInfected = false;
				//fprintf(fout,"No suspicious file detected!\n");
			}
		}
		// Check if bad folder is present on disk
		if (Threats[i].created_dir != NULL)
		{
			CString created_dir = Threats[i].created_dir;
			//created_dir.Preallocate(MAX_PATH);
			//TCHAR created_dir[MAX_PATH];
			//PathUnExpandEnvStrings(Threats[i].created_dir, created_dir.GetBuffer(), MAX_PATH);
			//created_dir.ReleaseBuffer();
			created_dir.Replace(_T("%System%"), system_dir);
			created_dir.Replace(_T("%Windir%"), windows_dir);
			if (!SetCurrentDirectory(created_dir))
			{
				bInfected = false;
				//fprintf(fout,"No suspicious directory detected!\n");
			}
		}
		// Check if bad registry key is present
		if (Threats[i].run_registry_key_name != NULL)
		{
			if (!TestRegistryKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), Threats[i].run_registry_key_name, Threats[i].run_registry_key_value))
			{
				if (!TestRegistryKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), Threats[i].run_registry_key_name, Threats[i].run_registry_key_value))
				{
					bInfected = false;
					//fprintf(fout,"Registry is clean!\n");
				}
			}
		}
		// Is the system infected?
		if (bInfected)
			break;
	}
	//fclose(fout);
	// Restore path
	SetCurrentDirectory(buffer);
	// Notify user about the infection!
	if (bInfected)
	{
		CString	notification;
		CString	caption;
		notification.Format(
			_T("WARNING!!! Your computer has been compromised.\n")
			_T("The '%s' was detected on your system.\n\n")
			_T("Please, update your antivirus software and restart eMule."),
			Threats[i].threat_name
			);
		caption.Format(_T("eMule %s"), theApp.m_strCurVersionLong); //Xman
		MessageBoxW(NULL, notification, caption, MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
	}
	return bInfected;
}

bool 
CP2PThreat::TestRegistryKey(HKEY hParent, LPCTSTR pszKey, LPCTSTR pszName, LPCTSTR pszValue)
{
	bool bFound = false;
	CRegKey regkey;
	if (regkey.Open(hParent, pszKey) == ERROR_SUCCESS)
	{
		TCHAR pszTempName[500], pszTempValue[500];
		DWORD dwNameLen, dwValueLen, idx = 0;
		if (_tcscmp(pszName,_T("*"))==0) //(pszName == _T("*"))//Xman
		{
			while (!bFound)
			{
				dwNameLen = ARRSIZE(pszTempName);
				dwValueLen = ARRSIZE(pszTempValue);
				if (regkey.EnumKey(idx++, pszTempName, &dwNameLen) != ERROR_SUCCESS)
					break;
				regkey.QueryStringValue(pszTempName, pszTempValue, &dwValueLen);
				CString strValue = pszTempValue;
				strValue.Replace(pszTempName, _T("*"));
	
				if (_tcscmp(pszTempValue, pszValue) == 0)
					bFound = true;
			}
		}
		else if(_tcscmp(pszName,_T("########"))==0) //(pszName == _T("########"))//Xman
		{
			while (!bFound)
			{
				dwNameLen = ARRSIZE(pszTempName);
				dwValueLen = ARRSIZE(pszTempValue);
				if (regkey.EnumKey(idx++, pszTempName, &dwNameLen) != ERROR_SUCCESS)
					break;
				regkey.QueryStringValue(pszTempName, pszTempValue, &dwValueLen);
				if (_tcslen(pszTempName) == 8)
				{
					bool bIsHex = true;
					for (int i = 0; i < 8; ++i)
					{
						if(!IsHexDigit(pszTempName[i]))
							bIsHex = false;
					}
					CString strValue = pszTempValue;
					strValue.Replace(pszTempName, _T("########"));
	
					if (bIsHex && _tcscmp(pszTempValue, pszValue) == 0)
						bFound = true;
				}
			}
		}
		else
		{
			dwValueLen = ARRSIZE(pszTempValue);
			regkey.QueryStringValue(pszName, pszTempValue, &dwValueLen);

			if (_tcscmp(pszTempValue, pszValue) == 0)
				bFound = true;
		}

		regkey.Close();
	}
	return bFound;
}