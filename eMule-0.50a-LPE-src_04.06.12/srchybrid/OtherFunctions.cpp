//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include <sys/stat.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#if _MSC_VER>=1400
#pragma warning(disable:6385 6011)
#endif
#include <atlrx.h>
#if _MSC_VER>=1400
#pragma warning(default:6385 6011)
#endif
#include "emule.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "Preferences.h"
#include "PartFile.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "UpDownClient.h"
#include <shlobj.h>
#include "emuledlg.h"
#include "MenuCmds.h"
#include <atlbase.h>
#include "StringConversion.h"
#include "shahashset.h"
#include "collection.h"
#include "SafeFile.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "Log.h"
//#include "CxImage/xImage.h"
#include "VolumeInfo.h" // X: [FSFS] - [FileSystemFeaturesSupport]
#include <Iphlpapi.h>
#include "DesktopIntegration.h"	// netfinity: Better support for WINE Gnome/KDE desktops

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Base chars for encode an decode functions
static const TCHAR _acHexDigits[] = _T("0123456789ABCDEF");
static byte base32Chars[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
#define BASE16_LOOKUP_MAX 23
static byte base16Lookup[BASE16_LOOKUP_MAX][2] = {
    { '0', 0x0 },
    { '1', 0x1 },
    { '2', 0x2 },
    { '3', 0x3 },
    { '4', 0x4 },
    { '5', 0x5 },
    { '6', 0x6 },
    { '7', 0x7 },
    { '8', 0x8 },
    { '9', 0x9 },
	{ ':', 0x9 },
    { ';', 0x9 },
    { '<', 0x9 },
    { '=', 0x9 },
    { '>', 0x9 },
    { '?', 0x9 },
    { '@', 0x9 },
    { 'A', 0xA },
    { 'B', 0xB },
    { 'C', 0xC },
    { 'D', 0xD },
    { 'E', 0xE },
    { 'F', 0xF }
};

CString CastItoXBytes(uint16 count, bool isK, bool isPerSec, uint32 decimal){
	return CastItoXBytes((double)count, isK, isPerSec, decimal);
}

CString CastItoXBytes(uint32 count, bool isK, bool isPerSec, uint32 decimal){
	return CastItoXBytes((double)count, isK, isPerSec, decimal);
}

CString CastItoXBytes(uint64 count, bool isK, bool isPerSec, uint32 decimal){
	return CastItoXBytes((double)count, isK, isPerSec, decimal);
}

#if defined(_DEBUG) && defined(USE_DEBUG_EMFILESIZE)
CString CastItoXBytes(EMFileSize count, bool isK, bool isPerSec, uint32 decimal){
	return CastItoXBytes((double)count, isK, isPerSec, decimal);
}
#endif

CString CastItoXBytes(float count, bool isK, bool isPerSec, uint32 decimal){
	return CastItoXBytes((double)count, isK, isPerSec, decimal);
}

CString CastItoXBytes(double count, bool isK, bool isPerSec, uint32 decimal){
	if( count <= 0.0 )
		return (isPerSec)?_T("0 B/s"):_T("0 B");
	if( isK ){
		if( count >  1.7E+300 )
			count =  1.7E+300;
		else
			count *= 1024.0;
	}
	CString buffer;
	if( isPerSec )
	{
		//Xman Xtreme Mod
		//use other system if decimal=99 (standard)
		if(decimal==99)
		{
			if (count < 1024000.0)
				buffer.Format(_T("%.1f KB/s"), count/1024.0);
			else if (count < 1048576000.0)
				buffer.Format(_T("%.2f MB/s"), count/1048576.0);
			else
				buffer.Format(_T("%.3f GB/s"), count/1073741824.0);
		}
		else
		{
			if (count < 1024.0)
				buffer.Format(_T("%.0f B/s"), count);
			else if (count < 1024000.0)
				buffer.Format(_T("%.*f KB/s"), decimal, count/1024.0);
			else if (count < 1048576000.0)
				buffer.Format(_T("%.*f MB/s"), decimal, count/1048576.0);
			else if (count < 1073741824000.0)
				buffer.Format(_T("%.*f GB/s"), decimal, count/1073741824.0);
			else 
				buffer.Format(_T("%.*f TB/s"), decimal, count/1099511627776.0);
			//Xman end
		}
	}
	else
	{
		//Xman Xtreme Mod
		//use other system if decimal=99 (standard)
		if(decimal==99)
		{
			if (count < 1024)
				buffer.Format(_T("%.0f B"),count);
			else if (count < 1048576)
				buffer.Format(_T("%.1f KB"),count/1024.0f);
			else if (count < 1073741824)
				buffer.Format(_T("%.2f MB"),count/1048576.0f);
			else if (count < 1099511627776)
				buffer.Format(_T("%.2f GB"),count/1073741824.0f);
			else 
				buffer.Format(_T("%.3f TB"),count/1099511627776.0f);
		}
		else
		{
			if (count < 1024.0)
				buffer.Format(_T("%.0f B"), count);
			else if (count < 1024000.0)
				buffer.Format(_T("%.*f KB"), decimal, count/1024.0);
			else if (count < 1048576000.0)
				buffer.Format(_T("%.*f MB"), decimal, count/1048576.0);
			else if (count < 1073741824000.0)
				buffer.Format(_T("%.*f GB"), decimal, count/1073741824.0);
			else 
				buffer.Format(_T("%.*f TB"), decimal, count/1099511627776.0);
		}
		//Xman end
	}
	return buffer;
}

CString CastItoIShort(uint16 count, bool isK, uint32 decimal){
	return CastItoIShort((double)count, isK, decimal);
}

CString CastItoIShort(uint32 count, bool isK, uint32 decimal){
	return CastItoIShort((double)count, isK, decimal);
}

CString CastItoIShort(uint64 count, bool isK, uint32 decimal){
	return CastItoIShort((double)count, isK, decimal);
}

CString CastItoIShort(float count, bool isK, uint32 decimal){
	return CastItoIShort((double)count, isK, decimal);
}

CString CastItoIShort(double count, bool isK, uint32 decimal){
	if( count <= 0.0 )
	{
		return _CString(_T("0"));
	}
	else if( isK )
	{
		if( count >  1.7E+300 )
			count =  1.7E+300;
		else
			count *= 1000.0;
	}
	CString output;
	if (count < 1000.0)
		output.Format(_T("%.0f"), count);
	else if (count < 1000000.0)
		output.Format(_T("%.*f K"), decimal, count/1000.0);
	else if (count < 1000000000.0)
		output.Format(_T("%.*f M"), decimal, count/1000000.0);
	else if (count < 1000000000000.0)
		output.Format(_T("%.*f G"), decimal, count/1000000000.0);
	else if (count < 1000000000000000.0)
		output.Format(_T("%.*f T"), decimal, count/1000000000000.0);
	return output;
}

CString CastSecondsToHM(uint_ptr tSeconds)
{
	if (tSeconds == (uint_ptr)-1)	// invalid or unknown time value
		return _CString(_T("?"));

	CString buffer;
	uint_ptr count = tSeconds;
	uint_ptr cntA, cntB;
	if (count < 60){
		cntA = count;
		buffer.Format(_T("%u %s"), cntA, GetResString(IDS_SECS));
	}
	else if (count < 3600){
		cntA = count/60;
		cntB = count - cntA*60;
		buffer.Format(_T("%u:%02u %s"), cntA, cntB, GetResString(IDS_MINS));
	}
	else if (count < 86400){
		cntA = count/3600;
		cntB = (count - cntA*3600)/60;
		buffer.Format(_T("%u:%02u %s"), cntA, cntB, GetResString(IDS_HOURS));
	}
	else {
		cntA = count/86400;
		cntB = (count - cntA*86400)/3600;
		buffer.Format(_T("%u %s %u %s"), cntA, GetResString(IDS_DAYS), cntB, GetResString(IDS_HOURS));
	}
	return buffer;
}

CString CastSecondsToLngHM(uint_ptr tSeconds)
{
	if (tSeconds == (uint_ptr)-1) // invalid or unknown time value
		return _CString(_T("?"));

	CString buffer;
	uint_ptr count = tSeconds;
	uint_ptr cntA, cntB;
	if (count < 60){
		cntA = count;
		buffer.Format(_T("%u %s"), cntA, GetResString(IDS_LONGSECS));
	}
	else if (count < 3600){
		cntA = count/60;
		cntB = count - cntA*60;
		buffer.Format(_T("%u:%02u %s"), cntA, cntB, GetResString(IDS_LONGMINS));
	}
	else if (count < 86400){
		cntA = count/3600;
		cntB = (count - cntA*3600)/60;
		buffer.Format(_T("%u:%02u %s"), cntA, cntB, GetResString(IDS_LONGHRS));
	}
	else {
		cntA = count/86400;
		cntB = (count - cntA*86400)/3600;
		uint_ptr cntC = (count - (cntA*86400) - (cntB*3600))/60;
		if (cntB)
			buffer.Format(_T("%u %s %u:%02u %s"), cntA, GetResString(IDS_DAYS2), cntB, cntC, GetResString(IDS_LONGHRS));
		else
			buffer.Format(_T("%u %s %u %s"), cntA, GetResString(IDS_DAYS2), cntC, GetResString(IDS_LONGMINS));
	}
	return buffer;
} 

void ShellOpenFile(CString name)
{
    ShellOpenFile(name, _T("open"));
}

void ShellOpenFile(CString name, LPCTSTR pszVerb)
{
    //ShellExecute(NULL, pszVerb, name, NULL, NULL, SW_SHOW);
	theDesktop.ShellExecute(pszVerb, name); // netfinity: Better support for WINE Gnome/KDE desktops
}

bool ShellDeleteFile(LPCTSTR pszFilePath)
{
	if (!PathFileExists(pszFilePath))
		return true;
	if (!thePrefs.GetRemoveToBin())
		return (DeleteFile(pszFilePath) != FALSE);

	TCHAR todel[MAX_PATH + 1];
	memset(todel, 0, sizeof todel);
	_tcsncpy(todel, pszFilePath, _countof(todel) - 2);

	SHFILEOPSTRUCT fp = {0};
	fp.wFunc = FO_DELETE;
	fp.hwnd = theApp.emuledlg->m_hWnd;
	fp.pFrom = todel;
	fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NORECURSION;
	bool bResult = false;
	__try {
		// Win98/MSLU: This seems to crash always in UNICOWS.DLL !?
		bResult = (SHFileOperation(&fp) == 0);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		bResult = (DeleteFile(pszFilePath) != FALSE);
	}
	return bResult;
}

CString ShellGetFolderPath(int iCSIDL)
{
	CString strFolderPath;

		WCHAR wszPath[MAX_PATH];
	VERIFY(SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, wszPath) == S_OK);
			strFolderPath = wszPath;
/*
	if (strFolderPath.IsEmpty())
	{
		HMODULE hLibShFolder = LoadLibrary(_T("shfolder.dll"));
		if (hLibShFolder)
		{
			HRESULT (WINAPI *pfnSHGetFolderPathW)(HWND, int, HANDLE, DWORD, LPWSTR);
			(FARPROC&)pfnSHGetFolderPathW = GetProcAddress(hLibShFolder, "SHGetFolderPathW");
			if (pfnSHGetFolderPathW)
			{
				WCHAR wszPath[MAX_PATH];
				if ((*pfnSHGetFolderPathW)(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, wszPath) == S_OK)
					strFolderPath = wszPath;
			}

			if (strFolderPath.IsEmpty())
			{
				HRESULT (WINAPI *pfnSHGetFolderPathA)(HWND, int, HANDLE, DWORD, LPSTR);
				(FARPROC&)pfnSHGetFolderPathA = GetProcAddress(hLibShFolder, "SHGetFolderPathA");
				if (pfnSHGetFolderPathA)
				{
					CHAR aszPath[MAX_PATH];
					if ((*pfnSHGetFolderPathA)(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, aszPath) == S_OK)
						strFolderPath = CString(aszPath);
				}
			}
			FreeLibrary(hLibShFolder);
		}
	}
*/
	return strFolderPath;
}

bool IsHexDigit(sint_ptr c)
{
	return((c>='0' && c<='9')||(c>='A' && c<='F')||(c>='a' && c<='f'));
}

CString URLDecode(const CString& inStr, bool bKeepNewLine)
{
	// decode escape sequences
	CString res;
	int x = 0;
	for (; x < inStr.GetLength(); x++){
		sint_ptr c1,c2;
		if (inStr.GetAt(x) == _T('%')
			&& x + 2 < inStr.GetLength()
			&& (c1 = charhexval(inStr[x+1])) != -1
			&& (c2 = charhexval(inStr[x+2])) != -1
		){
			// ok, we got a character from this hex number
			TCHAR ch = (TCHAR)((c1<<4) | c2);
			// basic check
			if (ch > '\x1F' || (bKeepNewLine && ch == '\x0A')) // filter control chars
				res.AppendChar(ch);
			x+=2;
		}
		else
		{
			res.AppendChar(inStr.GetAt(x));
		}
	}
	return res;
}

CString RemoveAmbersand(const CString& rstr)
{
	CString str(rstr);
	str.Remove(_T('&'));
	return str;
}

bool Ask4RegFix(bool checkOnly, bool dontAsk, bool bAutoTakeCollections)
{
	// Barry - Make backup first
	if (!checkOnly)
		BackupReg();

	bool bGlobalSet = false;
	CRegKey regkey;
	LONG result;
	TCHAR modbuffer[MAX_PATH];
	DWORD dwModPathLen = ::GetModuleFileName(NULL, modbuffer, _countof(modbuffer));
	if (dwModPathLen == 0 || dwModPathLen == _countof(modbuffer))
		return false;
	CString strCanonFileName = modbuffer;
	strCanonFileName.Replace(_T("%"), _T("%%"));
	CString regbuffer;
	regbuffer.Format(_T("\"%s\" \"%%1\""), strCanonFileName);

	// first check if the registry keys are already set (either by installer in HKLM or by user in HKCU)
	result = regkey.Open(HKEY_CLASSES_ROOT, _T("ed2k\\shell\\open\\command"), KEY_READ);
	if (result == ERROR_SUCCESS)
	{
		TCHAR rbuffer[MAX_PATH + 100];
		ULONG maxsize = _countof(rbuffer);
		regkey.QueryStringValue(NULL, rbuffer, &maxsize);
		rbuffer[_countof(rbuffer) - 1] = _T('\0');
		if (maxsize != 0 && _tcsicmp(rbuffer, regbuffer) == 0)
			bGlobalSet = true; // yup, globally we have an entrie for this mule
		regkey.Close();
	}

	if (!bGlobalSet)
	{
		// we actually need to change the registry and write an entry for HKCU
		if (checkOnly)
			return true;
		//HKEY hkeyCR = thePrefs.GetWindowsVersion() < _WINVER_2K_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
		if (regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\shell\\open\\command")) == ERROR_SUCCESS)
		{
			if (dontAsk || (AfxMessageBox(GetResString(IDS_ASSIGNED2K), MB_ICONQUESTION|MB_YESNO) == IDYES))
			{
				VERIFY( regkey.SetStringValue(NULL, regbuffer) == ERROR_SUCCESS );

				VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\DefaultIcon")) == ERROR_SUCCESS );
				VERIFY( regkey.SetStringValue(NULL, modbuffer) == ERROR_SUCCESS );

				VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k")) == ERROR_SUCCESS );
				VERIFY( regkey.SetStringValue(NULL, _T("URL: ed2k Protocol")) == ERROR_SUCCESS );
				VERIFY( regkey.SetStringValue(_T("URL Protocol"), _T("")) == ERROR_SUCCESS );

				VERIFY( regkey.Open(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\shell\\open")) == ERROR_SUCCESS );
				regkey.RecurseDeleteKey(_T("ddexec"));
				regkey.RecurseDeleteKey(_T("ddeexec"));
			}
			regkey.Close();
		}
		else
			ASSERT(0);
	}
	else if (checkOnly)
		return bAutoTakeCollections && DoCollectionRegFix(true);
	else if (bAutoTakeCollections)
		DoCollectionRegFix(false);

	return false;
}

void BackupReg(void)
{
	// TODO: This function needs to be changed in at least 2 regards
	//	1)	It must follow the rules: reading from HKCR and writing into HKCU. What we are currently doing
	//		is not consistent with the other registry function which are dealing with the same keys.
	//
	//	2)	It behavious quite(!) differently under Win98 due to an obvious change in the Windows API.
	//		WinXP: Reading an non existant value returns 'key not found' error.
	//		Win98: Reading an non existant value returns an empty string (which gets saved and restored by our code).
	//		This means that saving/restoring existant registry keys works completely different in Win98/XP.
	//		Actually it works correctly under Win98 and is broken in WinXP+. Though, did someone notice it all ?

	//HKEY hkeyCR = thePrefs.GetWindowsVersion() < _WINVER_2K_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	// Look for pre-existing old ed2k links
	CRegKey regkey;
	if (regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\shell\\open\\command")) == ERROR_SUCCESS)
	{
		TCHAR rbuffer[MAX_PATH + 100];
		ULONG maxsize = _countof(rbuffer);
		if (regkey.QueryStringValue(_T("OldDefault"), rbuffer, &maxsize) != ERROR_SUCCESS || maxsize == 0)
		{
			maxsize = _countof(rbuffer);
			if (regkey.QueryStringValue(NULL, rbuffer, &maxsize) == ERROR_SUCCESS)
				VERIFY( regkey.SetStringValue(_T("OldDefault"), rbuffer) == ERROR_SUCCESS );

			VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\DefaultIcon")) == ERROR_SUCCESS );
			maxsize = _countof(rbuffer);
			if (regkey.QueryStringValue(NULL, rbuffer, &maxsize) == ERROR_SUCCESS)
				VERIFY( regkey.SetStringValue(_T("OldIcon"), rbuffer) == ERROR_SUCCESS );
		}
		regkey.Close();
	}
	else
		ASSERT(0);
}

// Barry - Restore previous values
void RevertReg(void)
{
	//HKEY hkeyCR = thePrefs.GetWindowsVersion() < _WINVER_2K_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	// restore previous ed2k links before being assigned to emule
	CRegKey regkey;
	if (regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\shell\\open\\command")) == ERROR_SUCCESS)
	{
		TCHAR rbuffer[MAX_PATH + 100];
		ULONG maxsize = _countof(rbuffer);
		if (regkey.QueryStringValue(_T("OldDefault"), rbuffer, &maxsize) == ERROR_SUCCESS)
		{
			VERIFY( regkey.SetStringValue(NULL, rbuffer) == ERROR_SUCCESS );
			VERIFY( regkey.DeleteValue(_T("OldDefault")) == ERROR_SUCCESS );

			VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\ed2k\\DefaultIcon")) == ERROR_SUCCESS );
			maxsize = _countof(rbuffer);
			if (regkey.QueryStringValue(_T("OldIcon"), rbuffer, &maxsize) == ERROR_SUCCESS)
			{
				VERIFY( regkey.SetStringValue(NULL, rbuffer) == ERROR_SUCCESS );
				VERIFY( regkey.DeleteValue(_T("OldIcon")) == ERROR_SUCCESS );
			}
		}
		regkey.Close();
	}
	else
		ASSERT(0);
}

WORD DetectWinVersion()
{
	OSVERSIONINFOEX osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osvi))
			return 0;
	}

	switch (osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
			if (osvi.dwMajorVersion <= 4)
				return 0;
			if (osvi.dwMajorVersion == 5){
				if(osvi.dwMinorVersion == 0)
				return _WINVER_2K_;
				if (osvi.dwMinorVersion == 1)
				return _WINVER_XP_;
				//if (osvi.dwMinorVersion == 2)
				return _WINVER_2003_;
			}
			if (osvi.dwMajorVersion == 6){
				if(osvi.dwMinorVersion == 0)
				return _WINVER_VISTA_;
				if(osvi.dwMinorVersion == 1)
				return _WINVER_7_;
			}
			return _WINVER_7_;

		//case VER_PLATFORM_WIN32_WINDOWS:
			//return 0;
		default:
			return 0;
	}
	
}

sint_ptr IsRunningXPSP2(){
	OSVERSIONINFOEX osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO*)&osvi)) 
			return 0;
	}

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion == 5
		&& osvi.dwMinorVersion == 1 && osvi.wServicePackMajor >= 2)
			return 1;
	return 0;
}

sint_ptr IsRunningXPSP2OrHigher(){
	switch(thePrefs.GetWindowsVersion()){
//		case _WINVER_95_:
//		case _WINVER_98_:
		//case _WINVER_NT4_:
		case _WINVER_2K_:
//		case _WINVER_ME_:
			return 0;
			break;
		case _WINVER_XP_:
			return IsRunningXPSP2();
			break;
		default:
			return 1;
	}
}

// Returns a BASE32 encoded byte array
//
// [In]
//   buffer: Pointer to byte array
//   bufLen: Lenght of buffer array
//
// [Return]
//   CString object with BASE32 encoded byte array
CString EncodeBase32(const unsigned char* buffer, uint_ptr bufLen)
{
	CString Base32Buff;
    
	uint_ptr i, index;
    unsigned char word;

    for(i = 0, index = 0; i < bufLen;) {

		// Is the current word going to span a byte boundary?
        if (index > 3) {
            word = (BYTE)(buffer[i] & (0xFF >> index));
            index = (index + 5) % 8;
            word <<= index;
            if (i < bufLen - 1)
                word |= buffer[i + 1] >> (8 - index);

            i++;
        } else {
            word = (BYTE)((buffer[i] >> (8 - (index + 5))) & 0x1F);
            index = (index + 5) % 8;
            if (index == 0)
               i++;
        }

		Base32Buff.AppendChar(base32Chars[word]);
    }

    return Base32Buff;
}

// Returns a BASE16 encoded byte array
//
// [In]
//   buffer: Pointer to byte array
//   bufLen: Lenght of buffer array
//
// [Return]
//   CString object with BASE16 encoded byte array
CString EncodeBase16(const unsigned char* buffer, uint_ptr bufLen)
{
	CString Base16Buff;
	LPTSTR strBuffer = Base16Buff.GetBuffer(2*bufLen);// X: [CI] - [Code Improvement]
	for(uint_ptr i = 0; i < bufLen; i++) {
		*strBuffer++ = _acHexDigits[buffer[i] >> 4];
		*strBuffer++ = _acHexDigits[buffer[i] & 0xf];
	}
	Base16Buff.ReleaseBuffer(2*bufLen);
    return Base16Buff;
}

// Calculates length to decode from BASE16
//
// [In]
//   base16Length: Actual length of BASE16 string
//
// [Return]
//   New length of byte array decoded
uint_ptr DecodeLengthBase16(uint_ptr base16Length)
{
	return base16Length / 2U;
}

uint_ptr DecodeBase32(LPCTSTR pszInput, uchar* paucOutput, uint_ptr nBufferLen)
{
	if (pszInput == NULL)
		return false;
	uint_ptr nDecodeLen = (_tcslen(pszInput)*5)/8;
	if ((_tcslen(pszInput)*5) % 8 > 0)
		nDecodeLen++;
	uint_ptr nInputLen = _tcslen( pszInput );
	if (paucOutput == NULL || nBufferLen == 0)
		return nDecodeLen;
	if (nDecodeLen > nBufferLen || paucOutput == NULL) 
		return 0;

	uint_ptr nBits	= 0;
	uint_ptr nCount	= 0;

	for ( uint_ptr nChars = nInputLen ; nChars-- ; pszInput++ )
	{
		if ( *pszInput >= 'A' && *pszInput <= 'Z' )
			nBits |= ( *pszInput - 'A' );
		else if ( *pszInput >= 'a' && *pszInput <= 'z' )
			nBits |= ( *pszInput - 'a' );
		else if ( *pszInput >= '2' && *pszInput <= '7' )
			nBits |= ( *pszInput - '2' + 26 );
		else
			return 0;
		
		nCount += 5;

		if ( nCount >= 8 )
		{
			*paucOutput++ = (BYTE)( nBits >> ( nCount - 8 ) );
			nCount -= 8;
		}

		nBits <<= 5;
	}

	return nDecodeLen;
}

uint_ptr DecodeBase32(LPCSTR pszInput, uchar* paucOutput, uint_ptr nBufferLen)
{
	if (pszInput == NULL)
		return false;
	uint_ptr nDecodeLen = (strlen(pszInput)*5)/8;
	if ((strlen(pszInput)*5) % 8 > 0)
		nDecodeLen++;
	uint_ptr nInputLen = strlen( pszInput );
	if (paucOutput == NULL || nBufferLen == 0)
		return nDecodeLen;
	if (nDecodeLen > nBufferLen || paucOutput == NULL) 
		return 0;

	uint_ptr nBits	= 0;
	uint_ptr nCount	= 0;

	for ( uint_ptr nChars = nInputLen ; nChars-- ; pszInput++ )
	{
		if ( *pszInput >= 'A' && *pszInput <= 'Z' )
			nBits |= ( *pszInput - 'A' );
		else if ( *pszInput >= 'a' && *pszInput <= 'z' )
			nBits |= ( *pszInput - 'a' );
		else if ( *pszInput >= '2' && *pszInput <= '7' )
			nBits |= ( *pszInput - '2' + 26 );
		else
			return 0;

		nCount += 5;

		if ( nCount >= 8 )
		{
			*paucOutput++ = (BYTE)( nBits >> ( nCount - 8 ) );
			nCount -= 8;
		}

		nBits <<= 5;
	}

	return nDecodeLen;
}

uint_ptr DecodeBase32(LPCTSTR pszInput, CAICHHash& Hash)
{
	return DecodeBase32(pszInput, Hash.GetRawHash(), HASHSIZE);
}

typedef struct
{
	LPCTSTR	pszInitialDir;
	LPCTSTR	pszDlgTitle;
} BROWSEINIT, *LPBROWSEINIT;

extern "C" int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		// Set initial directory
		if ( ((LPBROWSEINIT)lpData)->pszInitialDir != NULL )
			SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)((LPBROWSEINIT)lpData)->pszInitialDir);

		// Set dialog's window title
		if ( ((LPBROWSEINIT)lpData)->pszDlgTitle != NULL )
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)((LPBROWSEINIT)lpData)->pszDlgTitle);
	}

	return 0;
}

bool SelectDir(HWND hWnd, LPTSTR pszPath, LPCTSTR pszTitle, LPCTSTR pszDlgTitle)
{
	bool bResult = false;
	CoInitialize(0);
	LPMALLOC pShlMalloc;
	if (SHGetMalloc(&pShlMalloc) == NOERROR)
	{
		BROWSEINFO BrsInfo = {0};
		BrsInfo.hwndOwner = hWnd;
		BrsInfo.lpszTitle = (pszTitle != NULL) ? pszTitle : pszDlgTitle;
		BrsInfo.ulFlags = BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_SHAREABLE | BIF_DONTGOBELOWDOMAIN;

		BROWSEINIT BrsInit = {0};
		if (pszPath != NULL || pszTitle != NULL || pszDlgTitle != NULL){
			// Need the 'BrowseCallbackProc' to set those strings
			BrsInfo.lpfn = BrowseCallbackProc;
			BrsInfo.lParam = (LPARAM)&BrsInit;
			BrsInit.pszDlgTitle = (pszDlgTitle != NULL) ? pszDlgTitle : NULL/*pszTitle*/;
			BrsInit.pszInitialDir = pszPath;
		}

		LPITEMIDLIST pidlBrowse;
		if ((pidlBrowse = SHBrowseForFolder(&BrsInfo)) != NULL){
			if (SHGetPathFromIDList(pidlBrowse, pszPath))
				bResult = true;
			pShlMalloc->Free(pidlBrowse);
		}
		pShlMalloc->Release();
	}
	CoUninitialize();
	return bResult;
}

void MakeFoldername(CString &rstrPath)
{
	if (!rstrPath.IsEmpty()) // don't canonicalize an empty path, we would get a "\"
	{
		CString strNewPath;
		LPTSTR pszNewPath = strNewPath.GetBuffer(MAX_PATH);
		PathCanonicalize(pszNewPath, rstrPath);
		PathRemoveBackslash(pszNewPath);
		strNewPath.ReleaseBuffer();
		rstrPath = strNewPath;
	}
}

void md4str(const uchar* hash, TCHAR* pszHash)
{
    for (size_t i = 0; i < 16; i++){
		*pszHash++ = _acHexDigits[hash[i] >> 4];
		*pszHash++ = _acHexDigits[hash[i] & 0xf];
	}
	*pszHash = _T('\0');
}

CString md4str(const uchar* hash)
{
	//TCHAR szHash[MAX_HASHSTR_SIZE];
	//md4str(hash, szHash);
	//return CString(szHash);
	CString str;
	md4str(hash, str.GetBuffer(16*2));
	str.ReleaseBuffer(16*2);
	return str;
}

void md4strA(const uchar* hash, CHAR* pszHash)
{
    static const CHAR _acHexDigits[] = "0123456789ABCDEF";
    for (size_t i = 0; i < 16; i++){
		*pszHash++ = _acHexDigits[hash[i] >> 4];
		*pszHash++ = _acHexDigits[hash[i] & 0xf];
	}
	*pszHash = '\0';
}

CStringA md4strA(const uchar* hash)
{
	//CHAR szHash[MAX_HASHSTR_SIZE];
	//md4strA(hash, szHash);
	//return CStringA(szHash);
	CStringA str;
	md4strA(hash, str.GetBuffer(16*2));
	str.ReleaseBuffer(16*2);
	return str;
}

bool strmd4(const char* pszHash, uchar* hash)
{
	for (size_t i = 0; i < 16; i++){
		sint_ptr h, l;
		if((h=charhexval(*pszHash++))==-1||(l=charhexval(*pszHash++))==-1)
		{
			md4clr(hash);
			return false;
		}
		hash[i] = (char)((h<<4)|l);
	}
	return true;
}

bool wcsmd4(const TCHAR* pszHash, uchar* hash)
{
	for (size_t i = 0; i < 16; i++){
		sint_ptr h, l;
		if((h=charhexval(*pszHash++))==-1||(l=charhexval(*pszHash++))==-1)
		{
			md4clr(hash);
			return false;
		}
		hash[i] = (char)((h<<4)|l);
	}
	return true;
}

void StripTrailingCollon(CString& rstr)
{
	if (!rstr.IsEmpty())
	{
		if (rstr[rstr.GetLength() - 1] == _T(':'))
			rstr = rstr.Left(rstr.GetLength() - 1);
	}
}

// ShellContextMenu :: Start
CString ConcatFullPath(const CString &strPath, const CString &strFname)
{
	if (strPath.Right(1) == _T('\\'))
		return (strPath + strFname);
	else
		return (strPath + _T('\\') + strFname);
}
// ShellContextMenu :: End

CString CleanupFilename(CString filename, bool bExtension)
{
	filename = URLDecode(filename);
	filename.MakeLower();

	//remove substrings, defined in the preferences (.ini)
	CString strlink = thePrefs.GetFilenameCleanups();
	strlink.MakeLower();

	int curPos = 0;
	CString resToken = strlink.Tokenize(_T("|"), curPos);
	while (!resToken.IsEmpty())
	{
		filename.Replace(resToken, _T(""));
		resToken = strlink.Tokenize(_T("|"), curPos);
	}

	// Replace "." with space - except the last one (extension-dot)
	int extpos = bExtension ? filename.ReverseFind(_T('.')) : filename.GetLength();
	if (extpos > 0)
	{
		for (int i = 0; i < extpos; i++)
		{
			if (filename.GetAt(i) != _T('.'))
				continue;
			if (i > 0 && i < filename.GetLength()-1 && _istdigit((_TUCHAR)filename.GetAt(i-1)) && _istdigit((_TUCHAR)filename.GetAt(i+1)))
				continue;
			filename.SetAt(i, _T(' '));
		}
	}

	// replace space-holders with spaces
	filename.Replace(_T('_'), _T(' '));
	filename.Replace(_T('+'), _T(' '));
	filename.Replace(_T('='), _T(' '));

	// remove invalid filename characters
	filename.Remove(_T('\\'));
	filename.Remove(_T('\"'));
	filename.Remove(_T('/'));
	filename.Remove(_T(':'));
	filename.Remove(_T('*'));
	filename.Remove(_T('?'));
	filename.Remove(_T('<'));
	filename.Remove(_T('>'));
	filename.Remove(_T('|'));

	// remove [AD]
	CString tempStr;
	int pos1 = -1;
	for (;;)
	{
		pos1 = filename.Find(_T('['), pos1+1);
		if (pos1 == -1)
			break;
		int pos2 = filename.Find(_T(']'), pos1);
		if (pos1 > -1 && pos2 > pos1)
		{
			if (pos2 - pos1 > 1)
			{
				tempStr = filename.Mid(pos1+1, pos2-pos1-1);
				int numcount = 0;
				for (int i = 0; i < tempStr.GetLength(); i++)
				{
					if (_istdigit((_TUCHAR)tempStr.GetAt(i)))
						numcount++;
				}
				if (numcount > tempStr.GetLength()/2)
					continue;
			}
			filename = filename.Left(pos1) + filename.Right(filename.GetLength()-pos2-1);
			pos1--;
		}
		else
			break;
	}

	// Make leading Caps 
	if (filename.GetLength() > 1)
	{
		tempStr = filename.GetAt(0);
		tempStr.MakeUpper();
		filename.SetAt(0, tempStr.GetAt(0));

		int topos = filename.ReverseFind(_T('.')) - 1;
		if (topos < 0)
			topos = filename.GetLength() - 1;

		for (int ix = 0; ix < topos; ix++)
		{
			if (!_istalpha((_TUCHAR)filename.GetAt(ix)))
			{
				if (	(ix < filename.GetLength()-2 && _istdigit((_TUCHAR)filename.GetAt(ix+2))) ||
						filename.GetAt(ix)==_T('\'')
					)
					continue;

				tempStr = filename.GetAt(ix+1);
				tempStr.MakeUpper();
				filename.SetAt(ix+1, tempStr.GetAt(0));
			}
		}
	}

	// additional formatting
	filename.Replace(_T("()"), _T(""));
	filename.Replace(_T("  "), _T(" "));
	filename.Replace(_T(" ."), _T("."));
	filename.Replace(_T("( "), _T("("));
	filename.Replace(_T(" )"), _T(")"));
	filename.Replace(_T("()"), _T(""));
	filename.Replace(_T("{ "), _T("{"));
	filename.Replace(_T(" }"), _T("}"));
	filename.Replace(_T("{}"), _T(""));

	filename.Trim();
	return filename;
}

struct SED2KFileType
{
	LPCTSTR pszExt;
	EED2KFileType iFileType;
} g_aED2KFileTypes[] = 
{
/*
	{ _T(".aac"),		ED2KFT_AUDIO },		// Advanced Audio Coding File
	{ _T(".ac3"),		ED2KFT_AUDIO },		// Audio Codec 3 File
	{ _T(".aif"),		ED2KFT_AUDIO },		// Audio Interchange File Format
	{ _T(".aifc"),		ED2KFT_AUDIO },		// Audio Interchange File Format
	{ _T(".aiff"),		ED2KFT_AUDIO },		// Audio Interchange File Format
	{ _T(".amr"),		ED2KFT_AUDIO },		// Adaptive Multi-Rate Codec File
	{ _T(".ape"),		ED2KFT_AUDIO },		// Monkey's Audio Lossless Audio File
	{ _T(".au"),		ED2KFT_AUDIO },		// Audio File (Sun,		Unix)
	{ _T(".aud"),		ED2KFT_AUDIO },		// General Audio File
	{ _T(".audio"),		ED2KFT_AUDIO },		// General Audio File
	{ _T(".cda"),		ED2KFT_AUDIO },		// CD Audio Track
	{ _T(".dmf"),		ED2KFT_AUDIO },		// Delusion Digital Music File
	{ _T(".dsm"),		ED2KFT_AUDIO },		// Digital Sound Module
	{ _T(".dts"),		ED2KFT_AUDIO },		// DTS Encoded Audio File
	{ _T(".far"),		ED2KFT_AUDIO },		// Farandole Composer Module
	{ _T(".flac"),		ED2KFT_AUDIO },		// Free Lossless Audio Codec File
	{ _T(".it"),		ED2KFT_AUDIO },		// Impulse Tracker Module
	{ _T(".m1a"),		ED2KFT_AUDIO },		// MPEG-1 Audio File
	{ _T(".m2a"),		ED2KFT_AUDIO },		// MPEG-2 Audio File
	{ _T(".m4a"),		ED2KFT_AUDIO },		// MPEG-4 Audio File
	{ _T(".mdl"),		ED2KFT_AUDIO },		// DigiTrakker Module
	{ _T(".med"),		ED2KFT_AUDIO },		// Amiga MED Sound File
	{ _T(".mid"),		ED2KFT_AUDIO },		// MIDI File
	{ _T(".midi"),		ED2KFT_AUDIO },		// MIDI File
	{ _T(".mka"),		ED2KFT_AUDIO },		// Matroska Audio File
	{ _T(".mod"),		ED2KFT_AUDIO },		// Amiga Music Module File
	{ _T(".mp1"),		ED2KFT_AUDIO },		// MPEG-1 Audio File
	{ _T(".mp2"),		ED2KFT_AUDIO },		// MPEG-2 Audio File
	{ _T(".mp3"),		ED2KFT_AUDIO },		// MPEG-3 Audio File
	{ _T(".mpa"),		ED2KFT_AUDIO },		// MPEG Audio File
	{ _T(".mpc"),		ED2KFT_AUDIO },		// Musepack Compressed Audio File
	{ _T(".mtm"),		ED2KFT_AUDIO },		// MultiTracker Module
	{ _T(".ogg"),		ED2KFT_AUDIO },		// Ogg Vorbis Compressed Audio File
	{ _T(".psm"),		ED2KFT_AUDIO },		// Protracker Studio Module
	{ _T(".ptm"),		ED2KFT_AUDIO },		// PolyTracker Module
	{ _T(".ra"),		ED2KFT_AUDIO },		// Real Audio File
	{ _T(".rmi"),		ED2KFT_AUDIO },		// MIDI File
	{ _T(".s3m"),		ED2KFT_AUDIO },		// Scream Tracker 3 Module
	{ _T(".snd"),		ED2KFT_AUDIO },		// Audio File (Sun,		Unix)
	{ _T(".stm"),		ED2KFT_AUDIO },		// Scream Tracker 2 Module
	{ _T(".umx"),		ED2KFT_AUDIO },		// Unreal Music Package
	{ _T(".wav"),		ED2KFT_AUDIO },		// WAVE Audio File
	{ _T(".wma"),		ED2KFT_AUDIO },		// Windows Media Audio File
	{ _T(".xm"),		ED2KFT_AUDIO },		// Fasttracker 2 Extended Module

	{ _T(".3g2"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T(".3gp"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T(".3gp2"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T(".3gpp"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T(".amv"),		ED2KFT_VIDEO },		// Anime Music Video File
	{ _T(".asf"),		ED2KFT_VIDEO },		// Advanced Systems Format File
	{ _T(".avi"),		ED2KFT_VIDEO },		// Audio Video Interleave File
	{ _T(".bik"),		ED2KFT_VIDEO },		// BINK Video File
	{ _T(".divx"),		ED2KFT_VIDEO },		// DivX-Encoded Movie File
	{ _T(".dvr-ms"),	ED2KFT_VIDEO },		// Microsoft Digital Video Recording
	{ _T(".flc"),		ED2KFT_VIDEO },		// FLIC Video File
	{ _T(".fli"),		ED2KFT_VIDEO },		// FLIC Video File
	{ _T(".flic"),		ED2KFT_VIDEO },		// FLIC Video File
	{ _T(".flv"),		ED2KFT_VIDEO },		// Flash Video File
	{ _T(".hdmov"),		ED2KFT_VIDEO },		// High-Definition QuickTime Movie
	{ _T(".ifo"),		ED2KFT_VIDEO },		// DVD-Video Disc Information File
	{ _T(".m1v"),		ED2KFT_VIDEO },		// MPEG-1 Video File
	{ _T(".m2t"),		ED2KFT_VIDEO },		// MPEG-2 Video Transport Stream
	{ _T(".m2ts"),		ED2KFT_VIDEO },		// MPEG-2 Video Transport Stream
	{ _T(".m2v"),		ED2KFT_VIDEO },		// MPEG-2 Video File
	{ _T(".m4b"),		ED2KFT_VIDEO },		// MPEG-4 Video File
	{ _T(".m4v"),		ED2KFT_VIDEO },		// MPEG-4 Video File
	{ _T(".mkv"),		ED2KFT_VIDEO },		// Matroska Video File
	{ _T(".mov"),		ED2KFT_VIDEO },		// QuickTime Movie File
	{ _T(".movie"),		ED2KFT_VIDEO },		// QuickTime Movie File
	{ _T(".mp1v"),		ED2KFT_VIDEO },		// MPEG-1 Video File        
	{ _T(".mp2v"),		ED2KFT_VIDEO },		// MPEG-2 Video File
	{ _T(".mp4"),		ED2KFT_VIDEO },		// MPEG-4 Video File
	{ _T(".mpe"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T(".mpeg"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T(".mpg"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T(".mpv"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T(".mpv1"),		ED2KFT_VIDEO },		// MPEG-1 Video File
	{ _T(".mpv2"),		ED2KFT_VIDEO },		// MPEG-2 Video File
	{ _T(".ogm"),		ED2KFT_VIDEO },		// Ogg Media File
	{ _T(".pva"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T(".qt"),		ED2KFT_VIDEO },		// QuickTime Movie
	{ _T(".ram"),		ED2KFT_VIDEO },		// Real Audio Media
	{ _T(".ratdvd"),	ED2KFT_VIDEO },		// RatDVD Disk Image
	{ _T(".rm"),		ED2KFT_VIDEO },		// Real Media File
	{ _T(".rmm"),		ED2KFT_VIDEO },		// Real Media File
	{ _T(".rmvb"),		ED2KFT_VIDEO },		// Real Video Variable Bit Rate File
	{ _T(".rv"),		ED2KFT_VIDEO },		// Real Video File
	{ _T(".smil"),		ED2KFT_VIDEO },		// SMIL Presentation File
	{ _T(".smk"),		ED2KFT_VIDEO },		// Smacker Compressed Movie File
	{ _T(".swf"),		ED2KFT_VIDEO },		// Macromedia Flash Movie
	{ _T(".tp"),		ED2KFT_VIDEO },		// Video Transport Stream File
	{ _T(".ts"),		ED2KFT_VIDEO },		// Video Transport Stream File
	{ _T(".vid"),		ED2KFT_VIDEO },		// General Video File
	{ _T(".video"),		ED2KFT_VIDEO },		// General Video File
	{ _T(".vob"),		ED2KFT_VIDEO },		// DVD Video Object File
	{ _T(".vp6"),		ED2KFT_VIDEO },		// TrueMotion VP6 Video File
	{ _T(".wm"),		ED2KFT_VIDEO },		// Windows Media Video File
	{ _T(".wmv"),		ED2KFT_VIDEO },		// Windows Media Video File
	{ _T(".xvid"),		ED2KFT_VIDEO },		// Xvid-Encoded Video File

	{ _T(".bmp"),		ED2KFT_IMAGE },		// Bitmap Image File
	{ _T(".emf"),		ED2KFT_IMAGE },		// Enhanced Windows Metafile
	{ _T(".gif"),		ED2KFT_IMAGE },		// Graphical Interchange Format File
	{ _T(".ico"),		ED2KFT_IMAGE },		// Icon File
	{ _T(".jfif"),		ED2KFT_IMAGE },		// JPEG File Interchange Format
	{ _T(".jpe"),		ED2KFT_IMAGE },		// JPEG Image File
	{ _T(".jpeg"),		ED2KFT_IMAGE },		// JPEG Image File
	{ _T(".jpg"),		ED2KFT_IMAGE },		// JPEG Image File
	{ _T(".pct"),		ED2KFT_IMAGE },		// PICT Picture File
	{ _T(".pcx"),		ED2KFT_IMAGE },		// Paintbrush Bitmap Image File
	{ _T(".pic"),		ED2KFT_IMAGE },		// PICT Picture File
	{ _T(".pict"),		ED2KFT_IMAGE },		// PICT Picture File
	{ _T(".png"),		ED2KFT_IMAGE },		// Portable Network Graphic
	{ _T(".psd"),		ED2KFT_IMAGE },		// Photoshop Document
	{ _T(".psp"),		ED2KFT_IMAGE },		// Paint Shop Pro Image File
	{ _T(".tga"),		ED2KFT_IMAGE },		// Targa Graphic
	{ _T(".tif"),		ED2KFT_IMAGE },		// Tagged Image File
	{ _T(".tiff"),		ED2KFT_IMAGE },		// Tagged Image File
	{ _T(".wmf"),		ED2KFT_IMAGE },		// Windows Metafile
	{ _T(".wmp"),		ED2KFT_IMAGE },		// Windows Media Photo File
	{ _T(".xif"),		ED2KFT_IMAGE },		// ScanSoft Pagis Extended Image Format File

	{ _T(".7z"),		ED2KFT_ARCHIVE },		// 7-Zip Compressed File
	{ _T(".ace"),		ED2KFT_ARCHIVE },		// WinAce Compressed File
	{ _T(".alz"),		ED2KFT_ARCHIVE },		// ALZip Archive
	{ _T(".arc"),		ED2KFT_ARCHIVE },		// Compressed File Archive
	{ _T(".arj"),		ED2KFT_ARCHIVE },		// ARJ Compressed File Archive
	{ _T(".bz2"),		ED2KFT_ARCHIVE },		// Bzip Compressed File
	{ _T(".cab"),		ED2KFT_ARCHIVE },		// Cabinet File
	{ _T(".cbr"),		ED2KFT_ARCHIVE },		// Comic Book RAR Archive
	{ _T(".cbz"),		ED2KFT_ARCHIVE },		// Comic Book ZIP Archive
	{ _T(".gz"),		ED2KFT_ARCHIVE },		// Gnu Zipped File
	{ _T(".hqx"),		ED2KFT_ARCHIVE },		// BinHex 4.0 Encoded File
	{ _T(".lha"),		ED2KFT_ARCHIVE },		// LHARC Compressed Archive
	{ _T(".lzh"),		ED2KFT_ARCHIVE },		// LZH Compressed File
	{ _T(".msi"),		ED2KFT_ARCHIVE },		// Microsoft Installer File
	{ _T(".pak"),		ED2KFT_ARCHIVE },		// PAK (Packed) File
	{ _T(".par"),		ED2KFT_ARCHIVE },		// Parchive Index File
	{ _T(".par2"),		ED2KFT_ARCHIVE },		// Parchive 2 Index File
	{ _T(".rar"),		ED2KFT_ARCHIVE },		// WinRAR Compressed Archive
	{ _T(".sit"),		ED2KFT_ARCHIVE },		// Stuffit Archive
	{ _T(".sitx"),		ED2KFT_ARCHIVE },		// Stuffit X Archive
	{ _T(".tar"),		ED2KFT_ARCHIVE },		// Consolidated Unix File Archive
	{ _T(".tbz2"),		ED2KFT_ARCHIVE },		// Tar BZip 2 Compressed File
	{ _T(".tgz"),		ED2KFT_ARCHIVE },		// Gzipped Tar File
	{ _T(".xpi"),		ED2KFT_ARCHIVE },		// Mozilla Installer Package
	{ _T(".z"),			ED2KFT_ARCHIVE },		// Unix Compressed File
	{ _T(".zip"),		ED2KFT_ARCHIVE },		// Zipped File

	{ _T(".bat"),		ED2KFT_PROGRAM },		// Batch File
	{ _T(".cmd"),		ED2KFT_PROGRAM },		// Command File
	{ _T(".com"),		ED2KFT_PROGRAM },		// COM File
	{ _T(".exe"),		ED2KFT_PROGRAM },		// Executable File
	{ _T(".hta"),		ED2KFT_PROGRAM },		// HTML Application
	{ _T(".js"),		ED2KFT_PROGRAM },		// Java Script
	{ _T(".jse"),		ED2KFT_PROGRAM },		// Encoded  Java Script
	{ _T(".msc"),		ED2KFT_PROGRAM },		// Microsoft Common Console File
	{ _T(".vbe"),		ED2KFT_PROGRAM },		// Encoded Visual Basic Script File
	{ _T(".vbs"),		ED2KFT_PROGRAM },		// Visual Basic Script File
	{ _T(".wsf"),		ED2KFT_PROGRAM },		// Windows Script File
	{ _T(".wsh"),		ED2KFT_PROGRAM },		// Windows Scripting Host File

	{ _T(".bin"),		ED2KFT_CDIMAGE },		// CD Image
	{ _T(".bwa"),		ED2KFT_CDIMAGE },		// BlindWrite Disk Information File
	{ _T(".bwi"),		ED2KFT_CDIMAGE },		// BlindWrite CD/DVD Disc Image
	{ _T(".bws"),		ED2KFT_CDIMAGE },		// BlindWrite Sub Code File
	{ _T(".bwt"),		ED2KFT_CDIMAGE },		// BlindWrite 4 Disk Image
	{ _T(".ccd"),		ED2KFT_CDIMAGE },		// CloneCD Disk Image
	{ _T(".cue"),		ED2KFT_CDIMAGE },		// Cue Sheet File
	{ _T(".dmg"),		ED2KFT_CDIMAGE },		// Mac OS X Disk Image
	{ _T(".img"),		ED2KFT_CDIMAGE },		// Disk Image Data File
	{ _T(".iso"),		ED2KFT_CDIMAGE },		// Disc Image File
	{ _T(".mdf"),		ED2KFT_CDIMAGE },		// Media Disc Image File
	{ _T(".mds"),		ED2KFT_CDIMAGE },		// Media Descriptor File
	{ _T(".nrg"),		ED2KFT_CDIMAGE },		// Nero CD/DVD Image File
	{ _T(".sub"),		ED2KFT_CDIMAGE },		// Subtitle File
	{ _T(".toast"),		ED2KFT_CDIMAGE },		// Toast Disc Image

	{ _T(".chm"),		ED2KFT_DOCUMENT },		// Compiled HTML Help File
	{ _T(".css"),		ED2KFT_DOCUMENT },		// Cascading Style Sheet
	{ _T(".diz"),		ED2KFT_DOCUMENT },		// Description in Zip File
	{ _T(".doc"),		ED2KFT_DOCUMENT },		// Document File
	{ _T(".dot"),		ED2KFT_DOCUMENT },		// Document Template File
	{ _T(".hlp"),		ED2KFT_DOCUMENT },		// Help File
	{ _T(".htm"),		ED2KFT_DOCUMENT },		// HTML File
	{ _T(".html"),		ED2KFT_DOCUMENT },		// HTML File
	{ _T(".nfo"),		ED2KFT_DOCUMENT },		// Warez Information File
	{ _T(".pdf"),		ED2KFT_DOCUMENT },		// Portable Document Format File
	{ _T(".pps"),		ED2KFT_DOCUMENT },		// PowerPoint Slide Show
	{ _T(".ppt"),		ED2KFT_DOCUMENT },		// PowerPoint Presentation
	{ _T(".ps"),		ED2KFT_DOCUMENT },		// PostScript File
	{ _T(".rtf"),		ED2KFT_DOCUMENT },		// Rich Text Format File
	{ _T(".text"),		ED2KFT_DOCUMENT },		// General Text File
	{ _T(".txt"),		ED2KFT_DOCUMENT },		// Text File
	{ _T(".wri"),		ED2KFT_DOCUMENT },		// Windows Write Document
	{ _T(".xls"),		ED2KFT_DOCUMENT },		// Microsoft Excel Spreadsheet
	{ _T(".xml"),		ED2KFT_DOCUMENT },		// XML File
	{ _T(".djvu"),		ED2KFT_DOCUMENT },		//Enig123::added from IS mod

	{ _T(".uha"),   ED2KFT_ARCHIVE },
	{ _T(".mht"),   ED2KFT_DOCUMENT },

	{ _T(".emulecollection"),		ED2KFT_EMULECOLLECTION }
	*/
// sorted version // X: [CI] - [Code Improvement]
	{ _T("3g2"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T("3gp"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T("3gp2"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T("3gpp"),		ED2KFT_VIDEO },		// 3GPP Multimedia File
	{ _T("7z"),		ED2KFT_ARCHIVE },		// 7-Zip Compressed File
	{ _T("aac"),		ED2KFT_AUDIO },		// Advanced Audio Coding File
	{ _T("ac3"),		ED2KFT_AUDIO },		// Audio Codec 3 File
	{ _T("ace"),		ED2KFT_ARCHIVE },		// WinAce Compressed File
	{ _T("aif"),		ED2KFT_AUDIO },		// Audio Interchange File Format
	{ _T("aifc"),		ED2KFT_AUDIO },		// Audio Interchange File Format
	{ _T("aiff"),		ED2KFT_AUDIO },		// Audio Interchange File Format
	{ _T("alz"),		ED2KFT_ARCHIVE },		// ALZip Archive
	{ _T("amr"),		ED2KFT_AUDIO },		// Adaptive Multi-Rate Codec File
	{ _T("amv"),		ED2KFT_VIDEO },		// Anime Music Video File
	{ _T("ape"),		ED2KFT_AUDIO },		// Monkey's Audio Lossless Audio File
	{ _T("arc"),		ED2KFT_ARCHIVE },		// Compressed File Archive
	{ _T("arj"),		ED2KFT_ARCHIVE },		// ARJ Compressed File Archive
	{ _T("asf"),		ED2KFT_VIDEO },		// Advanced Systems Format File
	{ _T("au"),		ED2KFT_AUDIO },		// Audio File (Sun,		Unix)
	{ _T("aud"),		ED2KFT_AUDIO },		// General Audio File
	{ _T("audio"),		ED2KFT_AUDIO },		// General Audio File
	{ _T("avi"),		ED2KFT_VIDEO },		// Audio Video Interleave File
	{ _T("bat"),		ED2KFT_PROGRAM },		// Batch File
	{ _T("bik"),		ED2KFT_VIDEO },		// BINK Video File
	{ _T("bin"),		ED2KFT_CDIMAGE },		// CD Image
	{ _T("bmp"),		ED2KFT_IMAGE },		// Bitmap Image File
	{ _T("bwa"),		ED2KFT_CDIMAGE },		// BlindWrite Disk Information File
	{ _T("bwi"),		ED2KFT_CDIMAGE },		// BlindWrite CD/DVD Disc Image
	{ _T("bws"),		ED2KFT_CDIMAGE },		// BlindWrite Sub Code File
	{ _T("bwt"),		ED2KFT_CDIMAGE },		// BlindWrite 4 Disk Image
	{ _T("bz2"),		ED2KFT_ARCHIVE },		// Bzip Compressed File
	{ _T("cab"),		ED2KFT_ARCHIVE },		// Cabinet File
	{ _T("cbr"),		ED2KFT_ARCHIVE },		// Comic Book RAR Archive
	{ _T("cbz"),		ED2KFT_ARCHIVE },		// Comic Book ZIP Archive
	{ _T("ccd"),		ED2KFT_CDIMAGE },		// CloneCD Disk Image
	{ _T("cda"),		ED2KFT_AUDIO },		// CD Audio Track
	{ _T("chm"),		ED2KFT_DOCUMENT },		// Compiled HTML Help File
	{ _T("cmd"),		ED2KFT_PROGRAM },		// Command File
	{ _T("com"),		ED2KFT_PROGRAM },		// COM File
	{ _T("css"),		ED2KFT_DOCUMENT },		// Cascading Style Sheet
	{ _T("cue"),		ED2KFT_CDIMAGE },		// Cue Sheet File
	{ _T("divx"),		ED2KFT_VIDEO },		// DivX-Encoded Movie File
	{ _T("diz"),		ED2KFT_DOCUMENT },		// Description in Zip File
	{ _T("djvu"),		ED2KFT_DOCUMENT },		//Enig123::added from IS mod
	{ _T("dmf"),		ED2KFT_AUDIO },		// Delusion Digital Music File
	{ _T("dmg"),		ED2KFT_CDIMAGE },		// Mac OS X Disk Image
	{ _T("doc"),		ED2KFT_DOCUMENT },		// Document File
	{ _T("dot"),		ED2KFT_DOCUMENT },		// Document Template File
	{ _T("dsm"),		ED2KFT_AUDIO },		// Digital Sound Module
	{ _T("dts"),		ED2KFT_AUDIO },		// DTS Encoded Audio File
	{ _T("dvr-ms"),	ED2KFT_VIDEO },		// Microsoft Digital Video Recording
	{ _T("emf"),		ED2KFT_IMAGE },		// Enhanced Windows Metafile
	{ _T("emulecollection"),		ED2KFT_EMULECOLLECTION },
	{ _T("exe"),		ED2KFT_PROGRAM },		// Executable File
	{ _T("far"),		ED2KFT_AUDIO },		// Farandole Composer Module
	{ _T("flac"),		ED2KFT_AUDIO },		// Free Lossless Audio Codec File
	{ _T("flc"),		ED2KFT_VIDEO },		// FLIC Video File
	{ _T("fli"),		ED2KFT_VIDEO },		// FLIC Video File
	{ _T("flic"),		ED2KFT_VIDEO },		// FLIC Video File
	{ _T("flv"),		ED2KFT_VIDEO },		// Flash Video File
	{ _T("gif"),		ED2KFT_IMAGE },		// Graphical Interchange Format File
	{ _T("gz"),		ED2KFT_ARCHIVE },		// Gnu Zipped File
	{ _T("hdmov"),		ED2KFT_VIDEO },		// High-Definition QuickTime Movie
	{ _T("hlp"),		ED2KFT_DOCUMENT },		// Help File
	{ _T("hqx"),		ED2KFT_ARCHIVE },		// BinHex 4.0 Encoded File
	{ _T("hta"),		ED2KFT_PROGRAM },		// HTML Application
	{ _T("htm"),		ED2KFT_DOCUMENT },		// HTML File
	{ _T("html"),		ED2KFT_DOCUMENT },		// HTML File
	{ _T("ico"),		ED2KFT_IMAGE },		// Icon File
	{ _T("ifo"),		ED2KFT_VIDEO },		// DVD-Video Disc Information File
	{ _T("img"),		ED2KFT_CDIMAGE },		// Disk Image Data File
	{ _T("iso"),		ED2KFT_CDIMAGE },		// Disc Image File
	{ _T("it"),		ED2KFT_AUDIO },		// Impulse Tracker Module
	{ _T("jfif"),		ED2KFT_IMAGE },		// JPEG File Interchange Format
	{ _T("jpe"),		ED2KFT_IMAGE },		// JPEG Image File
	{ _T("jpeg"),		ED2KFT_IMAGE },		// JPEG Image File
	{ _T("jpg"),		ED2KFT_IMAGE },		// JPEG Image File
	{ _T("js"),		ED2KFT_PROGRAM },		// Java Script
	{ _T("jse"),		ED2KFT_PROGRAM },		// Encoded  Java Script
	{ _T("lha"),		ED2KFT_ARCHIVE },		// LHARC Compressed Archive
	{ _T("lzh"),		ED2KFT_ARCHIVE },		// LZH Compressed File
	{ _T("m1a"),		ED2KFT_AUDIO },		// MPEG-1 Audio File
	{ _T("m1v"),		ED2KFT_VIDEO },		// MPEG-1 Video File
	{ _T("m2a"),		ED2KFT_AUDIO },		// MPEG-2 Audio File
	{ _T("m2t"),		ED2KFT_VIDEO },		// MPEG-2 Video Transport Stream
	{ _T("m2ts"),		ED2KFT_VIDEO },		// MPEG-2 Video Transport Stream
	{ _T("m2v"),		ED2KFT_VIDEO },		// MPEG-2 Video File
	{ _T("m4a"),		ED2KFT_AUDIO },		// MPEG-4 Audio File
	{ _T("m4b"),		ED2KFT_VIDEO },		// MPEG-4 Video File
	{ _T("m4v"),		ED2KFT_VIDEO },		// MPEG-4 Video File
	{ _T("mdf"),		ED2KFT_CDIMAGE },		// Media Disc Image File
	{ _T("mdl"),		ED2KFT_AUDIO },		// DigiTrakker Module
	{ _T("mds"),		ED2KFT_CDIMAGE },		// Media Descriptor File
	{ _T("med"),		ED2KFT_AUDIO },		// Amiga MED Sound File
	{ _T("mht"),		ED2KFT_DOCUMENT },
	{ _T("mid"),		ED2KFT_AUDIO },		// MIDI File
	{ _T("midi"),		ED2KFT_AUDIO },		// MIDI File
	{ _T("mka"),		ED2KFT_AUDIO },		// Matroska Audio File
	{ _T("mkv"),		ED2KFT_VIDEO },		// Matroska Video File
	{ _T("mod"),		ED2KFT_AUDIO },		// Amiga Music Module File
	{ _T("mov"),		ED2KFT_VIDEO },		// QuickTime Movie File
	{ _T("movie"),		ED2KFT_VIDEO },		// QuickTime Movie File
	{ _T("mp1"),		ED2KFT_AUDIO },		// MPEG-1 Audio File
	{ _T("mp1v"),		ED2KFT_VIDEO },		// MPEG-1 Video File        
	{ _T("mp2"),		ED2KFT_AUDIO },		// MPEG-2 Audio File
	{ _T("mp2v"),		ED2KFT_VIDEO },		// MPEG-2 Video File
	{ _T("mp3"),		ED2KFT_AUDIO },		// MPEG-3 Audio File
	{ _T("mp4"),		ED2KFT_VIDEO },		// MPEG-4 Video File
	{ _T("mpa"),		ED2KFT_AUDIO },		// MPEG Audio File
	{ _T("mpc"),		ED2KFT_AUDIO },		// Musepack Compressed Audio File
	{ _T("mpe"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T("mpeg"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T("mpg"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T("mpv"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T("mpv1"),		ED2KFT_VIDEO },		// MPEG-1 Video File
	{ _T("mpv2"),		ED2KFT_VIDEO },		// MPEG-2 Video File
	{ _T("msc"),		ED2KFT_PROGRAM },		// Microsoft Common Console File
	{ _T("msi"),		ED2KFT_ARCHIVE },		// Microsoft Installer File
	{ _T("mtm"),		ED2KFT_AUDIO },		// MultiTracker Module
	{ _T("nfo"),		ED2KFT_DOCUMENT },		// Warez Information File
	{ _T("nrg"),		ED2KFT_CDIMAGE },		// Nero CD/DVD Image File
	{ _T("ogg"),		ED2KFT_AUDIO },		// Ogg Vorbis Compressed Audio File
	{ _T("ogm"),		ED2KFT_VIDEO },		// Ogg Media File
	{ _T("pak"),		ED2KFT_ARCHIVE },		// PAK (Packed) File
	{ _T("par"),		ED2KFT_ARCHIVE },		// Parchive Index File
	{ _T("par2"),		ED2KFT_ARCHIVE },		// Parchive 2 Index File
	{ _T("pct"),		ED2KFT_IMAGE },		// PICT Picture File
	{ _T("pcx"),		ED2KFT_IMAGE },		// Paintbrush Bitmap Image File
	{ _T("pdf"),		ED2KFT_DOCUMENT },		// Portable Document Format File
	{ _T("pic"),		ED2KFT_IMAGE },		// PICT Picture File
	{ _T("pict"),		ED2KFT_IMAGE },		// PICT Picture File
	{ _T("png"),		ED2KFT_IMAGE },		// Portable Network Graphic
	{ _T("pps"),		ED2KFT_DOCUMENT },		// PowerPoint Slide Show
	{ _T("ppt"),		ED2KFT_DOCUMENT },		// PowerPoint Presentation
	{ _T("ps"),		ED2KFT_DOCUMENT },		// PostScript File
	{ _T("psd"),		ED2KFT_IMAGE },		// Photoshop Document
	{ _T("psm"),		ED2KFT_AUDIO },		// Protracker Studio Module
	{ _T("psp"),		ED2KFT_IMAGE },		// Paint Shop Pro Image File
	{ _T("ptm"),		ED2KFT_AUDIO },		// PolyTracker Module
	{ _T("pva"),		ED2KFT_VIDEO },		// MPEG Video File
	{ _T("qt"),		ED2KFT_VIDEO },		// QuickTime Movie
	{ _T("ra"),		ED2KFT_AUDIO },		// Real Audio File
	{ _T("ram"),		ED2KFT_VIDEO },		// Real Audio Media
	{ _T("rar"),		ED2KFT_ARCHIVE },		// WinRAR Compressed Archive
	{ _T("ratdvd"),	ED2KFT_VIDEO },		// RatDVD Disk Image
	{ _T("rm"),		ED2KFT_VIDEO },		// Real Media File
	{ _T("rmi"),		ED2KFT_AUDIO },		// MIDI File
	{ _T("rmm"),		ED2KFT_VIDEO },		// Real Media File
	{ _T("rmvb"),		ED2KFT_VIDEO },		// Real Video Variable Bit Rate File
	{ _T("rtf"),		ED2KFT_DOCUMENT },		// Rich Text Format File
	{ _T("rv"),		ED2KFT_VIDEO },		// Real Video File
	{ _T("s3m"),		ED2KFT_AUDIO },		// Scream Tracker 3 Module
	{ _T("sit"),		ED2KFT_ARCHIVE },		// Stuffit Archive
	{ _T("sitx"),		ED2KFT_ARCHIVE },		// Stuffit X Archive
	{ _T("smil"),		ED2KFT_VIDEO },		// SMIL Presentation File
	{ _T("smk"),		ED2KFT_VIDEO },		// Smacker Compressed Movie File
	{ _T("snd"),		ED2KFT_AUDIO },		// Audio File (Sun,		Unix)
	{ _T("stm"),		ED2KFT_AUDIO },		// Scream Tracker 2 Module
	{ _T("sub"),		ED2KFT_CDIMAGE },		// Subtitle File
	{ _T("swf"),		ED2KFT_VIDEO },		// Macromedia Flash Movie
	{ _T("tar"),		ED2KFT_ARCHIVE },		// Consolidated Unix File Archive
	{ _T("tbz2"),		ED2KFT_ARCHIVE },		// Tar BZip 2 Compressed File
	{ _T("text"),		ED2KFT_DOCUMENT },		// General Text File
	{ _T("tga"),		ED2KFT_IMAGE },		// Targa Graphic
	{ _T("tgz"),		ED2KFT_ARCHIVE },		// Gzipped Tar File
	{ _T("tif"),		ED2KFT_IMAGE },		// Tagged Image File
	{ _T("tiff"),		ED2KFT_IMAGE },		// Tagged Image File
	{ _T("toast"),		ED2KFT_CDIMAGE },		// Toast Disc Image
	{ _T("tp"),		ED2KFT_VIDEO },		// Video Transport Stream File
	{ _T("ts"),		ED2KFT_VIDEO },		// Video Transport Stream File
	{ _T("txt"),		ED2KFT_DOCUMENT },		// Text File
	{ _T("uha"),   ED2KFT_ARCHIVE },
	{ _T("umx"),		ED2KFT_AUDIO },		// Unreal Music Package
	{ _T("vbe"),		ED2KFT_PROGRAM },		// Encoded Visual Basic Script File
	{ _T("vbs"),		ED2KFT_PROGRAM },		// Visual Basic Script File
	{ _T("vid"),		ED2KFT_VIDEO },		// General Video File
	{ _T("video"),		ED2KFT_VIDEO },		// General Video File
	{ _T("vob"),		ED2KFT_VIDEO },		// DVD Video Object File
	{ _T("vp6"),		ED2KFT_VIDEO },		// TrueMotion VP6 Video File
	{ _T("wav"),		ED2KFT_AUDIO },		// WAVE Audio File
	{ _T("wm"),		ED2KFT_VIDEO },		// Windows Media Video File
	{ _T("wma"),		ED2KFT_AUDIO },		// Windows Media Audio File
	{ _T("wmf"),		ED2KFT_IMAGE },		// Windows Metafile
	{ _T("wmp"),		ED2KFT_IMAGE },		// Windows Media Photo File
	{ _T("wmv"),		ED2KFT_VIDEO },		// Windows Media Video File
	{ _T("wri"),		ED2KFT_DOCUMENT },		// Windows Write Document
	{ _T("wsf"),		ED2KFT_PROGRAM },		// Windows Script File
	{ _T("wsh"),		ED2KFT_PROGRAM },		// Windows Scripting Host File
	{ _T("xif"),		ED2KFT_IMAGE },		// ScanSoft Pagis Extended Image Format File
	{ _T("xls"),		ED2KFT_DOCUMENT },		// Microsoft Excel Spreadsheet
	{ _T("xm"),		ED2KFT_AUDIO },		// Fasttracker 2 Extended Module
	{ _T("xml"),		ED2KFT_DOCUMENT },		// XML File
	{ _T("xpi"),		ED2KFT_ARCHIVE },		// Mozilla Installer Package
	{ _T("xvid"),		ED2KFT_VIDEO },		// Xvid-Encoded Video File
	{ _T("z"),			ED2KFT_ARCHIVE },		// Unix Compressed File
	{ _T("zip"),		ED2KFT_ARCHIVE }		// Zipped File
};

int __cdecl CompareE2DKFileType(const void* p1, const void* p2)
{
	return _tcscmp( ((const SED2KFileType*)p1)->pszExt, ((const SED2KFileType*)p2)->pszExt );
}

EED2KFileType GetED2KFileTypeID(LPCTSTR pszFileName)
{
	LPCTSTR pszExt = _tcsrchr(pszFileName, _T('.'));
	if (pszExt == NULL)
		return ED2KFT_ANY;
	CString strExt(pszExt + 1);
	strExt.MakeLower();

	SED2KFileType ft;
	ft.pszExt = strExt;
	ft.iFileType = ED2KFT_ANY;
	const SED2KFileType* pFound = (SED2KFileType*)bsearch(&ft, g_aED2KFileTypes, _countof(g_aED2KFileTypes), sizeof g_aED2KFileTypes[0], CompareE2DKFileType);
	if (pFound != NULL)
		return pFound->iFileType;
	return ED2KFT_ANY;
}

// Retuns the ed2k file type string ID which is to be used for publishing+searching
LPCSTR GetED2KFileTypeSearchTerm(EED2KFileType iFileID)
{
	if (iFileID == ED2KFT_AUDIO)			return ED2KFTSTR_AUDIO;
	if (iFileID == ED2KFT_VIDEO)			return ED2KFTSTR_VIDEO;
	if (iFileID == ED2KFT_IMAGE)			return ED2KFTSTR_IMAGE;
	if (iFileID == ED2KFT_PROGRAM)			return ED2KFTSTR_PROGRAM;
	if (iFileID == ED2KFT_DOCUMENT)			return ED2KFTSTR_DOCUMENT;
	// NOTE: Archives and CD-Images are published+searched with file type "Pro"
	// NOTE: If this gets changed, the function 'GetED2KFileTypeSearchID' also needs to get updated!
	if (iFileID == ED2KFT_ARCHIVE)			return ED2KFTSTR_PROGRAM;
	if (iFileID == ED2KFT_CDIMAGE)			return ED2KFTSTR_PROGRAM;
	if (iFileID == ED2KFT_EMULECOLLECTION)	return ED2KFTSTR_EMULECOLLECTION;
	return NULL;
}

// Retuns the ed2k file type integer ID which is to be used for publishing+searching
EED2KFileType GetED2KFileTypeSearchID(EED2KFileType iFileID)
{
	if (iFileID == ED2KFT_AUDIO)			return ED2KFT_AUDIO;
	if (iFileID == ED2KFT_VIDEO)			return ED2KFT_VIDEO;
	if (iFileID == ED2KFT_IMAGE)			return ED2KFT_IMAGE;
	if (iFileID == ED2KFT_PROGRAM)			return ED2KFT_PROGRAM;
	if (iFileID == ED2KFT_DOCUMENT)			return ED2KFT_DOCUMENT;
	// NOTE: Archives and CD-Images are published+searched with file type "Pro"
	// NOTE: If this gets changed, the function 'GetED2KFileTypeSearchTerm' also needs to get updated!
	if (iFileID == ED2KFT_ARCHIVE)			return ED2KFT_PROGRAM;
	if (iFileID == ED2KFT_CDIMAGE)			return ED2KFT_PROGRAM;
	return ED2KFT_ANY;
}

// Returns a file type which is used eMule internally only, examining the extension of the given filename
CString GetFileTypeByName(LPCTSTR pszFileName)
{
	EED2KFileType iFileType = GetED2KFileTypeID(pszFileName);
	switch (iFileType) {
		case ED2KFT_AUDIO:			return _CString(_T(ED2KFTSTR_AUDIO));
		case ED2KFT_VIDEO:			return _CString(_T(ED2KFTSTR_VIDEO));
		case ED2KFT_IMAGE:			return _CString(_T(ED2KFTSTR_IMAGE));
		case ED2KFT_DOCUMENT:		return _CString(_T(ED2KFTSTR_DOCUMENT));
		case ED2KFT_PROGRAM:		return _CString(_T(ED2KFTSTR_PROGRAM));
		case ED2KFT_ARCHIVE:		return _CString(_T(ED2KFTSTR_ARCHIVE));
		case ED2KFT_CDIMAGE:		return _CString(_T(ED2KFTSTR_CDIMAGE));
		case ED2KFT_EMULECOLLECTION:return _CString(_T(ED2KFTSTR_EMULECOLLECTION));
		default:					return _T("");
	}
}

// Returns a file type which is used eMule internally only (GUI)
CString GetFileTypeDisplayStrFromED2KFileType(LPCTSTR pszED2KFileType)
{
	ASSERT( pszED2KFileType != NULL );
	if (pszED2KFileType != NULL)
	{
		if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_AUDIO)) == 0)			return GetResString(IDS_SEARCH_AUDIO);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_VIDEO)) == 0)    return GetResString(IDS_SEARCH_VIDEO);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_IMAGE)) == 0)    return GetResString(IDS_SEARCH_PICS);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_DOCUMENT)) == 0)	return GetResString(IDS_SEARCH_DOC);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_PROGRAM)) == 0)  return GetResString(IDS_SEARCH_PRG);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_ARCHIVE)) == 0)	return GetResString(IDS_SEARCH_ARC);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_CDIMAGE)) == 0)  return GetResString(IDS_SEARCH_CDIMG);
		else if (_tcscmp(pszED2KFileType, _T(ED2KFTSTR_EMULECOLLECTION)) == 0)	return GetResString(IDS_SEARCH_EMULECOLLECTION);
	}
	return _T("");
}

#ifdef _DEBUG// X: [CI] - [Code Improvement]
class CED2KFileTypes{
public:
	CED2KFileTypes(){
		SED2KFileType ED2KFileTypes[_countof(g_aED2KFileTypes)];
		memcpy(ED2KFileTypes, g_aED2KFileTypes, sizeof(g_aED2KFileTypes));

		qsort(g_aED2KFileTypes, _countof(g_aED2KFileTypes), sizeof g_aED2KFileTypes[0], CompareE2DKFileType);
		// check for duplicate entries
		LPCTSTR pszLast = g_aED2KFileTypes[0].pszExt;
		for (int i = 1; i < _countof(g_aED2KFileTypes); i++){
			ASSERT( _tcscmp(pszLast, g_aED2KFileTypes[i].pszExt) != 0 );
			pszLast = g_aED2KFileTypes[i].pszExt;
		}
		for(INT_PTR i = 0; i<_countof(g_aED2KFileTypes); ++i){
			if(ED2KFileTypes[i].pszExt!=g_aED2KFileTypes[i].pszExt)
				ASSERT(0);// not sorted
		}
	}
};
CED2KFileTypes theED2KFileTypes; // get the list sorted *before* any code is accessing it
#endif

const char *stristr(const char *str1, const char *str2)
{
	if (!*str2)
		return str1;

	const char *cp = str1;
	while (*cp)
	{
		const char *s1 = cp;
		const char *s2 = str2;

		while (*s1 && *s2 && tolower(*s1) == tolower(*s2))
			s1++, s2++;

		if (!*s2)
			return cp;

		cp++;
	}

	return NULL;
}

const TCHAR *wcsistr(const TCHAR *str1, const TCHAR *str2)
{
	if (!*str2)
		return str1;

	const TCHAR *cp = str1;
	while (*cp)
	{
		const TCHAR *s1 = cp;
		const TCHAR *s2 = str2;

		while (*s1 && *s2 && _totlower(*s1) == _totlower(*s2))
			s1++, s2++;

		if (!*s2)
			return cp;

		cp++;
	}

	return NULL;
}

CString GetNextString(const CString& rstr, LPCTSTR pszTokens, int& riStart)
{
	CString strResult;
	if (pszTokens != NULL && riStart != -1)
	{
		int iToken = rstr.Find(pszTokens, riStart);
		if (iToken != -1)
		{
			int iLen = iToken - riStart;
			if (iLen >= 0)
			{
				strResult = rstr.Mid(riStart, iLen);
				riStart += iLen + 1;
			}
		}
		else
		{
			strResult = rstr.Mid(riStart);
			riStart = -1;
		}
	}
	return strResult;
}

CString GetNextString(const CString& rstr, TCHAR chToken, int& riStart)
{
	CString strResult;
	if (chToken != _T('\0') && riStart != -1)
	{
		int iToken = rstr.Find(chToken, riStart);
		if (iToken != -1)
		{
			int iLen = iToken - riStart;
			if (iLen >= 0)
			{
				strResult = rstr.Mid(riStart, iLen);
				riStart += iLen + 1;
			}
		}
		else
		{
			strResult = rstr.Mid(riStart);
			riStart = -1;
		}
	}
	return strResult;
}

int GetSystemErrorString(DWORD dwError, CString &rstrError)
{
	// FormatMessage language flags:
	//
	// - MFC uses: MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT)
	//				SUBLANG_SYS_DEFAULT = 0x02 (system default)
	//
	// - SDK uses: MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
	//				SUBLANG_DEFAULT		= 0x01 (user default)
	//
	//
	// Found in "winnt.h"
	// ------------------
	//  Language IDs.
	//
	//  The following two combinations of primary language ID and
	//  sublanguage ID have special semantics:
	//
	//    Primary Language ID   Sublanguage ID      Result
	//    -------------------   ---------------     ------------------------
	//    LANG_NEUTRAL          SUBLANG_NEUTRAL     Language neutral
	//    LANG_NEUTRAL          SUBLANG_DEFAULT     User default language
	//    LANG_NEUTRAL          SUBLANG_SYS_DEFAULT System default language
	//
	// *** SDK notes also:
	// If you pass in zero, 'FormatMessage' looks for a message for LANGIDs in 
	// the following order:
	//
	//	1) Language neutral 
	//	2) Thread LANGID, based on the thread's locale value 
	//  3) User default LANGID, based on the user's default locale value 
	//	4) System default LANGID, based on the system default locale value 
	//	5) US English 
	LPTSTR pszSysMsg = NULL;
	DWORD dwLength = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
								   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
								   (LPTSTR)&pszSysMsg, 0, NULL);
	if (pszSysMsg != NULL && dwLength != 0)
	{
		if (dwLength >= 2 && pszSysMsg[dwLength - 2] == _T('\r'))
			pszSysMsg[dwLength - 2] = _T('\0');
		rstrError = pszSysMsg;
		rstrError.Replace(_T("\r\n"), _T(" ")); // some messages contain CRLF within the message!?
	}
	else {
		rstrError.Empty();
	}

	if (pszSysMsg)
		LocalFree(pszSysMsg);

	return rstrError.GetLength();
}

int GetModuleErrorString(DWORD dwError, CString &rstrError, LPCTSTR pszModule)
{
	LPTSTR pszSysMsg = NULL;
	DWORD dwLength = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
								   GetModuleHandle(pszModule), dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
								   (LPTSTR)&pszSysMsg, 0, NULL);
	if (pszSysMsg != NULL && dwLength != 0)
	{
		if (dwLength >= 2 && pszSysMsg[dwLength - 2] == _T('\r'))
			pszSysMsg[dwLength - 2] = _T('\0');
		rstrError = pszSysMsg;
		rstrError.Replace(_T("\r\n"), _T(" ")); // some messages contain CRLF within the message!?
	}
	else {
		rstrError.Empty();
	}

	if (pszSysMsg)
		LocalFree(pszSysMsg);

	return rstrError.GetLength();
}

int GetErrorMessage(DWORD dwError, CString &rstrErrorMsg, DWORD dwFlags)
{
	int iMsgLen = GetSystemErrorString(dwError, rstrErrorMsg);
	if (iMsgLen == 0)
	{
		if ((long)dwError >= 0)
			rstrErrorMsg.Format(_T("Error %u"), dwError);
		else
			rstrErrorMsg.Format(_T("Error 0x%08x"), dwError);
	}
	else if (dwFlags & 1)
	{
		CString strFullErrorMsg;
		if ((long)dwError >= 0)
			strFullErrorMsg.Format(_T("Error %u: %s"), dwError, rstrErrorMsg);
		else
			strFullErrorMsg.Format(_T("Error 0x%08x: %s"), dwError, rstrErrorMsg);
		rstrErrorMsg = strFullErrorMsg;
	}

	return rstrErrorMsg.GetLength();
}

CString GetErrorMessage(DWORD dwError, DWORD dwFlags)
{
	CString strError;
	GetErrorMessage(dwError, strError, dwFlags);
	return strError;
}

int GetDesktopColorDepth()
{
	HDC hdcScreen = ::GetDC(HWND_DESKTOP);
	int iColorBits = GetDeviceCaps(hdcScreen, BITSPIXEL) * GetDeviceCaps(hdcScreen, PLANES);
	::ReleaseDC(HWND_DESKTOP, hdcScreen);
	return iColorBits;
}

int GetAppImageListColorFlag()
{
	int iColorBits = GetDesktopColorDepth();
	if (iColorBits >= 32) // Avi-3k
		return ILC_COLOR32;
	if (iColorBits >= 24)
		return ILC_COLOR24;
	if (iColorBits >= 16)
		return ILC_COLOR16;
	if (iColorBits >= 8)
		return ILC_COLOR8;
	if (iColorBits >= 4)
		return ILC_COLOR4;
	return ILC_COLOR;
}

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
CString DbgGetHexDump(const uint8* data, UINT size)
{
	CString buffer; 
	buffer.Format(_T("Size=%u"), size);
	buffer += _T(", Data=[");
	UINT i = 0;
	for(; i < size && i < 50; i++){
		if (i > 0)
			buffer += _T(' ');
		TCHAR temp[3];
		_stprintf(temp, _T("%02x"), data[i]);
		buffer += temp;
	}
	buffer += (i == size) ? _T("]") : _T("..]");
	return buffer;
}

void DbgSetThreadName(LPCSTR szThreadName, ...) 
{

#ifndef MS_VC_EXCEPTION
#define MS_VC_EXCEPTION 0x406d1388 

typedef struct tagTHREADNAME_INFO 
{
	DWORD dwType;		// must be 0x1000 
	LPCSTR szName;		// pointer to name (in same addr space) 
	DWORD dwThreadID;	// thread ID (-1 caller thread) 
	DWORD dwFlags;		// reserved for future use, must be zero 
} THREADNAME_INFO; 
#endif

	__try
	{
		va_list args;
		va_start(args, szThreadName);
		int lenBuf = 0;
		char *buffer = NULL;
		int lenResult;
		do // the VS debugger truncates the string to 31 characters anyway!
		{
			lenBuf += 128;
			delete[] buffer;
			buffer = new char[lenBuf];
			lenResult = _vsnprintf(buffer, lenBuf - 1, szThreadName, args);
		} while (lenResult == -1);
		va_end(args);
		THREADNAME_INFO info; 
		info.dwType = 0x1000; 
		info.szName = buffer; 
		info.dwThreadID = (DWORD)-1; 
		info.dwFlags = 0; 
		__try 
		{ 
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info); 
		} __except (EXCEPTION_CONTINUE_EXECUTION) { } 
		delete [] buffer;
	}
	__except (EXCEPTION_CONTINUE_EXECUTION) {}
//#else
	//UNREFERENCED_PARAMETER(szThreadName);
}
#endif

CString RemoveFileExtension(const CString& rstrFilePath)
{
	int iDot = rstrFilePath.ReverseFind(_T('.'));
	if (iDot == -1)
		return rstrFilePath;
	return rstrFilePath.Mid(0, iDot);
}

int CompareDirectories(const CString& rstrDir1, const CString& rstrDir2)
{
	int len1 = rstrDir1.GetLength(),len2 = rstrDir2.GetLength();
	if(len1 == len2)
		return _tcsnicmp(rstrDir1, rstrDir2, len1);
	if(len1 && rstrDir1[len1-1] == L'\\')
		--len1;
	if(len2 && rstrDir2[len2-1] == L'\\')
		--len2;
	if(len1 == len2)
		return _tcsnicmp(rstrDir1, rstrDir2, len1);
	return 1;
}

bool IsGoodIP(uint32 nIP, bool forceCheck)
{
	// always filter following IP's
	// -------------------------------------------
	// 0.0.0.0							invalid
	// 127.0.0.0 - 127.255.255.255		Loopback
    // 224.0.0.0 - 239.255.255.255		Multicast
    // 240.0.0.0 - 255.255.255.255		Reserved for Future Use
	// 255.255.255.255					invalid

	if (nIP==0 || (uint8)nIP==127 || (uint8)nIP>=224){
#ifdef _DEBUG
		if (nIP==0x0100007F && thePrefs.GetAllowLocalHostIP())
			return true;
#endif
		return false;
	}

	if (!thePrefs.FilterLANIPs() && !forceCheck/*ZZ:UploadSpeedSense*/)
		return true;
	else
		return !IsLANIP(nIP);
}

bool IsLANIP(uint32 nIP){
	// LAN IP's
	// -------------------------------------------
	//	0.*								"This" Network
	//	10.0.0.0 - 10.255.255.255		Class A
	//	172.16.0.0 - 172.31.255.255		Class B
	//	192.168.0.0 - 192.168.255.255	Class C

	uint8 nFirst = (uint8)nIP;
	uint8 nSecond = (uint8)(nIP >> 8);

	if (nFirst==192 && nSecond==168) // check this 1st, because those LANs IPs are mostly spreaded
		return true;

	if (nFirst==172 && nSecond>=16 && nSecond<=31)
		return true;

	if (nFirst==0 || nFirst==10)
		return true;

	return false; 
}

bool IsGoodIPPort(uint32 nIP, uint16 nPort)
{
	return IsGoodIP(nIP) && nPort!=0;
}

CString GetFormatedUInt(ULONG ulVal)
{
	TCHAR szVal[12];
	_ultot_s(ulVal, szVal, 10);

	static NUMBERFMT nf;
	if (nf.Grouping == 0) {
		nf.NumDigits = 0;
		nf.LeadingZero = 0;
		nf.Grouping = 3;
		// we are hardcoding the following two format chars by intention because the C-RTL also has the decimal sep hardcoded to '.'
		nf.lpDecimalSep = _T(".");
		nf.lpThousandSep = _T(",");
		nf.NegativeOrder = 0;
	}
	CString strVal;
	const int iBuffSize = _countof(szVal)*2;
	int iResult = GetNumberFormat(LOCALE_SYSTEM_DEFAULT, 0, szVal, &nf, strVal.GetBuffer(iBuffSize), iBuffSize);
	strVal.ReleaseBuffer();
	if (iResult == 0)
		strVal = szVal;
	return strVal;
}

CString GetFormatedUInt64(ULONGLONG ullVal)
{
	TCHAR szVal[24];
	_ui64tot_s(ullVal, szVal, _countof(szVal), 10);

	static NUMBERFMT nf;
	if (nf.Grouping == 0) {
		nf.NumDigits = 0;
		nf.LeadingZero = 0;
		nf.Grouping = 3;
		// we are hardcoding the following two format chars by intention because the C-RTL also has the decimal sep hardcoded to '.'
		nf.lpDecimalSep = _T(".");
		nf.lpThousandSep = _T(",");
		nf.NegativeOrder = 0;
	}
	CString strVal;
	const int iBuffSize = _countof(szVal)*2;
	int iResult = GetNumberFormat(LOCALE_SYSTEM_DEFAULT, 0, szVal, &nf, strVal.GetBuffer(iBuffSize), iBuffSize);
	strVal.ReleaseBuffer();
	if (iResult == 0)
		strVal = szVal;
	return strVal;
}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
void Debug(LPCTSTR pszFmtMsg, ...)
{
	va_list pArgs;
	va_start(pArgs, pszFmtMsg);
	CString strBuff;
#ifdef _DEBUG
	time_t tNow = time(NULL);
	int iTimeLen = (int)_tcsftime(strBuff.GetBuffer(40), 40, _T("%H:%M:%S "), localtime(&tNow));
	strBuff.ReleaseBuffer(iTimeLen);
#endif
	strBuff.AppendFormatV(pszFmtMsg, pArgs);

	// get around a bug in the debug device which is not capable of dumping long strings
	int i = 0;
	while (i < strBuff.GetLength()){
		OutputDebugString(strBuff.Mid(i, 1024));
		i += 1024;
	}
	va_end(pArgs);
}

void DebugHexDump(const uint8* data, UINT lenData)
{
	int lenLine = 16;
	UINT pos = 0;
	byte c = 0;
	while (pos < lenData)
	{
		CStringA line;
		CStringA single;
		line.Format("%08X ", pos);
		lenLine = min((lenData - pos), 16);
		for (int i=0; i<lenLine; i++)
		{
			single.Format(" %02X", data[pos+i]);
			line += single;
			if (i == 7)
				line += ' ';
		}
		line += CStringA(' ', 60 - line.GetLength());
		for (int i=0; i<lenLine; i++)
		{
			c = data[pos + i];
			single.Format("%c", (((c > 31) && (c < 127)) ? (_TUCHAR)c : '.'));
			line += single;
		}
		Debug(_T("%hs\n"), line);
		pos += lenLine;
	}
}

void DebugHexDump(CFile& file)
{
	int iSize = (int)(file.GetLength() - file.GetPosition());
	if (iSize > 0)
	{
		uint8* data = NULL;
		try{
			data = new uint8[iSize];
			file.Read(data, iSize);
			DebugHexDump(data, iSize);
		}
		catch(CFileException* e){
			TRACE("*** DebugHexDump(CFile&); CFileException\n");
			e->Delete();
		}
		catch(CMemoryException* e){
			TRACE("*** DebugHexDump(CFile&); CMemoryException\n");
			e->Delete();
		}
		delete[] data;
	}
}
#endif
LPCTSTR DbgGetFileNameFromID(const uchar* hash)
{
	CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(hash);
	if (reqfile != NULL)
		return reqfile->GetFileName();

	CPartFile* partfile = theApp.downloadqueue->GetFileByID(hash);
	if (partfile != NULL)
		return partfile->GetFileName();

	CKnownFile* knownfile = theApp.knownfiles->FindKnownFileByID(hash);
	if (knownfile != NULL)
		return knownfile->GetFileName();

	return NULL;
}

CString DbgGetFileInfo(const uchar* hash)
{
	if (hash == NULL)
		return CString();

	CString strInfo(_T("File="));
	LPCTSTR pszName = DbgGetFileNameFromID(hash);
	if (pszName != NULL)
		strInfo += pszName;
	else
		strInfo += md4str(hash);
	return strInfo;
}

CString DbgGetFileStatus(UINT nPartCount, CSafeMemFile* data)
{
	CString strFileStatus;
	if (nPartCount == 0)
		strFileStatus = _T("Complete");
	else
	{
		CString strPartStatus;
		UINT nAvailableParts = 0;
		UINT nPart = 0;
		while (nPart < nPartCount)
		{
			uint8 ucPartMask;
			try {
				ucPartMask = data->ReadUInt8();
			}
			catch (CFileException* ex) {
				ex->Delete();
				strPartStatus = _T("*PacketException*");
				break;
			}
			for (int i = 0; i < 8; i++)
			{
				bool bPartAvailable = (((ucPartMask >> i) & 1) != 0);
				if (bPartAvailable)
					nAvailableParts++;
				strPartStatus += bPartAvailable ? _T('#') : _T('.');
				nPart++;
				if (nPart == nPartCount)
					break;
			}
		}
		strFileStatus.Format(_T("Parts=%u  Avail=%u  %s"), nPartCount, nAvailableParts, strPartStatus);
	}
	return strFileStatus;
}

CString DbgGetBlockInfo(const Requested_Block_Struct* block)
{
	return DbgGetBlockInfo(block->StartOffset, block->EndOffset);
}

CString DbgGetBlockInfo(uint64 StartOffset, uint64 EndOffset)
{
	CString strInfo;
	strInfo.Format(_T("%I64u-%I64u (%I64u bytes)"), StartOffset, EndOffset, EndOffset - StartOffset + 1);

	strInfo.AppendFormat(_T(", Part %I64u"), StartOffset/PARTSIZE);
	if (StartOffset/PARTSIZE != EndOffset/PARTSIZE)
		strInfo.AppendFormat(_T("-%I64u(**)"), EndOffset/PARTSIZE);

	strInfo.AppendFormat(_T(", Block %I64u"), StartOffset/EMBLOCKSIZE);
	if (StartOffset/EMBLOCKSIZE != EndOffset/EMBLOCKSIZE)
	{
		strInfo.AppendFormat(_T("-%I64u"), EndOffset/EMBLOCKSIZE);
		if (EndOffset/EMBLOCKSIZE - StartOffset/EMBLOCKSIZE > 1)
			strInfo += _T("(**)");
	}

	return strInfo;
}

CString DbgGetBlockFileInfo(const Requested_Block_Struct* block, const CPartFile* partfile)
{
	CString strInfo(DbgGetBlockInfo(block));
	strInfo += _T("; ");
	strInfo += DbgGetFileInfo(partfile ? partfile->GetFileHash() : NULL);
	return strInfo;
}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
int GetHashType(const uchar* hash)
{
	if (hash[5] == 13 && hash[14] == 110)
		return SO_OLDEMULE;
	else if (hash[5] == 14 && hash[14] == 111)
		return SO_EMULE;
 	else if (hash[5] == 'M' && hash[14] == 'L')
		return SO_MLDONKEY;
	else
		return SO_UNKNOWN;
}

LPCTSTR DbgGetHashTypeString(const uchar* hash)
{
	int iHashType = GetHashType(hash);
	if (iHashType == SO_EMULE)
		return _T("eMule");
	if (iHashType == SO_MLDONKEY)
		return _T("MLdonkey");
	if (iHashType == SO_OLDEMULE)
		return _T("Old eMule");
	ASSERT( iHashType == SO_UNKNOWN );
	return _T("Unknown");
}
#endif
CString DbgGetClientID(uint32 nClientID)
{
	CString strClientID;
	if (IsLowID(nClientID))
		strClientID.Format(_T("LowID=%u"), nClientID);
	else
		strClientID = ipstr(nClientID);
	return strClientID;
}

#define _STRVAL(o)	{_T(#o), o}

CString DbgGetDonkeyClientTCPOpcode(UINT opcode)
{
	static const struct
	{
		LPCTSTR pszOpcode;
		UINT uOpcode;
	} _aOpcodes[] =
	{
		_STRVAL(OP_HELLO),
		_STRVAL(OP_SENDINGPART),
		_STRVAL(OP_REQUESTPARTS),
		_STRVAL(OP_FILEREQANSNOFIL),
		_STRVAL(OP_END_OF_DOWNLOAD),
		_STRVAL(OP_ASKSHAREDFILES),
		_STRVAL(OP_ASKSHAREDFILESANSWER),
		_STRVAL(OP_HELLOANSWER),
		_STRVAL(OP_CHANGE_CLIENT_ID),
		_STRVAL(OP_MESSAGE),
		_STRVAL(OP_SETREQFILEID),
		_STRVAL(OP_FILESTATUS),
		_STRVAL(OP_HASHSETREQUEST),
		_STRVAL(OP_HASHSETANSWER),
		_STRVAL(OP_STARTUPLOADREQ),
		_STRVAL(OP_ACCEPTUPLOADREQ),
		_STRVAL(OP_CANCELTRANSFER),
		_STRVAL(OP_OUTOFPARTREQS),
		_STRVAL(OP_REQUESTFILENAME),
		_STRVAL(OP_REQFILENAMEANSWER),
//		_STRVAL(OP_CHANGE_SLOT),
		_STRVAL(OP_QUEUERANK),
		_STRVAL(OP_ASKSHAREDDIRS),
		_STRVAL(OP_ASKSHAREDFILESDIR),
		_STRVAL(OP_ASKSHAREDDIRSANS),
		_STRVAL(OP_ASKSHAREDFILESDIRANS),
		_STRVAL(OP_ASKSHAREDDENIEDANS)
	};

	for (int i = 0; i < _countof(_aOpcodes); i++)
	{
		if (_aOpcodes[i].uOpcode == opcode)
			return _aOpcodes[i].pszOpcode;
	}
	CString strOpcode;
	strOpcode.Format(_T("0x%02x"), opcode);
	return strOpcode;
}

CString DbgGetMuleClientTCPOpcode(UINT opcode)
{
	static const struct
	{
		LPCTSTR pszOpcode;
		UINT uOpcode;
	} _aOpcodes[] =
	{
		_STRVAL(OP_EMULEINFO),
		_STRVAL(OP_EMULEINFOANSWER),
		_STRVAL(OP_COMPRESSEDPART),
		_STRVAL(OP_QUEUERANKING),
//		_STRVAL(OP_FILEDESC),
		_STRVAL(OP_REQUESTSOURCES),
		_STRVAL(OP_ANSWERSOURCES),
		_STRVAL(OP_REQUESTSOURCES2),
		_STRVAL(OP_ANSWERSOURCES2),
		_STRVAL(OP_PUBLICKEY),
		_STRVAL(OP_SIGNATURE),
		_STRVAL(OP_SECIDENTSTATE),
//		_STRVAL(OP_REQUESTPREVIEW),
//		_STRVAL(OP_PREVIEWANSWER),
		_STRVAL(OP_MULTIPACKET),
		_STRVAL(OP_MULTIPACKETANSWER),
		_STRVAL(OP_PEERCACHE_QUERY),
		_STRVAL(OP_PEERCACHE_ANSWER),
//		_STRVAL(OP_PEERCACHE_ACK),
		_STRVAL(OP_PUBLICIP_ANSWER),
		_STRVAL(OP_PUBLICIP_REQ),
		_STRVAL(OP_PORTTEST),
		_STRVAL(OP_CALLBACK),
		_STRVAL(OP_BUDDYPING),
		_STRVAL(OP_BUDDYPONG),
		_STRVAL(OP_REASKCALLBACKTCP),
		_STRVAL(OP_AICHANSWER),
		_STRVAL(OP_AICHREQUEST),
		_STRVAL(OP_AICHFILEHASHANS),
		_STRVAL(OP_AICHFILEHASHREQ),
		_STRVAL(OP_COMPRESSEDPART_I64),
		_STRVAL(OP_SENDINGPART_I64),
		_STRVAL(OP_REQUESTPARTS_I64),
		_STRVAL(OP_MULTIPACKET_EXT),
//		_STRVAL(OP_CHATCAPTCHAREQ),
//		_STRVAL(OP_CHATCAPTCHARES),
		_STRVAL(OP_FWCHECKUDPREQ),
		_STRVAL(OP_KAD_FWTCPCHECK_ACK)
	};

	for (int i = 0; i < _countof(_aOpcodes); i++)
	{
		if (_aOpcodes[i].uOpcode == opcode)
			return _aOpcodes[i].pszOpcode;
	}
	CString strOpcode;
	strOpcode.Format(_T("0x%02x"), opcode);
	return strOpcode;
}

#undef _STRVAL

CString DbgGetClientTCPPacket(UINT protocol, UINT opcode, UINT size)
{
	CString str;
	if (protocol == OP_EDONKEYPROT)
		str.Format(_T("protocol=eDonkey  opcode=%s  size=%u"), DbgGetDonkeyClientTCPOpcode(opcode), size);
	else if (protocol == OP_PACKEDPROT)
		str.Format(_T("protocol=Packed  opcode=%s  size=%u"), DbgGetMuleClientTCPOpcode(opcode), size);
	else if (protocol == OP_EMULEPROT)
		str.Format(_T("protocol=eMule  opcode=%s  size=%u"), DbgGetMuleClientTCPOpcode(opcode), size);
	else
		str.Format(_T("protocol=0x%02x  opcode=0x%02x  size=%u"), protocol, opcode, size);
	return str;
}

CString DbgGetClientTCPOpcode(UINT protocol, UINT opcode)
{
	CString str;
	if (protocol == OP_EDONKEYPROT)
		str = DbgGetDonkeyClientTCPOpcode(opcode);
	else if (protocol == OP_PACKEDPROT)
		str = DbgGetMuleClientTCPOpcode(opcode);
	else if (protocol == OP_EMULEPROT)
		str = DbgGetMuleClientTCPOpcode(opcode);
	else
		str.Format(_T("protocol=0x%02x  opcode=0x%02x"), protocol, opcode);
	return str;
}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
void DebugRecv(LPCSTR pszMsg, const CUpDownClient* client, const uchar* packet, uint32 nIP)
{
	// 111.222.333.444 = 15 chars
	if (client){
		if (client != NULL && packet != NULL)
			Debug(_T("%-24hs from %s; %s\n"), pszMsg, client->DbgGetClientInfo(true), DbgGetFileInfo(packet));
		else if (client != NULL && packet == NULL)
			Debug(_T("%-24hs from %s\n"), pszMsg, client->DbgGetClientInfo(true));
		else if (client == NULL && packet != NULL)
			Debug(_T("%-24hs; %s\n"), pszMsg, DbgGetFileInfo(packet));
		else
			Debug(_T("%-24hs\n"), pszMsg);
	}
	else{
		if (nIP != 0 && packet != NULL)
			Debug(_T("%-24hs from %-15s; %s\n"), pszMsg, ipstr(nIP), DbgGetFileInfo(packet));
		else if (nIP != 0 && packet == NULL)
			Debug(_T("%-24hs from %-15s\n"), pszMsg, ipstr(nIP));
		else if (nIP == 0 && packet != NULL)
			Debug(_T("%-24hs; %s\n"), pszMsg, DbgGetFileInfo(packet));
		else
			Debug(_T("%-24hs\n"), pszMsg);
	}
}

void DebugSend(LPCSTR pszMsg, const CUpDownClient* client, const uchar* packet)
{
	if (client != NULL && packet != NULL)
		Debug(_T(">>> %-20hs to   %s; %s\n"), pszMsg, client->DbgGetClientInfo(true), DbgGetFileInfo(packet));
	else if (client != NULL && packet == NULL)
		Debug(_T(">>> %-20hs to   %s\n"), pszMsg, client->DbgGetClientInfo(true));
	else if (client == NULL && packet != NULL)
		Debug(_T(">>> %-20hs; %s\n"), pszMsg, DbgGetFileInfo(packet));
	else
		Debug(_T(">>> %-20hs\n"), pszMsg);
}

void DebugSend(LPCSTR pszOpcode, uint32 ip, uint16 port)
{
	TCHAR szIPPort[22];
	_sntprintf(szIPPort, _countof(szIPPort) - 1, _T("%s:%u"), ipstr(ntohl(ip)), port);
	szIPPort[_countof(szIPPort) - 1] = _T('\0');
	Debug(_T(">>> %-20hs to   %-21s\n"), pszOpcode, szIPPort);
}

void DebugSendF(LPCSTR pszOpcode, uint32 ip, uint16 port, LPCTSTR pszMsg, ...)
{
	va_list args;
	va_start(args, pszMsg);
	TCHAR szIPPort[22];
	_sntprintf(szIPPort, _countof(szIPPort) - 1, _T("%s:%u"), ipstr(ntohl(ip)), port);
	szIPPort[_countof(szIPPort) - 1] = _T('\0');
	CString str;
	str.Format(_T(">>> %-20hs to   %-21s; "), pszOpcode, szIPPort);
	str.AppendFormatV(pszMsg, args);
	va_end(args);
	Debug(_T("%s\n"), str);
}

void DebugRecv(LPCSTR pszOpcode, uint32 ip, uint16 port)
{
	TCHAR szIPPort[22];
	_sntprintf(szIPPort, _countof(szIPPort) - 1, _T("%s:%u"), ipstr(ntohl(ip)), port);
	szIPPort[_countof(szIPPort) - 1] = _T('\0');
	Debug(_T("%-24hs from %-21s\n"), pszOpcode, szIPPort);
}

#endif
ULONGLONG GetDiskFileSize(LPCTSTR pszFilePath)
{
	// If the file is not compressed nor sparse, 'GetCompressedFileSize' returns the 'normal' file size.
		ULONGLONG ullCompFileSize;
	((LPDWORD)&ullCompFileSize)[0] = GetCompressedFileSize(pszFilePath, &((LPDWORD)&ullCompFileSize)[1]);
		if (((LPDWORD)&ullCompFileSize)[0] != INVALID_FILE_SIZE || GetLastError() == NO_ERROR)
			return ullCompFileSize;

	// If 'GetCompressedFileSize' failed or is not available, use the default function
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(pszFilePath, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
		return 0;
	FindClose(hFind);

	return (ULONGLONG)fd.nFileSizeHigh << 32 | (ULONGLONG)fd.nFileSizeLow;
}

// Listview helper function
void GetPopupMenuPos(CListCtrl& lv, CPoint& point)
{
	// If the context menu was not opened using the right mouse button,
	// but the keyboard (Shift+F10), get a useful position for the context menu.
	if (point.x == -1 && point.y == -1)
	{
		int iIdxItem = lv.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
		if (iIdxItem != -1)
		{
			CRect rc;
			if (lv.GetItemRect(iIdxItem, &rc, LVIR_BOUNDS))
			{
				point.x = rc.left + lv.GetColumnWidth(0) / 2;
				point.y = rc.top + rc.Height() / 2;
				lv.ClientToScreen(&point);
			}
		}
		else
		{
			point.x = 16;
			point.y = 32;
			lv.ClientToScreen(&point);
		}
	}
}

void GetPopupMenuPos(CTreeCtrl& tv, CPoint& point)
{
	// If the context menu was not opened using the right mouse button,
	// but the keyboard (Shift+F10), get a useful position for the context menu.
	if (point.x == -1 && point.y == -1)
	{
		HTREEITEM hSel = tv.GetNextItem(TVI_ROOT, TVGN_CARET);
		if (hSel)
		{
			CRect rcItem;
			if (tv.GetItemRect(hSel, &rcItem, TRUE))
			{
				point.x = rcItem.left;
				point.y = rcItem.top;
				tv.ClientToScreen(&point);
			}
		}
		else
		{
			point.x = 16;
			point.y = 32;
			tv.ClientToScreen(&point);
		}
	}
}

time_t safe_mktime(struct tm* ptm)
{
	if (ptm == NULL)
		return -1;
	return mktime(ptm);
}

CString StripInvalidFilenameChars(const CString& strText)
{
	LPCTSTR pszSource = strText;
	CString strDest;

	while (*pszSource != _T('\0'))
	{
		if (!((/*(_TUCHAR)*pszSource >= 0 && */(_TCHAR)*pszSource <= 31) ||
			// lots of invalid chars for filenames in windows :=)
			*pszSource == _T('\"') || *pszSource == _T('*') || *pszSource == _T('<')  || *pszSource == _T('>') ||
			*pszSource == _T('?')  || *pszSource == _T('|') || *pszSource == _T('\\') || *pszSource == _T('/') || 
			*pszSource == _T(':')) )
		{
				strDest += *pszSource;
		}
		pszSource++;
	}

	static const LPCTSTR apszReservedFilenames[] = {
		_T("NUL"), _T("CON"), _T("PRN"), _T("AUX"), _T("CLOCK$"),
		_T("COM1"),_T("COM2"),_T("COM3"),_T("COM4"),_T("COM5"),_T("COM6"),_T("COM7"),_T("COM8"),_T("COM9"),
		_T("LPT1"),_T("LPT2"),_T("LPT3"),_T("LPT4"),_T("LPT5"),_T("LPT6"),_T("LPT7"),_T("LPT8"),_T("LPT9")
	};
	for (size_t i = 0; i < _countof(apszReservedFilenames); i++)
	{
		size_t nPrefixLen = _tcslen(apszReservedFilenames[i]);
		if (_tcsnicmp(strDest, apszReservedFilenames[i], nPrefixLen) == 0)
		{
			if (strDest.GetLength() == nPrefixLen) {
				// Filename is a reserved file name:
				// Append an underscore character
				strDest += _T('_');
				break;
			}
			else if (strDest[nPrefixLen] == _T('.')) {
				// Filename starts with a reserved file name followed by a '.' character:
				// Replace that ',' character with an '_' character.
				LPTSTR pszDest = strDest.GetBuffer(strDest.GetLength());
				pszDest[nPrefixLen] = _T('_');
				strDest.ReleaseBuffer(strDest.GetLength());
				break;
			}
		}
	}

	return strDest;
}

bool operator==(const CCKey& k1,const CCKey& k2)
{
	return !md4cmp(k1.m_key, k2.m_key);
}

bool operator==(const CSKey& k1,const CSKey& k2)
{
	return !md4cmp(k1.m_key, k2.m_key);
}

CString ipstr(uint32 nIP)
{
	// following gives the same string as 'inet_ntoa(*(in_addr*)&nIP)' but is not restricted to ASCII strings
	const BYTE* pucIP = (BYTE*)&nIP;
	CString strIP;
	strIP.ReleaseBuffer(_sntprintf(strIP.GetBuffer(3+1+3+1+3+1+3), 3+1+3+1+3+1+3, _T("%u.%u.%u.%u"), pucIP[0], pucIP[1], pucIP[2], pucIP[3]));
	return strIP;
}

CString ipstr(uint32 nIP, uint16 nPort)
{
	// following gives the same string as 'inet_ntoa(*(in_addr*)&nIP)' but is not restricted to ASCII strings
	const BYTE* pucIP = (BYTE*)&nIP;
	CString strIP;
	strIP.ReleaseBuffer(_sntprintf(strIP.GetBuffer(3+1+3+1+3+1+3+1+5), 3+1+3+1+3+1+3+1+5, _T("%u.%u.%u.%u:%u"), pucIP[0], pucIP[1], pucIP[2], pucIP[3], nPort));
	return strIP;
}

CString ipstr(LPCTSTR pszAddress, uint16 nPort)
{
	CString strIPPort;
	strIPPort.Format(_T("%s:%u"), pszAddress, nPort);
	return strIPPort;
}

CStringA ipstrA(uint32 nIP)
{
	const BYTE* pucIP = (BYTE*)&nIP;
	CStringA strIP;
	strIP.ReleaseBuffer(_snprintf(strIP.GetBuffer(3+1+3+1+3+1+3), 3+1+3+1+3+1+3, "%u.%u.%u.%u", pucIP[0], pucIP[1], pucIP[2], pucIP[3]));
	return strIP;
}

void ipstrA(CHAR* pszAddress, int iMaxAddress, uint32 nIP)
{
	const BYTE* pucIP = (BYTE*)&nIP;
	_snprintf(pszAddress, iMaxAddress - 1, "%u.%u.%u.%u", pucIP[0], pucIP[1], pucIP[2], pucIP[3]);
}

bool IsDaylightSavingTimeActive(LONG& rlDaylightBias)
{
	TIME_ZONE_INFORMATION tzi;
	if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_DAYLIGHT)
		return false;
	rlDaylightBias = tzi.DaylightBias;
	return true;
}

bool IsAutoDaylightTimeSetActive()
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"), KEY_READ) == ERROR_SUCCESS)
	{
		DWORD dwDisableAutoDaylightTimeSet = 0;
		if (key.QueryDWORDValue(_T("DisableAutoDaylightTimeSet"), dwDisableAutoDaylightTimeSet) == ERROR_SUCCESS)
		{
			if (dwDisableAutoDaylightTimeSet)
				return false;
		}
	}
	return true; // default to 'Automatically adjust clock for daylight saving changes'
}

bool AdjustNTFSDaylightFileTime(uint64& ruFileDate, LPCTSTR pszFilePath)
{
	if (!thePrefs.GetAdjustNTFSDaylightFileTime())
		return false;
	if (ruFileDate == 0 || ruFileDate == -1)
		return false;

	// See also KB 129574
	LONG lDaylightBias = 0;
	if (IsDaylightSavingTimeActive(lDaylightBias))
	{
		if (IsAutoDaylightTimeSetActive())
		{
			if (g_VolumeInfo.GetVolumeInfoByPath(pszFilePath)->IsNTFS()) // X: [FSFS] - [FileSystemFeaturesSupport]
			{
				ruFileDate += lDaylightBias*60;
				return true;
			}
		}
		else
		{
			// If 'Automatically adjust clock for daylight saving changes' is disabled and
			// if the file's date is within DST period, we get again a wrong file date.
			//
			// If 'Automatically adjust clock for daylight saving changes' is disabled, 
			// Windows always reports 'Active DST'(!!) with a bias of '0' although there is no 
			// DST specified. This means also, that there is no chance to determine if a date is
			// within any DST.
			
			// Following code might be correct, but because we don't have a DST and because we don't have a bias,
			// the code won't do anything useful.
			/*struct tm* ptm = localtime((time_t*)&ruFileDate);
			bool bFileDateInDST = (ptm && ptm->tm_isdst == 1);
			if (bFileDateInDST)
			{
				ruFileDate += lDaylightBias*60;
				return true;
			}*/
		}
	}

	return false;
}

bool ExpandEnvironmentStrings(CString& rstrStrings)
{
	DWORD dwSize = ExpandEnvironmentStrings(rstrStrings, NULL, 0);
	if (dwSize == 0)
		return false;

	CString strExpanded;
	DWORD dwCount = ExpandEnvironmentStrings(rstrStrings, strExpanded.GetBuffer(dwSize-1), dwSize);
	if (dwCount == 0 || dwCount != dwSize){
		ASSERT(0);
		return false;
	}
	strExpanded.ReleaseBuffer(dwCount-1);
	rstrStrings = strExpanded;
	return true;
}

void TriggerPortTest(uint16 tcp, uint16 udp) {
	CString m_sTestURL;

	// do not alter the connection test, this is a manual test only. If you want to change the behaviour, use your server!
	m_sTestURL.Format(PORTTESTURL, tcp, udp , thePrefs.GetLanguageID());

	// the portcheck will need to do an obfuscated callback too if obfuscation is requested, so we have to provide our userhash so it can create the key
	if (thePrefs.IsClientCryptLayerRequested())
		m_sTestURL += _T("&obfuscated_test=") + md4str(thePrefs.GetUserHash());

	ShellOpenFile(m_sTestURL);
}

int CompareLocaleStringNoCase(LPCTSTR psz1, LPCTSTR psz2)
{
	// SDK says: The 'CompareString' function is optimized to run at the highest speed when 'dwCmpFlags' is set to 0 
	// or NORM_IGNORECASE, and 'cchCount1' and 'cchCount2' have the value -1.
	int iResult = CompareString(GetThreadLocale(), NORM_IGNORECASE, psz1, -1, psz2, -1);
	if (iResult == 0)
		return 0;
	return iResult - 2;
}

int __cdecl CompareCStringPtrLocaleStringNoCase(const void* p1, const void* p2)
{
	const CString* pstr1 = (const CString*)p1;
	const CString* pstr2 = (const CString*)p2;
	return CompareLocaleStringNoCase(*pstr1, *pstr2);
}

int __cdecl CompareCStringPtrPtrLocaleStringNoCase(const void* p1, const void* p2)
{
	const CString* pstr1 = *(const CString**)p1;
	const CString* pstr2 = *(const CString**)p2;
	return CompareLocaleStringNoCase(*pstr1, *pstr2);
}

void Sort(CSimpleArray<const CString*>& apstr, int (__cdecl *pfnCompare)(const void*, const void*))
{
	qsort(apstr.GetData(), apstr.GetSize(), sizeof(CString*), pfnCompare);
}

void AddAutoStart()
{
#ifndef _DEBUG
	RemAutoStart();
	CString strKeyName;
	strKeyName = _T("eMuleAutoStart");
    TCHAR sExeFilePath[MAX_PATH];
	DWORD dwModPathLen = ::GetModuleFileName(NULL, sExeFilePath, _countof(sExeFilePath));
	if (dwModPathLen == 0 || dwModPathLen == _countof(sExeFilePath))
		return;
	CString sFullExeCommand;
	sFullExeCommand.Format(_T("%s -AutoStart"), sExeFilePath);
	CRegKey mKey;
	mKey.Create(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		REG_NONE,REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,	NULL,NULL);
	mKey.SetStringValue(strKeyName, sFullExeCommand);
	mKey.Close();
#endif
}

void RemAutoStart()
{
	CString strKeyName;
	strKeyName = _T("eMuleAutoStart");
	CRegKey mKey;
	mKey.Create(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
		REG_NONE,REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,	NULL,NULL);
	mKey.DeleteValue(strKeyName);
	mKey.Close();
}

int FontPointSizeToLogUnits(int nPointSize)
{
	HDC hDC = ::GetDC(HWND_DESKTOP);
	if (hDC)
	{
		POINT pt;
#if 0
		// This is the same math which is performed by "CFont::CreatePointFont",
		// which is flawed because it does not perform any rounding. But without
		// performing the correct rounding one can not get the correct LOGFONT-height
		// for an 8pt font!
		//
		// PointSize	Result
		// -------------------
		// 8*10			10.666 -> 10 (cut down and thus wrong result)
		pt.y = GetDeviceCaps(hDC, LOGPIXELSY) * nPointSize;
		pt.y /= 720;
#else
		// This math accounts for proper rounding and thus we will get the correct results.
		//
		// PointSize	Result
		// -------------------
		// 8*10			10.666 -> 11 (rounded up and thus correct result)
		pt.y = MulDiv(GetDeviceCaps(hDC, LOGPIXELSY), nPointSize, 720);
#endif
		pt.x = 0;
		DPtoLP(hDC, &pt, 1);
		POINT ptOrg = { 0, 0 };
		DPtoLP(hDC, &ptOrg, 1);
		nPointSize = -abs(pt.y - ptOrg.y);
		ReleaseDC(HWND_DESKTOP, hDC);
	}
	return nPointSize;
}

bool CreatePointFontIndirect(CFont &rFont, const LOGFONT *lpLogFont)
{
	LOGFONT logFont = *lpLogFont;
	logFont.lfHeight = FontPointSizeToLogUnits(logFont.lfHeight);
	return rFont.CreateFontIndirect(&logFont) != FALSE;
}

bool CreatePointFont(CFont &rFont, int nPointSize, LPCTSTR lpszFaceName)
{
	LOGFONT logFont = {0};
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = nPointSize;
	_tcsncpy(logFont.lfFaceName, lpszFaceName, _countof(logFont.lfFaceName));
	return CreatePointFontIndirect(rFont, &logFont);
}

bool IsUnicodeFile(LPCTSTR pszFilePath)
{
	bool bResult = false;
	FILE* fp = _tfsopen(pszFilePath, _T("rb"), _SH_DENYWR);
	if (fp != NULL)
	{
		WORD wBOM = 0;
		bResult = (fread(&wBOM, sizeof(wBOM), 1, fp) == 1 && wBOM == 0xFEFF);
		fclose(fp);
	}
	return bResult;
}

bool RegularExpressionMatch(CString regexpr, CString teststring) {
	
	CAtlRegExp<> reFN;

	REParseError status = reFN.Parse( regexpr );
	if (REPARSE_ERROR_OK != status)
	{
		// Unexpected error.
		return false;
	}

	CAtlREMatchContext<> mcUrl;
	if (!reFN.Match(
		teststring,
		&mcUrl))
	{
		// Unexpected error.
		return false;
	} else {
		return true;
	}
}

ULONGLONG GetModuleVersion(LPCTSTR pszFilePath)
{
	ULONGLONG ullVersion = 0;
	DWORD dwUnused;
	DWORD dwVerInfSize = GetFileVersionInfoSize(const_cast<LPTSTR>(pszFilePath), &dwUnused);
	if (dwVerInfSize != 0)
	{
		LPBYTE pucVerInf = (LPBYTE)calloc(dwVerInfSize, 1);
		if (pucVerInf)
		{
			if (GetFileVersionInfo(const_cast<LPTSTR>(pszFilePath), 0, dwVerInfSize, pucVerInf))
			{
				VS_FIXEDFILEINFO* pFileInf = NULL;
				UINT uLen = 0;
				if (VerQueryValue(pucVerInf, _T("\\"), (LPVOID*)&pFileInf, &uLen) && pFileInf && uLen)
				{
					ullVersion = MAKEDLLVERULL(HIWORD(pFileInf->dwFileVersionMS), LOWORD(pFileInf->dwFileVersionMS), 
											   HIWORD(pFileInf->dwFileVersionLS), LOWORD(pFileInf->dwFileVersionLS));
				}
			}
			free(pucVerInf);
		}
	}
	return ullVersion;
}

ULONGLONG GetModuleVersion(HMODULE hModule)
{
	TCHAR szFilePath[MAX_PATH];
	DWORD dwModPathLen = GetModuleFileName(hModule, szFilePath, _countof(szFilePath));
	if (dwModPathLen == 0 || dwModPathLen == _countof(szFilePath))
		return 0;
	return GetModuleVersion(szFilePath);
}

bool DoCollectionRegFix(bool checkOnly)
{
	bool bGlobalSet = false;
	CRegKey regkey;
	LONG result;
	TCHAR modbuffer[MAX_PATH];
	DWORD dwModPathLen = ::GetModuleFileName(NULL, modbuffer, _countof(modbuffer));
	if (dwModPathLen == 0 || dwModPathLen == _countof(modbuffer))
		return false;
	CString strCanonFileName = modbuffer;
	strCanonFileName.Replace(_T("%"), _T("%%"));
	CString regbuffer;
	regbuffer.Format(_T("\"%s\" \"%%1\""), strCanonFileName);

	// first check if the registry keys are already set (either by installer in HKLM or by user in HKCU)
	result = regkey.Open(HKEY_CLASSES_ROOT, _T("eMule\\shell\\open\\command"), KEY_READ);
	if (result == ERROR_SUCCESS)
	{
		TCHAR rbuffer[MAX_PATH + 100];
		ULONG maxsize = _countof(rbuffer);
		regkey.QueryStringValue(NULL, rbuffer, &maxsize);
		rbuffer[_countof(rbuffer) - 1] = _T('\0');
		if (maxsize != 0 && _tcsicmp(rbuffer, regbuffer) == 0)
			bGlobalSet = true; // yup, globally we have an entry for this mule
		regkey.Close();
	}

	if (!bGlobalSet)
	{
		// we actually need to change the registry and write an entry for HKCU
		if (checkOnly)
			return true;
		//HKEY hkeyCR = thePrefs.GetWindowsVersion() < _WINVER_2K_ ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
		if (regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\eMule\\shell\\open\\command")) == ERROR_SUCCESS)
		{
			VERIFY( regkey.SetStringValue(NULL, regbuffer) == ERROR_SUCCESS );

			VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\eMule\\DefaultIcon")) == ERROR_SUCCESS );
			VERIFY( regkey.SetStringValue(NULL, CString(modbuffer) + _T(",1")) == ERROR_SUCCESS );

			VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\eMule")) == ERROR_SUCCESS );
			VERIFY( regkey.SetStringValue(NULL, _T("eMule Collection File")) == ERROR_SUCCESS );

			VERIFY( regkey.Open(HKEY_CURRENT_USER, _T("Software\\Classes\\eMule\\shell\\open")) == ERROR_SUCCESS );
			regkey.RecurseDeleteKey(_T("ddexec"));
			regkey.RecurseDeleteKey(_T("ddeexec"));

			VERIFY( regkey.Create(HKEY_CURRENT_USER, _T("Software\\Classes\\") COLLECTION_FILEEXTENSION) == ERROR_SUCCESS );
			VERIFY( regkey.SetStringValue(NULL, _T("eMule")) == ERROR_SUCCESS );
			VERIFY( regkey.Close() == ERROR_SUCCESS );
		}
		else
			ASSERT(0);
	}
	return false;
}

inline void swap_byte (uint8&a, uint8&b){
	uint8 bySwap;
	bySwap = a;
	a = b;
	b = bySwap;
}

RC4_Key_Struct* RC4CreateKey(const uchar* const pachKeyData, const size_t nLen, RC4_Key_Struct* key, const bool bSkipDiscard){
	uint8 index1;
	uint8 index2;
	uint8* pabyState;

	if (key == NULL)
		key = new RC4_Key_Struct;

	pabyState= &key->abyState[0];
	for (size_t i = 0; i < 256; i++)
		pabyState[i] = (uint8)i;

	key->byX = 0;
	key->byY = 0;
	index1 = 0;
	index2 = 0;
	for (size_t i = 0; i < 256; i++){
		index2 = (pachKeyData[index1] + pabyState[i] + index2);
		swap_byte(pabyState[i], pabyState[index2]);
		index1 = (uint8)((index1 + 1) % nLen);
	}
	if (!bSkipDiscard)
		RC4Crypt(NULL, NULL, 1024, key);
	return key;
}

void RC4Crypt(const uchar* const pachIn, uchar* const pachOut, const size_t nLen, RC4_Key_Struct* const key){
	ASSERT( key != NULL && nLen > 0 );
	if (key == NULL)
		return;
	
	uint8 byX = key->byX;
	uint8 byY = key->byY;
	uint8* pabyState = &key->abyState[0];

	for (size_t i = 0; i < nLen; i++)
	{
		byX = (byX + 1);
		byY = (pabyState[byX] + byY);
		swap_byte(pabyState[byX], pabyState[byY]);
		
		if (pachIn != NULL)
		{
			uint8 byXorIndex = (pabyState[byX] + pabyState[byY]);
			pachOut[i] = pachIn[i] ^ pabyState[byXorIndex];
    }
    }
	key->byX = byX;
	key->byY = byY;
}

uint32 LevenshteinDistance(const CString& str1, const CString& str2)
{
	uint32 n1 = str1.GetLength();
	uint32 n2 = str2.GetLength();

	uint32* p = new uint32[n2+1];
	uint32* q = new uint32[n2+1];
	uint32* r;

	p[0] = 0;
	for(uint32 j = 1; j <= n2; ++j)
		p[j] = p[j-1] + 1;

	for(uint32 i = 1; i <= n1; ++i)
	{
		q[0] = p[0] + 1;
		for(uint32 j = 1; j <= n2; ++j )
		{
			uint32 d_del = p[j] + 1;
			uint32 d_ins = q[j-1] + 1;
			uint32 d_sub = p[j-1] + (str1.GetAt(i-1) == str2.GetAt(j-1) ? 0 : 1);
			q[j] = min(min(d_del, d_ins), d_sub);
		}
		r = p;
		p = q;
		q = r;
	}

	uint32 tmp = p[n2];
	delete[] p;
	delete[] q;

	return tmp;
}

// Wrapper for _tmakepath which ensures that the outputbuffer does not exceed MAX_PATH
// using a smaller buffer  without checking the sizes prior calling this function is not safe
// If the resulting path would be bigger than MAX_PATH-1, it will be empty and return false (similar to PathCombine)
bool _tmakepathlimit(TCHAR *path, const TCHAR *drive, const TCHAR *dir, const TCHAR *fname, const TCHAR *ext){
	if (path == NULL){
		ASSERT( false );
		return false;
	}

	size_t nSize = 64; // the function should actually only add 4 (+1 nullbyte) bytes max extra
	if (drive != NULL)
		nSize += _tcsclen(drive);
	if (dir != NULL)
		nSize += _tcsclen(dir);
	if (fname != NULL)
		nSize += _tcsclen(fname);
	if (ext != NULL)
		nSize += _tcsclen(ext);

	TCHAR* tchBuffer = new TCHAR[nSize];
	_tmakepath(tchBuffer, drive, dir, fname, ext);

	if (_tcslen(tchBuffer) >= MAX_PATH){
		path[0] = _T('\0');
		ASSERT( false );
		delete[] tchBuffer;
		return false;
	}
	else{
		_tcscpy(path, tchBuffer);
		delete[] tchBuffer;
		return true;
	}
}

uint8 GetMyConnectOptions(bool bEncryption, bool bCallback){
	// Connect options Tag
	// 4 Reserved (!)
	// 1 Direct Callback
	// 1 CryptLayer Required
	// 1 CryptLayer Requested
	// 1 CryptLayer Supported
	const uint8 uSupportsCryptLayer	= (thePrefs.IsClientCryptLayerSupported() && bEncryption) ? 1 : 0;
	const uint8 uRequestsCryptLayer	= (thePrefs.IsClientCryptLayerRequested() && bEncryption) ? 1 : 0;
	const uint8 uRequiresCryptLayer	= (thePrefs.IsClientCryptLayerRequired() && bEncryption) ? 1 : 0;
	// direct callback is only possible if connected to kad, tcp firewalled and verified UDP open (for example on a full cone NAT)
	const uint8 uDirectUDPCallback	= (bCallback && theApp.IsFirewalled() && Kademlia::CKademlia::IsRunning() && !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified()) ? 1 : 0;
	
	const uint8 byCryptOptions = (uDirectUDPCallback << 3) | (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
	return byCryptOptions;
}
/*
bool AddIconGrayscaledToImageList(CImageList& rList, HICON hIcon)
{
	// Use to create grayscaled alpha using icons on WinXP and lower
	// Only works with edited CxImage lib, not 6.0 standard
	bool bResult = false;
	ICONINFO iinfo;
	if (GetIconInfo(hIcon, &iinfo))
	{
		CxImage cxGray;
		if (cxGray.CreateFromHBITMAP(iinfo.hbmColor))
		{
			cxGray.GrayScale();
			HBITMAP hGrayBmp = cxGray.MakeBitmap(NULL, true);
			bResult = rList.Add(CBitmap::FromHandle(hGrayBmp), CBitmap::FromHandle(iinfo.hbmMask)) != (-1);
			DeleteObject(hGrayBmp);
		}
		DeleteObject(iinfo.hbmColor);
		DeleteObject(iinfo.hbmMask);
	}
	return bResult;
}
*/
// SLUGFILLER: heapsortCompletesrc
void HeapSort(CAtlArray<uint16> &count, size_t first, size_t last){
	size_t r;
	for ( r = first; !(r & (size_t)INT_PTR_MIN) && (r<<1) < last; ){
		size_t r2 = (r<<1)+1;
		if (r2 != last)
			if (count[r2] < count[r2+1])
				r2++;
		if (count[r] < count[r2]){
			uint16 t = count[r2];
			count[r2] = count[r];
			count[r] = t;
			r = r2;
		}
		else
			break;
	}
}
// SLUGFILLER: heapsortCompletesrc
int __cdecl __ascii_stricmp (
        const char * dst,
        const char * src
        )
{
    int f, l;

    do
    {
        if ( ((f = (unsigned char)(*(dst++))) >= 'A') && (f <= 'Z') )
            f -= 'A' - 'a';
        if ( ((l = (unsigned char)(*(src++))) >= 'A') && (l <= 'Z') )
            l -= 'A' - 'a';
    }
    while ( f && (f == l) );

    return(f - l);
}

sint_ptr charhexval(uint_ptr c){
	if(c>='0' && c<='9')
		return c-_T('0');
	if(c>='A' && c<='F')
		return c-_T('A')+10;
	if(c>='a' && c<='f')
		return c-'a'+10;
	return -1;
}

bool extInList(LPCTSTR extlist,LPCTSTR ext){
	if ( !*ext)
		return false;

	while (*extlist){
		LPCTSTR s1 = extlist;
		LPCTSTR s2 = ext;

		while ( *s1 && *s2 && !(*s1-((*s2>=L'A' && *s2<=L'Z')?*s2+L'a'-L'A':*s2)) )
			s1++, s2++;

		if (!*s2 && (*s1 == _T('|') || *s1 == 0))
			return true;

		while (*s1 != _T('|')){
			if(*s1++ == NULL)
				return false;
		}
		extlist = s1+1;
	}
	return false;
}

bool checkExt(LPCTSTR filename,LPCTSTR extlist){
	if(*extlist == 0)	return false;
	const TCHAR* ext = _tcsrchr(filename,_T('.'));
	if(ext != NULL)
		return extInList(extlist,ext);
	return false;
}

float GetMaxSlotSpeed(float maxUp)
{
	if (maxUp<6)
		return 2.0f;
	if (maxUp<10)
		return 3.0f;
	float maxSlotSpeed = maxUp/(3+(maxUp-10)/20.0f);
	if (maxSlotSpeed>XTREME_MAX_SLOTSPEED)
		return XTREME_MAX_SLOTSPEED;
	return maxSlotSpeed;
}

uint32 strtoul(const char*_src, const char**_dest){
	const char*_srcbase = _src;
	uint_ptr val = 0;
#ifdef SUPPORTSIGNED
	bool issigned = false;
#endif
	for(; *_src != 0; ++_src){
		if(*_src >= '0' && *_src <= '9'){
			val = val*10 + (*_src-'0');
			_srcbase = NULL;
		}
		else if(!_srcbase)
			break;
#ifdef SUPPORTSIGNED
		else if(*_src == '-')
			issigned = !issigned;
		else if(*_src == '+')
			;
#endif
		else if(*_src == ' ' || *_src == '\t')
			;
		else
			break;
	}
	if(_dest)
		*_dest = _srcbase ? _srcbase : _src;
#ifdef SUPPORTSIGNED
	return issigned ? -val : val;
#else
	return (uint32)val;
#endif
}

uint32 wcstoul(const wchar_t*_src, const wchar_t**_dest){
	const wchar_t*_srcbase = _src;
	uint_ptr val = 0;
#ifdef SUPPORTSIGNED
	bool issigned = false;
#endif
	for(; *_src != 0; ++_src){
		if(*_src >= L'0' && *_src <= L'9'){
			val = val*10 + (*_src-L'0');
			_srcbase = NULL;
		}
		else if(!_srcbase)
			break;
#ifdef SUPPORTSIGNED
		else if(*_src == L'-')
			issigned = !issigned;
		else if(*_src == L'+')
			;
#endif
		else if(*_src == L' ' || *_src == L'\t')
			;
		else
			break;
	}
	if(_dest)
		*_dest = _srcbase ? _srcbase : _src;
#ifdef SUPPORTSIGNED
	return issigned ? -val : val;
#else
	return (uint32)val;
#endif
}

char* inet_addr_ntohl(char*_src, uint32& ip){
	BYTE* _ip = (BYTE*)&ip+3;
	while(true){
		char * pnext;
		uint32 val = strtoul(_src, (const char**)&pnext);
		if(_src == pnext || val > 0xff)
			return NULL;
		*_ip = (BYTE)val;
		if(*pnext=='.'){
			if(_ip == (BYTE*)&ip)
				return NULL;
			_src = pnext+1;
			--_ip;
		}
		else
			return (_ip == (BYTE*)&ip && (*pnext == ' ' || *pnext==0 || *pnext==',' || *pnext=='-'))?pnext:NULL;
	}
}

uint32 GetHostIP()
{
	// Using 'gethostname/gethostbyname' does not solve the problem when we have more than 
	// one IP address. Using 'gethostname/gethostbyname' even seems to return the last IP 
	// address which we got. e.g. if we already got an IP from our ISP, 
	// 'gethostname/gethostbyname' will returned that (primary) IP, but if we add another
	// IP by opening a VPN connection, 'gethostname' will still return the same hostname, 
	// but 'gethostbyname' will return the 2nd IP.
	// To weaken that problem at least for users which are binding eMule to a certain IP,
	// we use the explicitly specified bind address as our local IP address.
	if (thePrefs.GetBindAddrA() != NULL) {
		unsigned long ulBindAddr = inet_addr(thePrefs.GetBindAddrA());
		if (ulBindAddr != INADDR_ANY && ulBindAddr != INADDR_NONE)
			return ulBindAddr;
	}

	DWORD best_if_index;
	GetBestInterface(/*inet_addr("223.255.255.255")*/0xffffffdf, &best_if_index);

	PMIB_IPADDRTABLE ip_addr_table;
	char buffer[1024];
	ip_addr_table = (PMIB_IPADDRTABLE)buffer;
	DWORD size = sizeof(buffer);
	GetIpAddrTable(ip_addr_table, &size, 0);
	DWORD local_ip = 0;
	for (DWORD i=0; i<ip_addr_table->dwNumEntries; i++) {
		if (ip_addr_table->table[i].dwIndex == best_if_index)
			return ip_addr_table->table[i].dwAddr;
	}
	// Don't use 'gethostbyname(NULL)'. The winsock DLL may be replaced by a DLL from a third party
	// which is not fully compatible to the original winsock DLL. ppl reported crash with SCORSOCK.DLL
	// when using 'gethostbyname(NULL)'.
	__try{
		char szHost[256];
		if (gethostname(szHost, sizeof szHost) == 0){
			hostent* pHostEnt = gethostbyname(szHost);
			if (pHostEnt != NULL && pHostEnt->h_length == 4 && pHostEnt->h_addr_list[0] != NULL)
				return *((uint32*)pHostEnt->h_addr_list[0]);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		// at least two ppl reported crashs when using 'gethostbyname' with third party winsock DLLs
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Unknown exception in GetHostIP"));
		ASSERT(0);
	}
	return 0;
}

// We have to add gz-header and some other stuff
// to standard zlib functions
// in order to use gzip in web pages
int GZipCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level)
{
	const static int gz_magic[2] = {0x1f, 0x8b}; // gzip magic header
	int err;
	uLong crc;
	z_stream stream/* = {0}*/;
	memset(&stream, 0, sizeof(z_stream));
	/*stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;*/
	crc = crc32(0L, Z_NULL, 0);
	// init Zlib stream
	// NOTE windowBits is passed < 0 to suppress zlib header
	err = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
	if (err != Z_OK)
		return err;

	sprintf((char*)dest , "%c%c%c%c%c%c%c%c%c%c", gz_magic[0], gz_magic[1],
		Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, 255);
	// wire buffers
	stream.next_in = (Bytef*) source ;
	stream.avail_in = (uInt)sourceLen;
	stream.next_out = ((Bytef*) dest) + 10;
	stream.avail_out = *destLen - 18;
	// doit
	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&stream);
		return err;
	}
	err = deflateEnd(&stream);
	crc = crc32(crc, (const Bytef *) source ,  sourceLen );
	//CRC
	*(((Bytef*) dest)+10+stream.total_out) = (Bytef)(crc & 0xFF);
	*(((Bytef*) dest)+10+stream.total_out+1) = (Bytef)((crc>>8) & 0xFF);
	*(((Bytef*) dest)+10+stream.total_out+2) = (Bytef)((crc>>16) & 0xFF);
	*(((Bytef*) dest)+10+stream.total_out+3) = (Bytef)((crc>>24) & 0xFF);
	// Length
	*(((Bytef*) dest)+10+stream.total_out+4) = (Bytef)( sourceLen  & 0xFF);
	*(((Bytef*) dest)+10+stream.total_out+5) = (Bytef)(( sourceLen >>8) & 0xFF);
	*(((Bytef*) dest)+10+stream.total_out+6) = (Bytef)(( sourceLen >>16) &	0xFF);
	*(((Bytef*) dest)+10+stream.total_out+7) = (Bytef)(( sourceLen >>24) &	0xFF);
	// return  destLength
	*destLen = 10 + stream.total_out + 8;

	return err;
}
//GZip file
//first 10 bytes:1f8b0808
//last 8 bytes:crc+filesize
int GZipUncompress(Bytef **dest, uLongf *destLen, const Bytef *source, uLong sourceLen){
	if(source[0] != 0x1f || source[1] != 0x8b || source[2] != 8 || source[3] != 8)
		return -1;
	// skip file name
	unsigned char* datastart = (unsigned char*)memchr(source + 10, 0, sourceLen - 10) + 1;
	if(datastart == NULL)
		return -1;
	sint_ptr datasize = sourceLen + (source - datastart) - 8;
	if(datasize<0)
		return -1;
	try
	{
		*destLen = *(uLongf*)(source+sourceLen-4);
		*dest = new unsigned char[*destLen];
		z_stream stream/* = {0}*/;
		memset(&stream, 0, sizeof(z_stream));
		// wire buffers
		stream.next_in = (Bytef*) datastart;
		stream.avail_in = (uInt)sourceLen;
		stream.next_out = ((Bytef*) *dest);
		stream.avail_out = *destLen;
		// init Zlib stream
		// NOTE windowBits is passed < 0 to suppress zlib header
		int err = inflateInit2(&stream, -MAX_WBITS);
		if (err != Z_OK){
			delete[] dest;
			return err;
		}
		// doit
		err = inflate(&stream, Z_FINISH);
		if (err != Z_STREAM_END){
			inflateEnd(&stream);
			delete[] dest;
			return err;
		}
	}
	catch(...)
	{
		ASSERT(0);
		delete[] dest;
	}
	return 0;
}

typedef DWORD           FOURCC;         /* a four character code */
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

BOOL IsEqualFOURCC(FOURCC fccA, FOURCC fccB)
{
	for (int i = 0; i < 4; i++)
	{
		if (tolower((unsigned char)fccA) != tolower((unsigned char)fccB))
			return FALSE;
		fccA >>= 8;
		fccB >>= 8;
	}
	return TRUE;
}

CString GetVideoFormatDisplayName(DWORD biCompression)
{
	CString strFormat;
	     if (biCompression == BI_RGB)									strFormat = _T("RGB (Uncompressed)");
	else if (biCompression == BI_RLE8)									strFormat = _T("RLE8 (Run Length Encoded 8-bit)");
	else if (biCompression == BI_RLE4)									strFormat = _T("RLE4 (Run Length Encoded 4-bit)");
	else if (biCompression == BI_BITFIELDS)								strFormat = _T("Bitfields");
	else if (biCompression == BI_JPEG)									strFormat = _T("JPEG");
	else if (biCompression == BI_PNG)									strFormat = _T("PNG");
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('D','I','V','3')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (DivX ;-) MPEG-4 v3 Low)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('D','I','V','4')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (DivX ;-) MPEG-4 v3 Fast)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('D','I','V','X')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (DivX 4)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('D','X','5','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (DivX 5)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('M','P','G','4')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Microsoft MPEG-4 v1)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('M','P','4','2')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Microsoft MPEG-4 v2)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('M','P','4','3')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Microsoft MPEG-4 v3)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('D','X','S','B')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Subtitle)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('W','M','V','1')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Windows Media Video 7)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('W','M','V','2')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Windows Media Video 8)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('W','M','V','3')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Windows Media Video 9)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('W','M','V','A')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Windows Media Video 9 Advanced Profile)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('R','V','1','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Real Video 5)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('R','V','1','3')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Real Video 5)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('R','V','2','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Real Video G2)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('R','V','3','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Real Video 8)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('R','V','4','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Real Video 9)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('H','2','6','4')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (MPEG-4 AVC)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('X','2','6','4')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (x264 MPEG-4 AVC)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('X','V','I','D')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Xvid MPEG-4)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('T','S','C','C')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (TechSmith Screen Capture)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('M','J','P','G')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (M-JPEG)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('I','V','3','2')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Intel Indeo Video 3.2)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('I','V','4','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Intel Indeo Video 4.0)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('I','V','5','0')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Intel Indeo Video 5.0)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('F','M','P','4')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (MPEG-4)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('C','V','I','D')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Cinepack)";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('C','R','A','M')))	strFormat = CStringA((LPCSTR)&biCompression, 4) + " (Microsoft Video 1)";
	// X-Ray :: MoreFourCCCodes :: Start
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('A','V','C','1'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('D','A','V','C'))
		//||	 IsEqualFOURCC(biCompression, MAKEFOURCC('H','2','6','4'))
		//||	 IsEqualFOURCC(biCompression, MAKEFOURCC('X','2','6','4'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('V','S','S','H'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('V','S','S','W')))
		strFormat = CStringA((LPCSTR)&biCompression, 4) + " H.264/MPEG-4 AVC";
	else if (IsEqualFOURCC(biCompression, MAKEFOURCC('3','I','V','0'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('3','I','V','1'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('3','I','V','2'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('3','I','V','D'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('3','I','V','X'))
		||	 IsEqualFOURCC(biCompression, MAKEFOURCC('3','V','I', 'D')))
		strFormat = CStringA((LPCSTR)&biCompression, 4) + " 3ivX";
	// X-Ray :: MoreFourCCCodes :: End
	return strFormat;
}

const static struct
{
	UINT	uFmtTag;
	LPCTSTR	pszDefine;
	LPCTSTR	pszComment;
} s_WavFmtTag[] =
{
// START: Codecs from Windows SDK "mmreg.h" file
	{ 0x0000, _T(""),						_T("Unknown") },
	{ 0x0001, _T("PCM"),					_T("Uncompressed") },
	{ 0x0002, _T("ADPCM"),					_T("") },
	{ 0x0003, _T("IEEE_FLOAT"),				_T("") },
	{ 0x0004, _T("VSELP"),					_T("Compaq Computer Corp.") },
	{ 0x0005, _T("IBM_CVSD"),				_T("") },
	{ 0x0006, _T("ALAW"),					_T("") },
	{ 0x0007, _T("MULAW"),					_T("") },
	{ 0x0008, _T("DTS"),					_T("Digital Theater Systems") },
	{ 0x0009, _T("DRM"),					_T("") },
	{ 0x000A, _T("WMAVOICE9"),				_T("") },
	{ 0x000B, _T("WMAVOICE10"),				_T("") },
	{ 0x0010, _T("OKI_ADPCM"),				_T("") },
	{ 0x0011, _T("DVI_ADPCM"),				_T("Intel Corporation") },
	{ 0x0012, _T("MEDIASPACE_ADPCM"),		_T("Videologic") },
	{ 0x0013, _T("SIERRA_ADPCM"),			_T("") },
	{ 0x0014, _T("G723_ADPCM"),				_T("Antex Electronics Corporation") },
	{ 0x0015, _T("DIGISTD"),				_T("DSP Solutions, Inc.") },
	{ 0x0016, _T("DIGIFIX"),				_T("DSP Solutions, Inc.") },
	{ 0x0017, _T("DIALOGIC_OKI_ADPCM"),		_T("") },
	{ 0x0018, _T("MEDIAVISION_ADPCM"),		_T("") },
	{ 0x0019, _T("CU_CODEC"),				_T("Hewlett-Packard Company") },
	{ 0x0020, _T("YAMAHA_ADPCM"),			_T("") },
	{ 0x0021, _T("SONARC"),					_T("Speech Compression") },
	{ 0x0022, _T("DSPGROUP_TRUESPEECH"),	_T("") },
	{ 0x0023, _T("ECHOSC1"),				_T("Echo Speech Corporation") },
	{ 0x0024, _T("AUDIOFILE_AF36"),			_T("Virtual Music, Inc.") },
	{ 0x0025, _T("APTX"),					_T("Audio Processing Technology") },
	{ 0x0026, _T("AUDIOFILE_AF10"),			_T("Virtual Music, Inc.") },
	{ 0x0027, _T("PROSODY_1612"),			_T("Aculab plc") },
	{ 0x0028, _T("LRC"),					_T("Merging Technologies S.A.") },
	{ 0x0030, _T("DOLBY_AC2"),				_T("") },
	{ 0x0031, _T("GSM610"),					_T("") },
	{ 0x0032, _T("MSNAUDIO"),				_T("") },
	{ 0x0033, _T("ANTEX_ADPCME"),			_T("") },
	{ 0x0034, _T("CONTROL_RES_VQLPC"),		_T("") },
	{ 0x0035, _T("DIGIREAL"),				_T("DSP Solutions, Inc.") },
	{ 0x0036, _T("DIGIADPCM"),				_T("DSP Solutions, Inc.") },
	{ 0x0037, _T("CONTROL_RES_CR10"),		_T("") },
	{ 0x0038, _T("NMS_VBXADPCM"),			_T("Natural MicroSystems") },
	{ 0x0039, _T("CS_IMAADPCM"),			_T("Crystal Semiconductor IMA ADPCM") },
	{ 0x003A, _T("ECHOSC3"),				_T("Echo Speech Corporation") },
	{ 0x003B, _T("ROCKWELL_ADPCM"),			_T("") },
	{ 0x003C, _T("ROCKWELL_DIGITALK"),		_T("") },
	{ 0x003D, _T("XEBEC"),					_T("") },
	{ 0x0040, _T("G721_ADPCM"),				_T("Antex Electronics Corporation") },
	{ 0x0041, _T("G728_CELP"),				_T("Antex Electronics Corporation") },
	{ 0x0042, _T("MSG723"),					_T("") },
	{ 0x0050, _T("MP1"),					_T("MPEG-1, Layer 1") },
	{ 0x0051, _T("MP2"),					_T("MPEG-1, Layer 2") },
	{ 0x0052, _T("RT24"),					_T("InSoft, Inc.") },
	{ 0x0053, _T("PAC"),					_T("InSoft, Inc.") },
	{ 0x0055, _T("MP3"),					_T("MPEG-1, Layer 3") },
	{ 0x0059, _T("LUCENT_G723"),			_T("") },
	{ 0x0060, _T("CIRRUS"),					_T("") },
	{ 0x0061, _T("ESPCM"),					_T("ESS Technology") },
	{ 0x0062, _T("VOXWARE"),				_T("") },
	{ 0x0063, _T("CANOPUS_ATRAC"),			_T("") },
	{ 0x0064, _T("G726_ADPCM"),				_T("APICOM") },
	{ 0x0065, _T("G722_ADPCM"),				_T("APICOM") },
	{ 0x0067, _T("DSAT_DISPLAY"),			_T("") },
	{ 0x0069, _T("VOXWARE_BYTE_ALIGNED"),	_T("") },
	{ 0x0070, _T("VOXWARE_AC8"),			_T("") },
	{ 0x0071, _T("VOXWARE_AC10"),			_T("") },
	{ 0x0072, _T("VOXWARE_AC16"),			_T("") },
	{ 0x0073, _T("VOXWARE_AC20"),			_T("") },
	{ 0x0074, _T("VOXWARE_RT24"),			_T("") },
	{ 0x0075, _T("VOXWARE_RT29"),			_T("") },
	{ 0x0076, _T("VOXWARE_RT29HW"),			_T("") },
	{ 0x0077, _T("VOXWARE_VR12"),			_T("") },
	{ 0x0078, _T("VOXWARE_VR18"),			_T("") },
	{ 0x0079, _T("VOXWARE_TQ40"),			_T("") },
	{ 0x0080, _T("SOFTSOUND"),				_T("") },
	{ 0x0081, _T("VOXWARE_TQ60"),			_T("") },
	{ 0x0082, _T("MSRT24"),					_T("") },
	{ 0x0083, _T("G729A"),					_T("AT&T Labs, Inc.") },
	{ 0x0084, _T("MVI_MVI2"),				_T("Motion Pixels") },
	{ 0x0085, _T("DF_G726"),				_T("DataFusion Systems (Pty) (Ltd)") },
	{ 0x0086, _T("DF_GSM610"),				_T("DataFusion Systems (Pty) (Ltd)") },
	{ 0x0088, _T("ISIAUDIO"),				_T("Iterated Systems, Inc.") },
	{ 0x0089, _T("ONLIVE"),					_T("") },
	{ 0x0091, _T("SBC24"),					_T("Siemens Business Communications Sys") },
	{ 0x0092, _T("DOLBY_AC3_SPDIF"),		_T("Sonic Foundry") },
	{ 0x0093, _T("MEDIASONIC_G723"),		_T("") },
	{ 0x0094, _T("PROSODY_8KBPS"),			_T("Aculab plc") },
	{ 0x0097, _T("ZYXEL_ADPCM"),			_T("") },
	{ 0x0098, _T("PHILIPS_LPCBB"),			_T("") },
	{ 0x0099, _T("PACKED"),					_T("Studer Professional Audio AG") },
	{ 0x00A0, _T("MALDEN_PHONYTALK"),		_T("") },
	{ 0x0100, _T("RHETOREX_ADPCM"),			_T("") },
	{ 0x0101, _T("IRAT"),					_T("BeCubed Software Inc.") },
	{ 0x0111, _T("VIVO_G723"),				_T("") },
	{ 0x0112, _T("VIVO_SIREN"),				_T("") },
	{ 0x0123, _T("DIGITAL_G723"),			_T("Digital Equipment Corporation") },
	{ 0x0125, _T("SANYO_LD_ADPCM"),			_T("") },
	{ 0x0130, _T("SIPROLAB_ACEPLNET"),		_T("") },
	{ 0x0130, _T("SIPR"),					_T("Real Audio 4 (Sipro)") },
	{ 0x0131, _T("SIPROLAB_ACELP4800"),		_T("") },
	{ 0x0132, _T("SIPROLAB_ACELP8V3"),		_T("") },
	{ 0x0133, _T("SIPROLAB_G729"),			_T("") },
	{ 0x0134, _T("SIPROLAB_G729A"),			_T("") },
	{ 0x0135, _T("SIPROLAB_KELVIN"),		_T("") },
	{ 0x0140, _T("G726ADPCM"),				_T("Dictaphone Corporation") },
	{ 0x0150, _T("QUALCOMM_PUREVOICE"),		_T("") },
	{ 0x0151, _T("QUALCOMM_HALFRATE"),		_T("") },
	{ 0x0155, _T("TUBGSM"),					_T("Ring Zero Systems, Inc.") },
	{ 0x0160, _T("MSAUDIO1"),				_T("Microsoft Audio") },
	{ 0x0161, _T("WMAUDIO2"),				_T("Windows Media Audio") },
	{ 0x0162, _T("WMAUDIO3"),				_T("Windows Media Audio 9 Pro") },
	{ 0x0163, _T("WMAUDIO_LOSSLESS"),		_T("Windows Media Audio 9 Lossless") },
	{ 0x0164, _T("WMASPDIF"),				_T("Windows Media Audio Pro-over-S/PDIF") },
	{ 0x0170, _T("UNISYS_NAP_ADPCM"),		_T("") },
	{ 0x0171, _T("UNISYS_NAP_ULAW"),		_T("") },
	{ 0x0172, _T("UNISYS_NAP_ALAW"),		_T("") },
	{ 0x0173, _T("UNISYS_NAP_16K"),			_T("") },
	{ 0x0200, _T("CREATIVE_ADPCM"),			_T("") },
	{ 0x0202, _T("CREATIVE_FASTSPEECH8"),	_T("") },
	{ 0x0203, _T("CREATIVE_FASTSPEECH10"),	_T("") },
	{ 0x0210, _T("UHER_ADPCM"),				_T("") },
	{ 0x0220, _T("QUARTERDECK"),			_T("") },
	{ 0x0230, _T("ILINK_VC"),				_T("I-link Worldwide") },
	{ 0x0240, _T("RAW_SPORT"),				_T("Aureal Semiconductor") },
	{ 0x0241, _T("ESST_AC3"),				_T("ESS Technology, Inc.") },
	{ 0x0250, _T("IPI_HSX"),				_T("Interactive Products, Inc.") },
	{ 0x0251, _T("IPI_RPELP"),				_T("Interactive Products, Inc.") },
	{ 0x0260, _T("CS2"),					_T("Consistent Software") },
	{ 0x0270, _T("SONY_SCX"),				_T("") },
	{ 0x0300, _T("FM_TOWNS_SND"),			_T("Fujitsu Corp.") },
	{ 0x0400, _T("BTV_DIGITAL"),			_T("Brooktree Corporation") },
	{ 0x0401, _T("IMC"),					_T("Intel Music Coder for MSACM") },
	{ 0x0450, _T("QDESIGN_MUSIC"),			_T("") },
	{ 0x0680, _T("VME_VMPCM"),				_T("AT&T Labs, Inc.") },
	{ 0x0681, _T("TPC"),					_T("AT&T Labs, Inc.") },
	{ 0x1000, _T("OLIGSM"),					_T("Olivetti") },
	{ 0x1001, _T("OLIADPCM"),				_T("Olivetti") },
	{ 0x1002, _T("OLICELP"),				_T("Olivetti") },
	{ 0x1003, _T("OLISBC"),					_T("Olivetti") },
	{ 0x1004, _T("OLIOPR"),					_T("Olivetti") },
	{ 0x1100, _T("LH_CODEC"),				_T("Lernout & Hauspie") },
	{ 0x1400, _T("NORRIS"),					_T("") },
	{ 0x1500, _T("SOUNDSPACE_MUSICOMPRESS"),_T("AT&T Labs, Inc.") },
	{ 0x1600, _T("MPEG_ADTS_AAC"),			_T("") },
	{ 0x1601, _T("MPEG_RAW_AAC"),			_T("") },
	{ 0x1608, _T("NOKIA_MPEG_ADTS_AAC"),	_T("") },
	{ 0x1609, _T("NOKIA_MPEG_RAW_AAC"),		_T("") },
	{ 0x160A, _T("VODAFONE_MPEG_ADTS_AAC"),	_T("") },
	{ 0x160B, _T("VODAFONE_MPEG_RAW_AAC"),	_T("") },
	{ 0x2000, _T("AC3"),					_T("Dolby AC3") },
// END: Codecs from Windows SDK "mmreg.h" file

	{ 0x2001, _T("DTS"),					_T("Digital Theater Systems") },

// Real Audio (Baked) codecs
	{ 0x2002, _T("RA14"),					_T("RealAudio 1/2 14.4") },
	{ 0x2003, _T("RA28"),					_T("RealAudio 1/2 28.8") },
	{ 0x2004, _T("COOK"),					_T("RealAudio G2/8 Cook (Low Bitrate)") },
	{ 0x2005, _T("DNET"),					_T("RealAudio 3/4/5 Music (DNET)") },
	{ 0x2006, _T("RAAC"),					_T("RealAudio 10 AAC (RAAC)") },
	{ 0x2007, _T("RACP"),					_T("RealAudio 10 AAC+ (RACP)") }
};

CString GetAudioFormatDisplayName(const CString &strCodecId)
{
	for (int i = 0; i < _countof(s_WavFmtTag); i++)
	{
		if (_tcsicmp(s_WavFmtTag[i].pszDefine, strCodecId) == 0)
		{
			if (s_WavFmtTag[i].uFmtTag == 0)
				break;
			if (s_WavFmtTag[i].pszDefine[0] == _T('\0') || s_WavFmtTag[i].pszComment[0] == _T('\0'))
				break;
			return CString(s_WavFmtTag[i].pszDefine) + _T(" (") + CString(s_WavFmtTag[i].pszComment) + _T(')');
		}
	}
	return _T("");
}

CString GetCodecDisplayName(const CString &strCodecId)
{
	static CMapStringToString s_mapCodecDisplayName;
	CString strCodecDisplayName;
	if (s_mapCodecDisplayName.Lookup(strCodecId, strCodecDisplayName))
		return strCodecDisplayName;

	if (strCodecId.GetLength() == 3 || strCodecId.GetLength() == 4)
	{
		bool bHaveFourCC = true;
		FOURCC fcc;
		if (strCodecId == L"rgb")
			fcc = BI_RGB;
		else if (strCodecId == L"rle8")
			fcc = BI_RLE8;
		else if (strCodecId == L"rle4")
			fcc = BI_RLE4;
		else if (strCodecId == L"jpeg")
			fcc = BI_JPEG;
		else if (strCodecId == L"png")
			fcc = BI_PNG;
		else
		{
			fcc = MAKEFOURCC(' ',' ',' ',' ');
			LPSTR pcFourCC = (LPSTR)&fcc;
			for (int i = 0; i < strCodecId.GetLength(); i++)
			{
				WCHAR wch = strCodecId[i];
				if (wch >= 0x100 || (!__iscsym((unsigned char)wch) && wch != L'.' && wch != L' ')) {
					bHaveFourCC = false;
					break;
				}
				pcFourCC[i] = (CHAR)toupper((unsigned char)wch);
			}
		}
		if (bHaveFourCC)
			strCodecDisplayName = GetVideoFormatDisplayName(fcc);
	}

	if (strCodecDisplayName.IsEmpty())
		strCodecDisplayName = GetAudioFormatDisplayName(strCodecId);
	if (strCodecDisplayName.IsEmpty())
	{
		strCodecDisplayName = strCodecId;
		strCodecDisplayName.MakeUpper();
	}
	s_mapCodecDisplayName.SetAt(strCodecId, strCodecDisplayName);
	return strCodecDisplayName;
}

//morph4u shutdown +
bool MySystemShutdown()
{
   HANDLE hToken; 
   TOKEN_PRIVILEGES tkp; 
 
   // Get a token for this process. 
 
   if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
      return( FALSE ); 
 
   // Get the LUID for the shutdown privilege. 
 
   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
 
   tkp.PrivilegeCount = 1;  // one privilege to set    
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
   // Get the shutdown privilege for this process. 
 
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
 
   if (GetLastError() != ERROR_SUCCESS) 
      return FALSE; 
 
   // Shut down the system and force all applications to close. 
 
   if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 
               SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
               SHTDN_REASON_MINOR_UPGRADE |
               SHTDN_REASON_FLAG_PLANNED)) 
      return FALSE; 

   return TRUE;
}
//morph4u shutdown -