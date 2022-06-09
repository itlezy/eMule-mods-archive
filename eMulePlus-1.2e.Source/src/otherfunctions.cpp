//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "otherfunctions.h"
#include "updownclient.h"
#ifndef NEW_SOCKETS_ENGINE
#include "emule.h"
#endif //NEW_SOCKETS_ENGINE
#include "memcpy_amd.h"
#include "StringConversion.h"
#include <share.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const TCHAR g_acHexDigits[16] =
{
	_T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'),
	_T('8'), _T('9'), _T('A'), _T('B'), _T('C'), _T('D'), _T('E'), _T('F')
};

#if 0	// Can be required later for AICH
static const byte base32Chars[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
#endif

CString __stdcall CastItoXBytes(uint64 qwNum)
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	CString		strBuffer;
	double		dNum;

	if (qwNum <= 0xFFFFFFFFui64)
	{
		uint32	dwNum = static_cast<uint32>(qwNum);

		if (dwNum < 1024)
			strBuffer.Format(_T("%u %s"), dwNum, GetResString(IDS_BYTES));
		else
		{
			dNum = static_cast<double>(dwNum);
			if (dwNum < (1024 * 1024))
				strBuffer.Format( (g_App.m_pPrefs->ShowRoundSizes()) ? _T("%.f %s") : _T("%.2f %s"),
					dNum / 1024.0, GetResString(IDS_KBYTES) );
			else if (dwNum < (1024 * 1024 * 1024))
				strBuffer.Format(_T("%.2f %s"), dNum / (1024.0 * 1024.0), GetResString(IDS_MBYTES));
			else
				strBuffer.Format(_T("%.2f %s"), dNum / (1024.0 * 1024.0 * 1024.0), GetResString(IDS_GBYTES));
		}
	}
	else
	{
		dNum = static_cast<double>(qwNum);
		if (qwNum < 1099511627776)
			strBuffer.Format(_T("%.2f %s"), dNum / (1024.0 * 1024.0 * 1024.0), GetResString(IDS_GBYTES));
		else
			strBuffer.Format(_T("%.3f %s"), dNum / 1099511627776.0, GetResString(IDS_TBYTES));
	}

	if (g_App.m_pPrefs->ShowRoundSizes())
	{
		CString	strDecimalPoint = GetLocalDecimalPoint();
		int		iPos2, iPos1 = strBuffer.Find(strDecimalPoint);

		if (iPos1 >= 0)
		{
			iPos2 = strBuffer.Find(_T(' '), iPos1 + strDecimalPoint.GetLength());
			if (iPos2 >= 0)
			{
				while (strBuffer.GetAt(--iPos2) == _T('0'))
					strBuffer.Delete(iPos2, 1);
				if (iPos2 == iPos1)
					strBuffer.Delete(iPos1, strDecimalPoint.GetLength());
			}
		}
	}

	return strBuffer;
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH2

	return _T("");
}

CString __stdcall CastItoIShort(uint64 qwCount)
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	CString	strBuffer;
	double	dNum;

	if (qwCount < 1000000000)
	{
		uint32	dwNum = static_cast<uint32>(qwCount);

		if (dwNum < 1000)
			strBuffer.Format(_T("%u"), dwNum);
		else
		{
			dNum = static_cast<double>(dwNum);
			if (dwNum < 1000000)
				strBuffer.Format(g_App.m_pPrefs->ShowRoundSizes() ? _T("%.f%s") : _T("%.2f%s"), dNum / 1000.0, GetResString(IDS_KILO));
			else
				strBuffer.Format(_T("%.2f%s"), dNum / 1000000.0, GetResString(IDS_MEGA));
		}
	}
	else
	{
		dNum = static_cast<double>(qwCount);
		if (qwCount < 1000000000000)
			strBuffer.Format(_T("%.2f%s"), dNum / 1000000000.0, GetResString(IDS_GIGA));
		else if (qwCount < 1000000000000000)
			strBuffer.Format(_T("%.2f%s"), dNum / 1000000000000.0, GetResString(IDS_TERRA));
	}

	return strBuffer;
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH2

	return _T("");
}

CString __stdcall CastSecondsToHM(sint32 iCount)
{
#ifndef NEW_SOCKETS_ENGINE
	CString	strBuffer;

	if (iCount < 0)
		strBuffer = _T("?");
	else if (iCount < 60)
		strBuffer.Format(_T("%u %s"), iCount, GetResString(IDS_SECS));
	else if (iCount < 3600)
		strBuffer.Format(_T("%u:%02u %s"), iCount / 60, iCount - (iCount / 60) * 60, GetResString(IDS_MINS));
	else if (iCount < 86400)
		strBuffer.Format(_T("%u:%02u %s"), iCount / 3600, (iCount - (iCount / 3600) * 3600) / 60, GetResString(IDS_HOURS));
	else
		strBuffer.Format(_T("%u %s %u %s"), iCount / 86400, GetResString(IDS_DAYS),  (iCount - (iCount / 86400) * 86400) / 3600, GetResString(IDS_HOURS));

	return strBuffer;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}

CString __stdcall CastSecondsToLngHM(uint32 dwSecs)
{
#ifndef NEW_SOCKETS_ENGINE
	CString	strBuffer;

	if (dwSecs < 86400)
	{
		if (dwSecs < 3600)
		{
			if (dwSecs < 60)
				strBuffer.Format(_T("%u %s"), dwSecs, GetResString(IDS_LONGSECS));
			else
				strBuffer.Format(_T("%u:%02u %s"), dwSecs / 60, static_cast<uint32>(dwSecs - (dwSecs / 60) * 60), GetResString(IDS_LONGMINS));
		}
		else
			strBuffer.Format(_T("%u:%02u %s"), dwSecs / 3600, static_cast<uint32>((dwSecs - (dwSecs / 3600) * 3600) / 60), GetResString(IDS_LONGHRS));
	}
	else
	{
		uint32	dwCntDays = dwSecs / 86400;
		uint32	dwCntHrs = (dwSecs - dwCntDays * 86400u) / 3600u;
		uint32	dwCntMins = ((dwSecs - (dwCntDays * 86400u)) - (dwCntHrs * 3600u)) / 60u;

		CString strDay = GetResString((dwCntDays > 1) ? IDS_LONGDAYS : IDS_LONGDAY);

		if (dwCntHrs != 0)
			strBuffer.Format(_T("%u %s %u:%02u %s"), dwCntDays, strDay, dwCntHrs, dwCntMins, GetResString(IDS_LONGHRS));
		else
			strBuffer.Format(_T("%u %s %02u %s"), dwCntDays, strDay, dwCntMins, GetResString(IDS_LONGMINS));
	}
	return strBuffer;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}

// Credits: adapted from http://www.codeguru.com/string/long2string.html
CString __stdcall CastItoThousands(sint64 iVal)
{
	EMULE_TRY

	CString		strBuffer;
	uint32		dwDigVal;
	CString		strThousandSeparator = GetLocalThousandsSep();
	int			iIndex, iDecimalPos = 0;

	if (iVal < 0)
	{
		strBuffer = GetLocalNegativeSign();
		iVal = -iVal;
	}
	iIndex = strBuffer.GetLength();

	do
	{
		if (++iDecimalPos != 4)
		{
			dwDigVal = (uint32)(iVal % 10);
			iVal /= 10;
			strBuffer.Insert(iIndex, static_cast<TCHAR>(dwDigVal + _T('0')));
		}
		else
		{
			strBuffer.Insert(iIndex, strThousandSeparator);
			iDecimalPos = 0;
		}
	} while (iVal > 0);

	return strBuffer;

	EMULE_CATCH2

	return _T("");
}

void ShellOpenFile(const CString& name)
{
	EMULE_TRY
	ShellExecute(NULL, _T("open"), name, NULL, NULL, SW_SHOW);
	EMULE_CATCH
}

CString __stdcall GetResString(UINT dwStringID)
{
#ifndef NEW_SOCKETS_ENGINE
	HINSTANCE	hMod = ::GetModuleHandle(NULL);
	CString		strResString;

	if (!strResString.LoadString(hMod, dwStringID, g_App.m_pPrefs->GetLanguageID()))
		strResString.LoadString(hMod, dwStringID, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
	return strResString;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}

CString __stdcall GetResString(UINT dwStringID, WORD uLanguageID)
{
#ifndef NEW_SOCKETS_ENGINE
	HINSTANCE	hMod = ::GetModuleHandle(NULL);
	CString		strResString;

	if (!strResString.LoadString(hMod, dwStringID, uLanguageID))
		strResString.LoadString(hMod, dwStringID, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
	return strResString;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetResString() loads default language resource string into already existing string.
__declspec(noinline) void __stdcall GetResString(CString *pstrOutput, UINT dwStringID)
{
#ifndef NEW_SOCKETS_ENGINE
	HINSTANCE hMod = ::GetModuleHandle(NULL);

	if (!pstrOutput->LoadString(hMod, dwStringID, g_App.m_pPrefs->GetLanguageID()))
		pstrOutput->LoadString(hMod, dwStringID, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT));
#endif //NEW_SOCKETS_ENGINE
}

bool CheckIsRegistrySet()
{
	EMULE_TRY

	CRegKey	regkey;

	regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\shell\\open\\command"));

	TCHAR	acBuffer[500];
	ULONG	dwSize = ARRSIZE(acBuffer);
	LONG	iRet;

	if ((iRet = regkey.QueryStringValue(NULL, acBuffer, &dwSize)) != ERROR_SUCCESS)
		return false;

	TCHAR	acMod[MAX_PATH];

	if (::GetModuleFileName(NULL, acMod, MAX_PATH) == 0)
		return false;

	CString	strReg = _T("\"");

	strReg += acMod;
	strReg += _T("\" \"%1\"");

	return (strReg == acBuffer);

	EMULE_CATCH

	return false;
}

static void BackupReg(void)
{
	EMULE_TRY
	// Look for pre-existing old ed2k links
	CRegKey regkey;
	regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\shell\\open\\command"));

	TCHAR rbuffer[500];
	ULONG maxsize = ARRSIZE(rbuffer);
	// Is it ok to write new values
	if ((regkey.QueryStringValue(_T("OldDefault"), rbuffer, &maxsize) != ERROR_SUCCESS) || (maxsize == 0))
	{
		maxsize = ARRSIZE(rbuffer);
		regkey.QueryStringValue(0, rbuffer, &maxsize);
		regkey.SetStringValue(_T("OldDefault"), rbuffer);
		regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\DefaultIcon") );
		maxsize = ARRSIZE(rbuffer);
		if (regkey.QueryStringValue(0, rbuffer, &maxsize) == ERROR_SUCCESS)
			regkey.SetStringValue(_T("OldIcon"), rbuffer);
	}
	regkey.Close();
	EMULE_CATCH
}

bool Ask4RegFix(bool bDontAsk)
{
	EMULE_TRY

	BackupReg();
#ifndef NEW_SOCKETS_ENGINE

//	Check registry if ed2k links is assigned to emule
	if (!CheckIsRegistrySet())
	{
		if (bDontAsk || (AfxMessageBox(GetResString(IDS_ASSIGNED2K), MB_ICONQUESTION | MB_YESNO) == IDYES))
		{
			CRegKey	regkey;
			TCHAR	acMod[MAX_PATH];

			if (::GetModuleFileName(NULL, acMod, MAX_PATH) == 0)
				return false;

			CString	strReg = _T("\"");

			strReg += acMod;
			strReg += _T("\" \"%1\"");

			regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\shell\\open\\command"));
			regkey.SetStringValue(0, strReg);
			regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\DefaultIcon"));
			regkey.SetStringValue(0, strReg);
			regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k") );
			regkey.SetStringValue(0, _T("URL: ed2k Protocol"));
			regkey.SetStringValue(_T("URL Protocol"), _T(""));
			regkey.Close();
		}
	}
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH

	return false;
}

void RevertReg(void)
{
	EMULE_TRY
	// restore previous ed2k links before being assigned to emule
	CRegKey regkey;
	regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\shell\\open\\command"));
	TCHAR rbuffer[500];
	ULONG maxsize = ARRSIZE(rbuffer);

	if (regkey.QueryStringValue(_T("OldDefault"), rbuffer, &maxsize) == ERROR_SUCCESS)
	{
		regkey.SetStringValue(0, rbuffer);
		regkey.Create(HKEY_CLASSES_ROOT, _T("ed2k\\DefaultIcon") );
		regkey.DeleteValue(_T("OldDefault"));
		maxsize = ARRSIZE(rbuffer);
		if (regkey.QueryStringValue(_T("OldIcon"), rbuffer, &maxsize) == ERROR_SUCCESS)
		{
			regkey.SetStringValue(0, rbuffer);
			regkey.DeleteValue(_T("OldIcon"));
		}
	}
	regkey.Close();
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetMaxConnections() determines a reasonable maximum number of connections based on the OS version.
int GetMaxConnections()
{
	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)// Windows 95 product family
	{
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)//old school 95
		{
			HKEY hKey;
			DWORD dwValue;
			DWORD dwLength = sizeof(dwValue);
			LONG lResult;

		//	Read the OS' max connection setting from the Registry
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\VxD\\MSTCP"),
				0, KEY_QUERY_VALUE, &hKey);
			lResult = RegQueryValueEx(hKey, TEXT("MaxConnections"), NULL, NULL,
				(LPBYTE)&dwValue, &dwLength);
			RegCloseKey(hKey);

			if(lResult != ERROR_SUCCESS || lResult < 1)
				return 100;  //the default for 95 is 100

			return dwValue;
		}
		else
		{ //98 or ME
			HKEY hKey;
			TCHAR szValue[32];
			DWORD dwLength = sizeof(szValue);
			LONG lResult;

		//	Read the OS' max connection setting from the Registry
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\VxD\\MSTCP"),
				0, KEY_QUERY_VALUE, &hKey);
			lResult = RegQueryValueEx(hKey, TEXT("MaxConnections"), NULL, NULL,
				(LPBYTE)szValue, &dwLength);
			RegCloseKey(hKey);

			LONG lMaxConnections;

			if(lResult != ERROR_SUCCESS || (lMaxConnections = _tstoi(szValue)) < 1)
				return 100;  //the default for 98/ME is 100

			return lMaxConnections;
		}
	}
	return -1;  //no limits (Windows NT product family)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WORD DetectWinVersion(bool *pbNTBased)
{
	*pbNTBased = false;

	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	switch(osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
			*pbNTBased = true;

			if (osvi.dwMajorVersion <= 4)
				return _WINVER_NT4_;
			if (osvi.dwMajorVersion == 5)
			{
				if (osvi.dwMinorVersion == 0)
					return _WINVER_2K_;
				if (osvi.dwMinorVersion == 1)
					return _WINVER_XP_;
				if (osvi.dwMinorVersion == 2)	//	Windows Server 2003
					return _WINVER_SE_;
			}
			else if (osvi.dwMajorVersion == 6)
			{
#if 0	// following versions of this family will be probably closer to Vista than to XP
				if (osvi.dwMinorVersion == 0)
#endif
					return _WINVER_VISTA_;
			}
			return _WINVER_XP_;	//	Never return Win95 if we get the info about a NT system

		case VER_PLATFORM_WIN32_WINDOWS:
			if (osvi.dwMajorVersion == 4)
			{
				if (osvi.dwMinorVersion == 0)
					return _WINVER_95_;
				if (osvi.dwMinorVersion == 10)
					return _WINVER_98_;
				if (osvi.dwMinorVersion == 90)
					return _WINVER_ME_;
			}

		default:
			break;
	}
	return _WINVER_95_;		// there shouldn't be anything lower than this
}

uint64 GetFreeDiskSpaceX(LPCTSTR pDirectory)
{
	static BOOL	_bInitialized = FALSE;
	static BOOL	(WINAPI *_pGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER) = NULL;

	if (!_bInitialized)
	{
		_bInitialized = TRUE;
#ifdef _UNICODE
		(FARPROC&)_pGetDiskFreeSpaceEx = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetDiskFreeSpaceExW");
#else
		(FARPROC&)_pGetDiskFreeSpaceEx = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetDiskFreeSpaceExA");
#endif
	}

	if (_pGetDiskFreeSpaceEx != NULL)
	{
		ULARGE_INTEGER	qwFreeDiskSpace, qwDummy;

		_pGetDiskFreeSpaceEx(pDirectory, &qwFreeDiskSpace, &qwDummy, NULL);
		return qwFreeDiskSpace.QuadPart;
	}
	else
	{
		TCHAR	acDrive[16];
		const TCHAR	*p = _tcschr(pDirectory, _T('\\'));

		if (p != NULL)
		{
			memcpy2(acDrive, pDirectory, (p - pDirectory) * sizeof(TCHAR));
			acDrive[p - pDirectory] = _T('\0');
		}
		else
			_tcscpy(acDrive, pDirectory);

		DWORD	dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwDummy;

		if (GetDiskFreeSpace(acDrive, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwDummy))
			return (static_cast<uint64>(dwFreeClusters) * dwSectPerClust * dwBytesPerSect);
		else
			return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FillImgLstWith16x16Icons() fills image list with 16x16 icons.
//		Params:
//			pImgLst     - image list to fill;
//			uResIDs     - array of icon resource IDs;
//			uiResAmount - number of icons in array.
void __stdcall FillImgLstWith16x16Icons(CImageList *pImgLst, const uint16 *uResIDs, unsigned uiResAmount)
{
	HINSTANCE hInst = AfxGetInstanceHandle();

	for (unsigned ui = 0; ui < uiResAmount; ui++)
	{
		HICON	hIcon = reinterpret_cast<HICON>(::LoadImage(hInst, MAKEINTRESOURCE(uResIDs[ui]), IMAGE_ICON, 16, 16, 0));

		pImgLst->Add(hIcon);
	//	This resource isn't required anymore, unload it
		::DestroyIcon(hIcon);
	}
}

CString GetRatingString(EnumPartFileRating nRating)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY
	switch (nRating)
	{
	case PF_RATING_NONE:
		return GetResString(IDS_CMT_NOTRATED);
		break;
	case PF_RATING_FAKE:
		return GetResString(IDS_CMT_FAKE);
		break;
	case PF_RATING_POOR:
		return GetResString(IDS_CMT_POOR);
		break;
	case PF_RATING_GOOD:
		return GetResString(IDS_CMT_GOOD);
		break;
	case PF_RATING_FAIR:
		return GetResString(IDS_CMT_FAIR);
		break;
	case PF_RATING_EXCELLENT:
		return GetResString(IDS_CMT_EXCELLENT);
		break;
	}
	EMULE_CATCH
	return GetResString(IDS_CMT_NOTRATED);
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}

#if 0	// Can be required later for AICH
// Returns a BASE32 encoded byte array
//
// [In]
//   buffer: Pointer to byte array
//   bufLen: Lenght of buffer array
//
// [Return]
//   CString object with BASE32 encoded byte array
CString EncodeBase32(const unsigned char* buffer, unsigned int bufLen)
{
	CString	strBase32Buff;

	EMULE_TRY

	unsigned int i, index;
	unsigned char word;
//	int pos = 0;

	for(i = 0, index = 0; i < bufLen;)
	{

		// Is the current word going to span a byte boundary?
		if (index > 3)
		{
			word = (buffer[i] & (0xFF >> index));
			index = (index + 5) % 8;
			word <<= index;
			if (i < bufLen - 1)
				word |= buffer[i + 1] >> (8 - index);

			i++;
		}
		else
		{
			word = (buffer[i] >> (8 - (index + 5))) & 0x1F;
			index = (index + 5) % 8;
			if (index == 0)
				i++;
		}

		strBase32Buff += (char) base32Chars[word];
	}

	EMULE_CATCH

	return strBase32Buff;
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int UpdateURLMenu(CMenu &menu)
{
	int iNumURLs = 0;

	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	g_App.m_strWebServiceURLArray.RemoveAll();

	TCHAR	acBuf[1024];
	FILE	*pWebServiceFile = _tfsopen(g_App.m_pPrefs->GetConfigDir() + _T("webservices.dat"), _T("r"), _SH_DENYWR);

	if (pWebServiceFile != NULL)
	{
		CString	strBuffer;

	//	Until we've read the last line...
		while (feof(pWebServiceFile) == 0)
		{
		//	Try to get a line from the web services file. If we fail...
			if (_fgetts(acBuf, ARRSIZE(acBuf), pWebServiceFile) == 0)
				break;
			if (*acBuf == _T('#') || *acBuf == _T('/'))	// Ignore comments
				continue;
			strBuffer = acBuf;
			if (strBuffer.GetLength() < 5)	// Ignore too short lines
				continue;

			if (strBuffer == _T("<separator>\n") || strBuffer == _T("<separator>"))
				menu.AppendMenu(MF_SEPARATOR);
			else
			{
				int	iPos = strBuffer.Find(_T(','));

				if (iPos > 0 && iNumURLs < 64)
				{
					menu.AppendMenu(MF_STRING, MP_WEBURL + iNumURLs++, strBuffer.Left(iPos).Trim());
					g_App.m_strWebServiceURLArray.Add(strBuffer.Right(strBuffer.GetLength() - iPos - 1).Trim());
				}
			}
		}
		fclose(pWebServiceFile);
	}
#endif //NEW_SOCKETS_ENGINE
	EMULE_CATCH

	return iNumURLs;
}

static CString ReplaceDotsWithSpaces(CString strIn, CString exceptStr, CString addClear, bool keepExt)
{
	//with standard-params simply all dots are replaced with spaces :)
	//dots won't be replaced between 2 chars of exceptStr (numbers for versions and dates)
	//if u want to replace additional chars with spaces set addClear (invalid & nonsense chars)
	//set keepExt=true for keeping the extension-dot
	EMULE_TRY
	int extpos = strIn.ReverseFind(_T('.'));
	int max = strIn.GetLength() - 1;
	for (int pos = 0; pos <= max; pos++)
	{
		if (strIn.GetAt(pos) == _T('.'))
		{
			//replace dot at beginning and end
			if (pos == 0 || pos == max)
				strIn.SetAt(pos, _T(' '));
			//replace dot only if not between 2 chars of exceptStr
			else if (exceptStr.Find(strIn.GetAt(pos-1)) < 0 || exceptStr.Find(strIn.GetAt(pos+1)) < 0)
				strIn.SetAt(pos, _T(' '));
		}
		//replace chars from addClear with space, too
		else if (addClear.Find(strIn.GetAt(pos)) >= 0)
			strIn.SetAt(pos, _T(' '));
	}
	if (keepExt && extpos >= 0)
		strIn.SetAt(extpos, _T('.'));
	EMULE_CATCH
	return strIn;
}

static CString DeleteNonAlphaNumeric(CString inStr, CString exceptStr, bool onlyRep)
{
	//with standard-params output simply won't contain any nonalphanumeric chars
	//set exceptStr to chars u don't want to be deleted (i.e. pipe as separator-char "|")
	//if onlyRep==true only repeated non-alphanumeric chars will be reduced to one occurence
	//if onlyRep==false all non-alphanumeric chars are deleted except the allowed ones in exceptStr
	EMULE_TRY
	TCHAR curChar;
	int pos = 0;
	while (pos < inStr.GetLength())
	{
		curChar = inStr.GetAt(pos);
		if (!IsCharAlphaNumeric(curChar) && exceptStr.Find(curChar) < 0
		&& (!onlyRep || (pos+1 < inStr.GetLength() && inStr.Find(curChar, pos+1) == pos+1 )))
			inStr.Delete(pos);
		else
			pos++;
	}
	EMULE_CATCH
	return inStr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ExtractTitle() extracts title from the file name
static CString ExtractTitle(const CString &strIn)
{
	static const TCHAR s_acDelim[] = {
		_T('['), _T(']'), _T('('), _T(')'), _T('{'), _T('}')
	};
	static const TCHAR *s_apcTailAd[] = {
		_T("dvd"), _T("divx"), _T("xvid"), _T("ac3"), _T("mp3"), _T("vcd"), _T("svcd"),
		_T("dvb"), _T("vh-prod"), _T("vhs")
	};
	static const TCHAR *s_apcTailWords[] = {
		_T("proper"), _T("internal"), _T("int"), _T("uncut")
	};
	CString		strName(URLDecode(strIn).MakeLower());
	CString		strToken;
	int			iTkPos, iPos;
	TCHAR		cCh;
	unsigned	ui;

	iPos = strName.ReverseFind(_T('.'));
	if (iPos >= 0)
		strName.Truncate(iPos);	//	cut off extension

//	CD# is usually after the title, cut off by it
	for (iTkPos = 0;;)
	{
		if ((iPos = strName.Find(_T("cd"), iTkPos)) <= 0)
			break;
		cCh = strName.GetString()[iPos + CSTRLEN(_T("cd"))];
		if (static_cast<unsigned>(cCh - _T('0')) <= 9u)
		{
			strName.Truncate(iPos);
			break;
		}
		iTkPos = iPos + 1;
	}

//	Format specifiers are usually after the title, cut off by them
	for (ui = 0; ui < ARRSIZE(s_apcTailAd); ui++)
	{
		if ((iPos = strName.Find(s_apcTailAd[ui])) <= 0)
			continue;
		strName.Truncate(iPos);
	}

	CString	strList(g_App.m_pPrefs->GetFilenameCleanups().MakeLower());	//list of pipe-separated tokens

	for (iTkPos = 0;;)
	{
		strToken = strList.Tokenize(_T("|"), iTkPos);
		if (strToken.IsEmpty())
			break;
	//	As ads is normally at the end, cut off by it
		if ((iPos = strName.Find(strToken)) > 0)
			strName.Truncate(iPos);
	}

//	Take out everything in brackets
	for (ui = 0; ui < ARRSIZE(s_acDelim); ui += 2)
	{
		for (;;)
		{
			iPos = strName.Find(s_acDelim[ui]);
			if (iPos < 0)
				break;
			iTkPos = strName.Find(s_acDelim[ui + 1], iPos);
			if (iTkPos > 0)
			//	Closing bracket is left for ReplaceDotsWithSpaces to be replaced with a space
				strName.Delete(iPos, iTkPos - iPos);
			else
				strName.Truncate(iPos);
		}
	}

	strName = ReplaceDotsWithSpaces(strName, _T(""), _T("_+\\\"/:*?<>|-&@#=,])}"), false);
	strName = DeleteNonAlphaNumeric(strName, _T(""), true);	// reduce doubles

	strName.Trim();

//	Remove specific words from the tail
	iPos = strName.ReverseFind(_T(' '));
	if (iPos >= 0)
	{
		for (ui = 0; ui < ARRSIZE(s_apcTailWords); ui++)
		{
			if (_tcscmp(&strName.GetString()[iPos + 1], s_apcTailWords[ui]) == 0)
			{
				strName.Truncate(iPos);
				break;
			}
		}
	}

//	Remove year at the end
	iPos = strName.ReverseFind(_T(' '));
	if (iPos >= 0)
	{
		cCh = strName.GetString()[iPos + 1];
		if (static_cast<unsigned>(cCh - _T('0')) <= 9u)
		{
			ui = _tstoi(&strName.GetString()[iPos + 1]);
			if ((ui - 1900u) < (2099u - 1900u))	// check centures
				strName.Truncate(iPos);
		}
	}

	return strName;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RunURL(CAbstractFile* pFile, CString srtURLPattern)
{
	EMULE_TRY
	if (pFile != NULL)
	{
	//	Convert hash to hexadecimal text and add it to the URL
		srtURLPattern.Replace(_T("#hashid"), HashToString(pFile->GetFileHash()));

	//	Add file size to the URL
		CString strTemp;
		strTemp.Format(_T("%I64u"), pFile->GetFileSize());
		srtURLPattern.Replace(_T("#filesize"), strTemp);

	//	Add filename to the URL
		srtURLPattern.Replace(_T("#filename"), URLEncode(pFile->GetFileName()));

	//	Add cleaned up name to the URL
		srtURLPattern.Replace(_T("#cleanname"), URLEncode(ExtractTitle(pFile->GetFileName())));
	}

//	Replace "|" for Mozilla compatibility
	srtURLPattern.Replace(_T("|"), _T("%7C"));

//	Open URL
#ifndef NEW_SOCKETS_ENGINE
	ShellExecute(NULL, NULL, srtURLPattern, NULL, g_App.m_pPrefs->GetAppDir(), SW_SHOWDEFAULT);
#endif //NEW_SOCKETS_ENGINE
	EMULE_CATCH
}

CString GetClientVersionString(EnumClientTypes eClienType, uint32 dwVersion)
{
#ifndef NEW_SOCKETS_ENGINE
	if (dwVersion == 0)
		return _T("v ?");

	CString	strBuff;
	uint32	dwMajVer = GET_CLIENT_MAJVER(dwVersion);
	uint32	dwMinVer = GET_CLIENT_MINVER(dwVersion);
	uint32	dwUpdVer = GET_CLIENT_UDPVER(dwVersion);

	switch(eClienType)
	{
		case SO_PLUS:
			strBuff.Format(_T("v%u"), dwMajVer);
			if (dwMinVer != 0)
				strBuff.AppendFormat(_T(".%u"), dwMinVer);
			if (dwUpdVer != 0)
				strBuff += static_cast<TCHAR>(_T('a') + dwUpdVer - 1);
			break;
		case SO_EMULE:
			if ((dwVersion >= FORM_CLIENT_VER(0, 40, 0)) && (dwUpdVer < 26))
			{
				strBuff.Format(_T("v%u.%u%c"), dwMajVer, dwMinVer, _T('a') + dwUpdVer);
				break;
			}
		default:
			if ((dwUpdVer != 0) || ((eClienType == SO_AMULE) && (dwVersion >= FORM_CLIENT_VER(0, 40, 0))))
				strBuff.Format(_T("v%u.%u.%u"), dwMajVer, dwMinVer, dwUpdVer);
			else
				strBuff.Format(_T("v%u.%u"), dwMajVer, dwMinVer);
			break;
	}
	return strBuff;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}

CString StringLimit(const CString& in, unsigned uiLen)
{
	if ((static_cast<unsigned>(in.GetLength()) <= uiLen) || (uiLen <= (1u + 3u + 8u)))
		return in;
	return (in.Left(uiLen - 8) + _T("...") + in.Right(8));
}

CString ConcatFullPath(const CString &strPath, const CString &strFname)
{
	if (strPath.Right(1) == _T('\\'))
		return (strPath + strFname);
	else
		return (strPath + _T('\\') + strFname);
}

void GetStatusULQueueString(CString *pstrOut, EnumULQState State)
{
#ifndef NEW_SOCKETS_ENGINE
	static const uint16 s_auResTbl[] =
	{
		IDS_TRANSFERRING,		//US_UPLOADING
		IDS_ONQUEUE,			//US_ONUPLOADQUEUE
		IDS_CONNECTING,			//US_CONNECTING
		IDS_BANNED,				//US_BANNED
		IDS_UNKNOWN				//US_NONE
	};
	unsigned	uiState = State;

	if (uiState >= ARRSIZE(s_auResTbl))
		uiState = US_NONE;
	GetResString(pstrOut, s_auResTbl[uiState]);
#else
	*pstrOut = _T("");
#endif //NEW_SOCKETS_ENGINE
}

CString GetClientNameString(EnumClientTypes eClientType)
{
	static const TCHAR s_acNames[][10] =
	{
		_T("Plus"),		//SO_PLUS
		_T("eMule"),	//SO_EMULE
		_T("aMule"),	//SO_AMULE
		_T("Hybrid"),	//SO_EDONKEYHYBRID
		_T("eDonkey"),	//SO_EDONKEY
		_T("MLdonkey"),	//SO_MLDONKEY
		_T(""),			//SO_OLDEMULE
		_T("Shareaza"),	//SO_SHAREAZA
		_T("xMule"),	//SO_XMULE
		_T("lphant")	//SO_LPHANT
	};
	CString	strName;
	uint32	dwClientType = static_cast<uint32>(eClientType);

	if (dwClientType < SO_UNKNOWN)
		strName = reinterpret_cast<const TCHAR*>(&s_acNames[dwClientType]);
#ifndef NEW_SOCKETS_ENGINE
	else
		GetResString(&strName, IDS_UNKNOWN);
#endif //NEW_SOCKETS_ENGINE

	return strName;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	NOPRM(lParam);

	if ((uMsg == BFFM_INITIALIZED) && lpData)
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);

	return 0;
}

CString BrowseFolder(HWND hwndOwner, LPCTSTR pszTitle, LPCTSTR pszStartFolder)
{
	CString retstring(pszStartFolder);
	EMULE_TRY
	TCHAR pszDisplayName[MAX_PATH];
	LPITEMIDLIST lpID;
	BROWSEINFO bi;

	bi.hwndOwner = hwndOwner;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = pszDisplayName;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)pszStartFolder;
	bi.iImage = NULL;
	lpID = SHBrowseForFolder(&bi);
	if (lpID != NULL)
	{
		if (SHGetPathFromIDList(lpID, pszDisplayName))
			retstring.Format(_T("%s"), pszDisplayName);
		LPMALLOC pMalloc = NULL;
		HRESULT hResult = SHGetMalloc (&pMalloc);
		if (hResult == NOERROR && pMalloc)
			pMalloc->Free (lpID);
		pMalloc->Release();
	}
	EMULE_CATCH
	return retstring;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	HexChr2Num() converts hexadecimal character to a number.
//		Params: cCh - char ('0'-'9','A'-'F','a'-'f')
//		Return: 0..15 - number, else error
unsigned HexChr2Num(TCHAR cCh)
{
	unsigned	uiDigit;

	if ((uiDigit = static_cast<unsigned>(cCh - _T('0'))) > 9u)
		uiDigit = static_cast<unsigned>(static_cast<TCHAR>(CHR2UP(cCh)) - _T('A')) + 10u;
	return uiDigit;
}

static CString LeadingCaps(CString inStr, CString exceptStr = _T(""))
{
	//there is no upper after chars defined in exceptStr (recommended: "'")
	EMULE_TRY
	if (!inStr.IsEmpty())
	{
		CString tempStr(inStr.GetAt(0));
		tempStr.MakeUpper();
		inStr.SetAt(0, tempStr.GetAt(0));

		int max = inStr.GetLength()-1;
		for (int pos=0; pos < max; pos++)
		{
			if (!IsCharAlpha(inStr.GetAt(pos)) && exceptStr.Find(inStr.GetAt(pos)) < 0)
			{
				tempStr=inStr.GetAt(pos+1);
				tempStr.MakeUpper();
				inStr.SetAt(pos+1,tempStr.GetAt(0));
			}
		}
	}
	EMULE_CATCH
	return inStr;
}

//	TODO: ms-help://MS.VSCC/MS.MSDNVS.1031/vclib/html/_crt_Interpretation_of_Multibyte.2d.Character_Sequences.htm
CString CleanupFilename(const CString &inStr)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY
	CString filename(inStr);	//process on local copy only
	CString resToken;			//one token out of strList
	CString extension;			//only the extension of filename with dot
	int foundPos = 0;			//position where something is found
	int curPos = 0;				//position for tokenize() (by reference!)

	filename.SetString(URLDecode(filename.MakeLower()));

//	Cut off & cleanup extension before further process
	foundPos = filename.ReverseFind(_T('.'));
	if ((foundPos >= 0) && (foundPos < (filename.GetLength() - 1)))	//minimum one char
	{
		extension.SetString(filename.Right(filename.GetLength()-foundPos));		//copy extension with dot
		extension.SetString(DeleteNonAlphaNumeric(extension, _T("."), false));	//only alphanum & dot allowed
		filename.SetString(filename.Left(foundPos));							//cut to filename without dot
		for (;;)
		{	//cutoff doubled extensions in filename
			filename.TrimRight(_T(" .,"));
			curPos = filename.GetLength()-extension.GetLength();
			if (curPos < 0)
				break;
			foundPos = filename.Find(extension,curPos);
			if (foundPos == curPos)
				filename.SetString(filename.Left(curPos));
			else
				break;
		}
	}
	if (extension.IsEmpty()) extension.SetString(_T(".non"));

	//remove all between [ and ]
	if (g_App.m_pPrefs->GetFilenameCleanupTags())
	{
		int foundPo2, foundPo3;
		for (;;)
		{
			foundPos = filename.Find(_T('['));
			if (foundPos < 0)
				break;
			foundPo2 = filename.Find(_T(']'), foundPos);
			if (foundPo2 > foundPos)
			{
				foundPo3 = filename.Find(_T('['), foundPos + 1);
				while (foundPo3 > 0 && foundPo3 < foundPo2)
				{	//check for nested brackets
					foundPos = foundPo3;
					foundPo3 = filename.Find(_T('['), foundPos + 1);
				}
				filename.SetString(filename.Left(foundPos) + _T(" ") + filename.Right(filename.GetLength()-foundPo2-1));
			}
			else
				break;
		}
	}

	CString	strList(g_App.m_pPrefs->GetFilenameCleanups());	//list of pipe-separated tokens

	strList.MakeLower();
	for (curPos = 0;;)
	{
		resToken = strList.Tokenize(_T("|"), curPos);
		if (resToken.IsEmpty())
			break;
		filename.Replace(resToken, _T(" "));
	}

	filename.SetString(ReplaceDotsWithSpaces(filename, _T("01234567689"), _T("_+\\\"/:*?<>|"), false)); //spaceholders

	//advertising cleanup (lowercase without dots)
	filename.Replace(_T("shared by"), _T(""));
	filename.Replace(_T("shared for"), _T(""));
	filename.Replace(_T("shared via"), _T(""));
	filename.Replace(_T("shared "), _T(""));
	filename.Replace(_T("found at"), _T(""));
	filename.Replace(_T("found via xxx"), _T(""));	//filedonkey.com special
	filename.Replace(_T("found via"), _T(""));
	filename.Replace(_T("link by"), _T(""));
	filename.Replace(_T("linked at"), _T(""));
	filename.Replace(_T("linked by"), _T(""));
	filename.Replace(_T("linked "), _T(""));
	filename.Replace(_T("powered by"), _T(""));
	filename.Replace(_T("-powered-by-"), _T(""));	//eselfilme.de special
	filename.Replace(_T("powered-by-"), _T(""));	//eselfilme.de special
	filename.Replace(_T("powered-for-"), _T(""));	//eselfilme.de special
	filename.Replace(_T("powered "), _T(""));
	filename.Replace(_T("sponsored for"), _T(""));
	filename.Replace(_T("sponsored by"), _T(""));
	filename.Replace(_T("sponsored "), _T(""));

	filename.SetString(DeleteNonAlphaNumeric(filename, _T(""), true));	//reduce doubles

	//remove spaces inside brackets
	filename.Replace(_T("( "), _T("("));
	filename.Replace(_T(" )"), _T(")"));
	filename.Replace(_T("[ "), _T("["));
	filename.Replace(_T(" ]"), _T("]"));
	filename.Replace(_T("{ "), _T("{"));
	filename.Replace(_T(" }"), _T("}"));
	//remove empty brackets
	filename.Replace(_T("()"), _T(""));
	filename.Replace(_T("[]"), _T(""));
	filename.Replace(_T("{}"), _T(""));

	filename.SetString(LeadingCaps(filename, _T("'´`0123456789")));	//leadingcaps except after "'´`" and numbers

	//special uppercasing by netwolf and DoubleT
	filename.Replace(_T("Cd"),_T("CD"));
	filename.Replace(_T("Vcd"),_T("VCD"));
	filename.Replace(_T("Svcd"),_T("SVCD"));
	filename.Replace(_T("Mvcd"),_T("MVCD"));
	filename.Replace(_T("Rvcd"),_T("RVCD"));
	filename.Replace(_T("Rsvcd"),_T("RSVCD"));
	filename.Replace(_T("Dvd"),_T("DVD"));
	filename.Replace(_T("Divx"),_T("DivX"));
	filename.Replace(_T("Xvid"),_T("XviD"));
	filename.Replace(_T("Ac3"),_T("AC3"));
	filename.Replace(_T("Aac"),_T("AAC"));
	filename.Replace(_T("Mp3"),_T("MP3"));
	filename.Replace(_T("Vbr"),_T("VBR"));
	filename.Replace(_T("Cbr"),_T("CBR"));
	filename.Replace(_T("Vhs"),_T("VHS"));
	filename.Replace(_T("Tv"),_T("TV"));
	filename.Replace(_T("Fsk"),_T("FSK"));
	filename.Replace(_T("Dvb"),_T("DVB"));
	filename.Replace(_T("Emuleplus"), CLIENT_NAME);
	filename.Replace(_T("Emule"),_T("eMule"));
	//make "v" in front of versions lowercase
	filename.Replace(_T("V1"),_T("v1"));
	filename.Replace(_T("V2"),_T("v2"));
	filename.Replace(_T("V3"),_T("v3"));
	filename.Replace(_T("V4"),_T("v4"));
	filename.Replace(_T("V5"),_T("v5"));
	filename.Replace(_T("V6"),_T("v6"));
	filename.Replace(_T("V7"),_T("v7"));
	filename.Replace(_T("V8"),_T("v8"));
	filename.Replace(_T("V9"),_T("v9"));
	filename.Replace(_T("V0"),_T("v0"));
	//roman numerals up to 20
	filename.Replace(_T("Iii"),_T("III"));
	filename.Replace(_T("Viii"),_T("VIII"));
	filename.Replace(_T("Vii"),_T("VII"));
	filename.Replace(_T("Xiii"),_T("XIII"));
	filename.Replace(_T("Xii"),_T("XII"));
	filename.Replace(_T("Xiv"),_T("XIV"));
	filename.Replace(_T("Xviii"),_T("XVIII"));
	filename.Replace(_T("Xvii"),_T("XVII"));
	filename.Replace(_T("Xvi"),_T("XVI"));
	filename.Replace(_T("Xix"),_T("XIX"));
	filename.Replace(_T("Xxx"),_T("XXX"));
	filename += _T(' ');
	filename.Replace(_T("Ii "),_T("II "));
	filename.Replace(_T("Iv "),_T("IV "));
	filename.Replace(_T("Vi "),_T("VI "));
	filename.Replace(_T("Ix "),_T("IX "));
	filename.Replace(_T("Xi "),_T("XI "));
	filename.Replace(_T("Xx "),_T("XX "));

	//final steps
	filename.TrimRight(_T(" -.,#&([{"));
	filename.TrimLeft(_T(" -.,#&)]}"));
	if (filename.IsEmpty())
		return inStr;
	filename.Append(extension);
	return filename;
	EMULE_CATCH
	return inStr;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}

static int GetSystemErrorString(DWORD dwError, CString &rstrError)
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
	LPTSTR		pszSysMsg = NULL;
	DWORD		dwLength = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
								(LPTSTR)&pszSysMsg, 0, NULL );

	if (pszSysMsg != NULL && dwLength != 0)
	{
		if (dwLength >= 2 && pszSysMsg[dwLength - 2] == _T('\r'))
			pszSysMsg[dwLength - 2] = _T('\0');
		rstrError = pszSysMsg;
		rstrError.Replace(_T("\r\n"), _T(" ")); // some messages contain CRLF within the message!?
	}
	else
	{
		rstrError.Empty();
	}

	if (pszSysMsg)
		LocalFree(pszSysMsg);

	return rstrError.GetLength();
}

int GetErrorMessage(DWORD dwError, CString &rstrErrorMsg, DWORD dwFlags /* = 0*/)
{
	int			iMsgLen = GetSystemErrorString(dwError, rstrErrorMsg);

	if (iMsgLen == 0)
	{
		if ((long)dwError >= 0)
			rstrErrorMsg.Format(_T("Error %u"), dwError);
		else
			rstrErrorMsg.Format(_T("Error 0x%08x"), dwError);
	}
	else if (dwFlags & 1)
	{
		CString			strFullErrorMsg;

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
	CString			strError;

	GetErrorMessage(dwError, strError, dwFlags);
	return strError;
}

CString GetErrorMessage(CException *pError)
{
	ASSERT(pError != NULL);
	EMULE_TRY

	const size_t	bs = 255;
	TCHAR	acBuf[bs];

	pError->GetErrorMessage(acBuf, bs);
	return acBuf;

	EMULE_CATCH

	return _T("N/A");
}

bool DialogBrowseFile(CString &rstrPath, LPCTSTR pcFilters, LPCTSTR pcDefFileName/*=NULL*/,
	DWORD dwFlags/*=0*/, bool bOpenFile/*=true*/, LPCTSTR pcDefDir/*=NULL*/)
{
	bool		bRc;
	CFileDialog	myFileDialog( bOpenFile, NULL, pcDefFileName,
		dwFlags | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, pcFilters, NULL,
		0 );	// Automatically use explorer style open dialog on systems which support it

	myFileDialog.m_pOFN->lpstrInitialDir = pcDefDir;
	if ((bRc = (myFileDialog.DoModal() == IDOK)) == true)
		rstrPath = myFileDialog.GetPathName();
	return bRc;
}

#ifdef _UNICODE
unsigned long inet_addr(LPCTSTR cp)
{
	USES_CONVERSION;
	return inet_addr(T2A(cp));
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Same as 'inet_ntoa(*(in_addr*)&nIP)' but is not restricted to ASCII strings
CString ipstr(uint32 dwIP)
{
	const byte	*pbyteIP = reinterpret_cast<byte*>(&dwIP);
	CString		strIP;

	strIP.ReleaseBuffer( _stprintf( strIP.GetBuffer(3 + 1 + 3 + 1 + 3 + 1 + 3),
		_T("%u.%u.%u.%u"), pbyteIP[0], pbyteIP[1], pbyteIP[2], pbyteIP[3] ) );
	return strIP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ipstr(CString *pstrOut, uint32 dwIP)
{
	const byte	*pbyteIP = reinterpret_cast<byte*>(&dwIP);

	pstrOut->ReleaseBuffer( _stprintf( pstrOut->GetBuffer(3 + 1 + 3 + 1 + 3 + 1 + 3),
		_T("%u.%u.%u.%u"), pbyteIP[0], pbyteIP[1], pbyteIP[2], pbyteIP[3] ) );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	The validity check of HybridID based on invalid IP ranges
bool IsGoodHybridID(uint32 dwHybridID)
{
//	0.0.0.0					invalid
	if (dwHybridID == 0)
		return false;
//	127.0.0.0 - 127.255.255.255		Loopback
	if (((dwHybridID & 0xFF000000) == 0x7F000000) && !g_App.m_pPrefs->AllowLocalHostIP())
		return false;
//	224.0.0.0 - 239.255.255.255		Multicast
//	240.0.0.0 - 255.255.255.255		Reserved for Future Use
	if (dwHybridID >= 0xE0000000)
		return false;

	return true;
}

CString Crypt(const CString &strNormal)
{
	uint32		dwStrLen = strNormal.GetLength();
	const TCHAR	*pcSrc = strNormal.GetString();
	CString		str;
	TCHAR		*pcDst = str.GetBufferSetLength(2 * dwStrLen);

	for (uint32 dwIdx = 0; dwIdx < dwStrLen; dwIdx++, pcDst += 2)
	{
		pcDst[0] = ((pcSrc[dwIdx] >> 4) & 0x0F) + _T('A');
		pcDst[1] = (pcSrc[dwIdx] & 0x0F) + _T('A');
	}
	return str;
}

CString Decrypt(const CString &strCrypted)
{
	uint32		dwStrLen = strCrypted.GetLength() / 2;
	const TCHAR	*pcSrc = strCrypted.GetString();
	CString		str;
	TCHAR		*pcDst = str.GetBufferSetLength(dwStrLen);

	for (uint32 dwIdx = 0; dwIdx < dwStrLen; dwIdx++, pcSrc += 2)
	{
		pcDst[dwIdx] = (((pcSrc[0] - _T('A')) << 4) | (pcSrc[1] - _T('A'))) & 0xFF;
	}
	return str;
}

typedef struct
{
	LPCTSTR	pszInitialDir;
	LPCTSTR	pszDlgTitle;
} BROWSEINIT, *LPBROWSEINIT;

bool SelectDir(HWND hWnd, LPTSTR pszPath, LPCTSTR pszTitle, LPCTSTR pszDlgTitle)
{
	bool	bResult = false;

	CoInitialize(0);
	LPMALLOC pShlMalloc;
	if (SHGetMalloc(&pShlMalloc) == NOERROR)
	{
		BROWSEINFO BrsInfo = {0};
		BrsInfo.hwndOwner = hWnd;
		BrsInfo.lpszTitle = (pszTitle != NULL) ? pszTitle : pszDlgTitle;
		BrsInfo.ulFlags = BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_SHAREABLE | BIF_DONTGOBELOWDOMAIN;

		BROWSEINIT BrsInit = {0};
		if (pszPath != NULL || pszTitle != NULL || pszDlgTitle != NULL)
		{
			// Need the 'BrowseCallbackProc' to set those strings
			BrsInfo.lpfn = BrowseCallbackProc;
			BrsInfo.lParam = (LPARAM)&BrsInit;
			BrsInit.pszDlgTitle = (pszDlgTitle != NULL) ? pszDlgTitle : NULL/*pszTitle*/;
			BrsInit.pszInitialDir = pszPath;
		}

		LPITEMIDLIST pidlBrowse;
		if ((pidlBrowse = SHBrowseForFolder(&BrsInfo)) != NULL)
		{
			if (SHGetPathFromIDList(pidlBrowse, pszPath))
				bResult = true;
			pShlMalloc->Free(pidlBrowse);
		}
		pShlMalloc->Release();
	}
	CoUninitialize();
	return bResult;
}

void MakeFolderName(TCHAR *pcPath)
{
	CString strPath(pcPath);

	PathCanonicalize(pcPath, strPath);
	PathRemoveBackslash(pcPath);
}

TCHAR* stristr(const TCHAR *str1, const TCHAR *str2)
{
	const TCHAR *cp = str1;
	const TCHAR *s1;
	const TCHAR *s2;

	if (!*str2)
		return (TCHAR *)str1;

	while (*cp)
	{
		s1 = cp;
		s2 = str2;

		while (*s1 && *s2 && _totlower(*s1) == _totlower(*s2))
			s1++, s2++;

		if (!*s2)
			return (TCHAR *)cp;

		cp++;
	}

	return NULL;
}

size_t fast_strlen(const char *pcStr)
{
#define hasNullChar(x) ((x - 0x01010101) & ~x & 0x80808080)
#define SW (sizeof(int) / sizeof(char))
	const char *pcTmp;
	int iVal;

	pcTmp = pcStr - 1;
	do {
		pcTmp++;
		if ((((int)pcTmp) & (SW - 1)) == 0)
		{
			do {
				iVal = *((int*)pcTmp);
				pcTmp += SW;
			} while(!hasNullChar(iVal));
			pcTmp -= SW;
		}
	} while(*pcTmp != 0);
	return pcTmp - pcStr;
#undef hasNullChar
#undef SW
}

COLORREF LightenColor(COLORREF crColor, int i)
{
	return RGB( max(min(GetRValue(crColor) + i, 255), 0),
				max(min(GetGValue(crColor) + i, 255), 0),
				max(min(GetBValue(crColor) + i, 255), 0) );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// note: if value of qwTotal is larger as 1e+13 (10000 Gb), then calculation will be incorrect
double __fastcall GetPercent(uint64 qwValue, uint64 qwTotal)
{
	double		dblPercentage = 0.0;

	if (qwTotal > 0)
		dblPercentage = static_cast<double>(100.0 * qwValue) / static_cast<double>(qwTotal);

	return dblPercentage;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double __fastcall GetPercent(uint32 dwValue, uint32 dwTotal)
{
	double		dblPercentage = 0.0;

	if (dwTotal > 0)
		dblPercentage = static_cast<double>(100.0 * dwValue) / static_cast<double>(dwTotal);

	return dblPercentage;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CompareDirectories(const CString& rstrDir1, const CString& rstrDir2)
{
	// use case insensitive compare as a starter
	if (rstrDir1.CompareNoCase(rstrDir2)==0)
	{
		return 0;
	}

	// if one of the paths ends with a '\' the paths may still be equal from the file system's POV
	CString strDir1(rstrDir1);
	CString strDir2(rstrDir2);

	PathRemoveBackslash(strDir1.GetBuffer());	// remove any available backslash
	strDir1.ReleaseBuffer();
	PathRemoveBackslash(strDir2.GetBuffer());	// remove any available backslash
	strDir2.ReleaseBuffer();

	return strDir1.CompareNoCase(strDir2);		// compare again
}

TCHAR* md4str(const uchar *hash, TCHAR *pszHash)
{
	TCHAR		*pcHashCopy = pszHash;
	unsigned	uiCh;

	for (int i = 0; i < 16; i++, pszHash += 2)
	{
		uiCh = static_cast<unsigned>(hash[i]);
		pszHash[0] = g_acHexDigits[uiCh >> 4];
		pszHash[1] = g_acHexDigits[uiCh & 0xf];
	}
	*pszHash = _T('\0');
	return pcHashCopy;
}

CString __stdcall HashToString(const uchar *hash)
{
	TCHAR	acHash[MAX_HASHSTR_SIZE];

	return md4str(hash, acHash);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	StringToHash() converts hexadecimal string into hash (case insensitive).
//		Params:
//			strHexStr - string to convert;
//			pbyteHash - output hash buffer.
//		Return: pointer to hash buffer, in case of error zero hash is returned.
byte* __stdcall StringToHash(const CString &strHexStr, byte *pbyteHash)
{
	for (;;)
	{
		if (strHexStr.GetLength() == 32)
		{
			unsigned uiDgt1, uiDgt2, ui;

			for (ui = 0; ui < 16; ui++)
			{
				if ((uiDgt1 = HexChr2Num(strHexStr.GetAt(ui * 2))) > 15)
					break;
				if ((uiDgt2 = HexChr2Num(strHexStr.GetAt(ui * 2 + 1))) > 15)
					break;
				pbyteHash[ui] = static_cast<byte>((uiDgt1 << 4) | uiDgt2);
			}
			if (ui == 16)
				break;
		}
		md4clr(pbyteHash);
		break;
	}
	return pbyteHash;
}

#define _STRVAL(op)	{_T(#op), op}
typedef struct {
	const TCHAR *pcOpcode;
	byte byteOpcode;
} OpCodeStr;

CString DbgGetClientTCPOpcode(bool bEMulePck, byte byteOpcode)
{
	static const OpCodeStr aDonkeyOpcodes[] = {
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
		_STRVAL(OP_CHANGE_SLOT),
		_STRVAL(OP_QUEUERANK),
		_STRVAL(OP_ASKSHAREDDIRS),
		_STRVAL(OP_ASKSHAREDFILESDIR),
		_STRVAL(OP_ASKSHAREDDIRSANS),
		_STRVAL(OP_ASKSHAREDFILESDIRANS),
		_STRVAL(OP_ASKSHAREDDENIEDANS)
	};
	static const OpCodeStr aMuleOpcodes[] = {
		_STRVAL(OP_EMULEINFO),
		_STRVAL(OP_EMULEINFOANSWER),
		_STRVAL(OP_COMPRESSEDPART),
		_STRVAL(OP_QUEUERANKING),
		_STRVAL(OP_FILEDESC),
		_STRVAL(OP_REQUESTSOURCES),
		_STRVAL(OP_ANSWERSOURCES),
		_STRVAL(OP_REQUESTSOURCES2),
		_STRVAL(OP_ANSWERSOURCES2),
		_STRVAL(OP_PUBLICKEY),
		_STRVAL(OP_SIGNATURE),
		_STRVAL(OP_SECIDENTSTATE),
		_STRVAL(OP_MULTIPACKET),
		_STRVAL(OP_MULTIPACKETANSWER),
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
		_STRVAL(OP_CHATCAPTCHAREQ),
		_STRVAL(OP_CHATCAPTCHARES)
	};
	const OpCodeStr	*pOpcode;
	unsigned		uiTabSz;

	if (bEMulePck)
	{
		uiTabSz = ARRSIZE(aMuleOpcodes);
		pOpcode = aMuleOpcodes;
	}
	else
	{
		uiTabSz = ARRSIZE(aDonkeyOpcodes);
		pOpcode = aDonkeyOpcodes;
	}

	for (unsigned ui = 0; ui < uiTabSz; ui++)
	{
		if (pOpcode[ui].byteOpcode == byteOpcode)
			return pOpcode[ui].pcOpcode;
	}

	CString strOpcode;

	strOpcode.Format(_T("opcode %#x"), byteOpcode);
	return strOpcode;
}
#undef _STRVAL

ULONGLONG GetDiskFileSize(LPCTSTR pszFilePath)
{
	static BOOL _bInitialized = FALSE;
	static DWORD (WINAPI *_pfnGetCompressedFileSize)(LPCTSTR, LPDWORD) = NULL;

	if (!_bInitialized)
	{
		_bInitialized = TRUE;
#ifdef _UNICODE
		(FARPROC&)_pfnGetCompressedFileSize = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetCompressedFileSizeW");
#else
		(FARPROC&)_pfnGetCompressedFileSize = GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetCompressedFileSizeA");
#endif
	}

	// If the file is not compressed nor sparse, 'GetCompressedFileSize' returns the 'normal' file size.
	if (_pfnGetCompressedFileSize)
	{
		ULONGLONG ullCompFileSize;
		LPDWORD pdwCompFileSize = (LPDWORD)&ullCompFileSize;
		pdwCompFileSize[0] = (*_pfnGetCompressedFileSize)(pszFilePath, &pdwCompFileSize[1]);
		if (pdwCompFileSize[0] != INVALID_FILE_SIZE || GetLastError() == NO_ERROR)
			return ullCompFileSize;
	}

	// If 'GetCompressedFileSize' failed or is not available, use the default function
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(pszFilePath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;
	FindClose(hFind);

	return (ULONGLONG)fd.nFileSizeHigh << 32 | (ULONGLONG)fd.nFileSizeLow;
}

bool IsRightToLeftLanguage()
{
#ifndef NEW_SOCKETS_ENGINE
	switch (g_App.m_pPrefs->GetLanguageID())
	{
		case MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT):
		case MAKELANGID(LANG_FARSI, SUBLANG_DEFAULT):
			return true;
	}
#endif //NEW_SOCKETS_ENGINE
	return false;
}

CString GetLocalDecimalPoint(bool bSystem /*= false*/)
{
	CString	strResult;

	if (bSystem)
	{
		int	iSize = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SDECIMAL, NULL, 0);

		if (iSize > 0)
		{
			GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SDECIMAL, strResult.GetBufferSetLength(iSize), iSize);
			strResult.ReleaseBuffer();
		}
	}

	if (strResult.IsEmpty())
		strResult = localeconv()->decimal_point;

	return strResult;
}

CString GetLocalThousandsSep(bool bSystem /*= false*/)
{
	CString	strResult;

	if (bSystem)
	{
		int	iSize = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, NULL, 0);

		if (iSize > 0)
		{
			GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, strResult.GetBufferSetLength(iSize), iSize);
			strResult.ReleaseBuffer();
		}
	}

	if (strResult.IsEmpty())
		strResult = localeconv()->thousands_sep;

	return strResult;
}

CString GetLocalNegativeSign(bool bSystem /*= false*/)
{
	CString	strResult;

	if (bSystem)
	{
		int	iSize = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SNEGATIVESIGN, NULL, 0);

		if (iSize > 0)
		{
			GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SNEGATIVESIGN, strResult.GetBufferSetLength(iSize), iSize);
			strResult.ReleaseBuffer();
		}
	}

	if (strResult.IsEmpty())
		strResult = localeconv()->negative_sign;

	return strResult;
}

int GetClientListActionFromShortcutCode(short nCode, CUpDownClient* pClient)
{
	int iMessage = 0;
#ifndef NEW_SOCKETS_ENGINE

	if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_SRC_DETAILS))
	{
		iMessage = MP_DETAIL;
	}
	else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_SRC_FRIEND))
	{
		if (pClient != NULL)
			iMessage = (pClient->IsFriend()) ? MP_REMOVEFRIEND : MP_ADDFRIEND;
	}
	else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_SRC_MSG))
	{
		iMessage = MP_MESSAGE;
	}
	else if (nCode == g_App.m_pPrefs->GetShortcutCode(SCUT_SRC_SHAREDFILES))
	{
		if ((pClient != NULL) && pClient->GetViewSharedFilesSupport())
			iMessage = MP_SHOWLIST;
	}

#endif //NEW_SOCKETS_ENGINE
	return iMessage;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsStolenName(const CString &strUserName)
{
	static const TCHAR *s_apcFullStolenNames[] =
	{
		_T("pbwll"),
		_T("unix user"),
		_T("http://emule-element.tk")
	};
	static const TCHAR *s_apcPartialStolenNames[] =
	{
		_T("$GAM3R$"),
		_T("G@m3r´s Edit"),
		_T("G@m3rs Edit"),
		_T("[RAMMSTEIN"),
		_T("[toXic]"),
		_T("Leecha"),
		_T("leecha"),
		_T("darkmule"),
		_T("DarkMule"),
		_T("eVortex"),
		_T("|eVorte|X|"),
		_T("MISON"),
		_T("Mison"),
		_T("[miciolino.de]")
	};
	static const TCHAR *s_apcStartsWith[] =
	{
		_T("EDD ")
	};

	if (!strUserName.IsEmpty())
	{
		for (unsigned ui = 0; ui < ARRSIZE(s_apcFullStolenNames); ui++)
		{
			if (strUserName == s_apcFullStolenNames[ui])
				return true;
		}
		for (unsigned ui = 0; ui < ARRSIZE(s_apcPartialStolenNames); ui++)
		{
			if (strUserName.Find(s_apcPartialStolenNames[ui]) >= 0)
				return true;
		}
		for (unsigned ui = 0; ui < ARRSIZE(s_apcStartsWith); ui++)
		{
			if (strUserName.Find(s_apcStartsWith[ui]) == 0)
				return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsLeecherType(const CString &strModVersion)
{
	static const TCHAR *s_apcPartialLeecherModVersions[] =
	{
		_T("00de.de"),
		_T("LioNetwork"),
		_T("element"),
		_T("Plus Plus"),
		_T("LH"),
		_T("Mison"),
		_T("eVortex"),
		_T("Rappi"),
		_T("XvooM"),
		_T("Plus+")
	};

	if (!strModVersion.IsEmpty())
	{
		for (unsigned ui = 0; ui < ARRSIZE(s_apcPartialLeecherModVersions); ui++)
		{
			if (strModVersion.Find(s_apcPartialLeecherModVersions[ui]) >= 0)
				return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int __stdcall GetItemUnderMouse(CListCtrl *ctrl)
{
	EMULE_TRY

	LVHITTESTINFO	hit;

	::GetCursorPos(&hit.pt);
	ctrl->ScreenToClient(&hit.pt);

//	Return the index of parent item
	if ((ctrl->SubItemHitTest(&hit) != -1) && (hit.iSubItem == 0) && (hit.flags & LVHT_ONITEM))
		return hit.iItem;

	EMULE_CATCH

	return (-1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EnumClientTypes GetClientTypeFromCompatibilityTag(uint32 dwTagValue)
{
	switch (dwTagValue)
	{
		case 0:
			return SO_EMULE;

		case 2:
			return SO_XMULE;

		case 3:
			return SO_AMULE;

		case 4:
		case 40:	// new Shareaza ID
			return SO_SHAREAZA;

		case PLUS_COMPATIBLECLIENTID:
			return SO_PLUS;

		case 10:
			return SO_MLDONKEY;

		case 20:
			return SO_LPHANT;

		default:
			return SO_UNKNOWN;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CreateAllDirectories() creates full path (not only last directory like CreateDirectory()).
//		Note: no need to check directory existence before this call.
//		Params:
//			pstrPath - directory(ies) to cleate (trailing '\' not required, but allowed).
void CreateAllDirectories(const CString *pstrPath)
{
	if (!::CreateDirectory(*pstrPath, NULL) && (::GetLastError() == ERROR_PATH_NOT_FOUND))
	{
		int	iPos = 0;

		while (((iPos = pstrPath->Find(_T('\\'), iPos + 1)) >= 0) && (iPos < (pstrPath->GetLength() - 1)))
			::CreateDirectory(pstrPath->Left(iPos), NULL);
		::CreateDirectory(*pstrPath, NULL);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FractionalRate2String()
void FractionalRate2String(CString *pstrOut, uint32 dwRate)
{
	if ((dwRate % 10) == 0)
		pstrOut->Format(_T("%u"), dwRate / 10);
	else
		pstrOut->Format(_T("%.1f"), static_cast<double>(dwRate) / 10.0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsDaylightSavingTimeActive(int *piDaylightBias)
{
	TIME_ZONE_INFORMATION tzi;

	if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_DAYLIGHT)
		return false;
	*piDaylightBias = tzi.DaylightBias;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsFileOnNTFSVolume(const CString &strFilePath)
{
	CString	strRootPath(strFilePath);
	BOOL	bResult = PathStripToRoot(strRootPath.GetBuffer());

	strRootPath.ReleaseBuffer();
	if (!bResult)
		return false;

	DWORD	dwMaximumComponentLength = 0, dwFileSystemFlags = 0;
	TCHAR	acFileSystemNameBuffer[128];

	if (!GetVolumeInformation(strRootPath, NULL, 0, NULL, &dwMaximumComponentLength, &dwFileSystemFlags, acFileSystemNameBuffer, ARRSIZE(acFileSystemNameBuffer)))
		return false;
	return (_tcscmp(acFileSystemNameBuffer, _T("NTFS")) == 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsFileOnFATVolume(const CString &strFilePath)
{
	CString	strRootPath(strFilePath);
	BOOL	bResult = PathStripToRoot(strRootPath.GetBuffer());

	strRootPath.ReleaseBuffer();
	if (!bResult)
		return false;

	DWORD	dwMaximumComponentLength = 0, dwFileSystemFlags = 0;
	TCHAR	acFileSystemNameBuffer[128];

	if (!GetVolumeInformation(strRootPath, NULL, 0, NULL, &dwMaximumComponentLength, &dwFileSystemFlags, acFileSystemNameBuffer, ARRSIZE(acFileSystemNameBuffer)))
		return false;
	return (_tcsnicmp(acFileSystemNameBuffer, _T("FAT"), 3) == 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsAutoDaylightTimeSetActive()
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AdjustNTFSDaylightFileTime(uint32 *pdwFileDate, const CString &strFilePath)
{
	if (*pdwFileDate == 0)
		return false;

	int	iDaylightBias;

	if (IsDaylightSavingTimeActive(&iDaylightBias))
	{
		if (IsAutoDaylightTimeSetActive() && IsFileOnNTFSVolume(strFilePath))
		{
			*pdwFileDate += 60 * iDaylightBias;
			return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetAdjustedFileTime() returns file date considering daylight saving time
//	To avoid issues with weird dates, the method used to get the date has to be
//	similar to one used in checking routines (e.g. AddFilesFromDirectory())
void GetAdjustedFileTime(const CString &strFName, uint32 *pdwTime)
{
	WIN32_FIND_DATA	findFileData;
	HANDLE			hFind = FindFirstFile(strFName, &findFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		*pdwTime = static_cast<uint32>(CTime(findFileData.ftLastWriteTime).GetTime());
		AdjustNTFSDaylightFileTime(pdwTime, strFName);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetFileTypeForWebServer() returns file type according to WebServer requirements
//		Params: strFileName - file name (case insensitive)
const TCHAR* GetFileTypeForWebServer(const CString &strFileName)
{
	static const TCHAR *const s_apcTypes[ED2KFT_COUNT] =
	{
		_T("other"),		//ED2KFT_ANY
		_T("music"),		//ED2KFT_AUDIO
		_T("movie"),		//ED2KFT_VIDEO
		_T("picture"),		//ED2KFT_IMAGE
		_T("application"),	//ED2KFT_PROGRAM
		_T("document"),		//ED2KFT_DOCUMENT
		_T("archive"),		//ED2KFT_ARCHIVE
		_T("cdimage")		//ED2KFT_CDIMAGE
	};
	return s_apcTypes[static_cast<unsigned>(GetED2KFileTypeID(strFileName))];
}
#ifdef EP_SPIDERWEB
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	StructuredExceptionHandler() intercepts all system crashes and logs crash information.
void StructuredExceptionHandler(unsigned int uiCode, EXCEPTION_POINTERS *pExp)
{
	EMULE_TRY

	if (g_App.m_pMDlg != NULL)
	{
		g_App.m_pMDlg->AddLogLine( LOG_FL_DBG | LOG_RGB_ERROR,
			_T("EXCEPTION! code=%#x addr=%#x AX=%#x BX=%#x CX=%#x DX=%#x SI=%#x DI=%#x BP=%#x"),
			uiCode, pExp->ExceptionRecord->ExceptionAddress,
			pExp->ContextRecord->Eax, pExp->ContextRecord->Ebx, pExp->ContextRecord->Ecx,
			pExp->ContextRecord->Edx, pExp->ContextRecord->Esi, pExp->ContextRecord->Edi, pExp->ContextRecord->Ebp );
	}

	EMULE_CATCH2
}
#endif //EP_SPIDERWEB
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetAppImageListColorFlag()
{
	HDC hdcScreen = ::GetDC(NULL);
	int iColorBits = GetDeviceCaps(hdcScreen, BITSPIXEL) * GetDeviceCaps(hdcScreen, PLANES);
	::ReleaseDC(NULL, hdcScreen);
	int iIlcFlag;
	if (iColorBits >= 32)
		iIlcFlag = ILC_COLOR32;
	else if (iColorBits >= 24)
		iIlcFlag = ILC_COLOR24;
	else if (iColorBits >= 16)
		iIlcFlag = ILC_COLOR16;
	else if (iColorBits >= 8)
		iIlcFlag = ILC_COLOR8;
	else if (iColorBits >= 4)
		iIlcFlag = ILC_COLOR4;
	else
		iIlcFlag = ILC_COLOR;

	return iIlcFlag;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RC4 Encryption
static __inline void swap_byte(byte *a, byte *b)
{
	byte byteSwap = *a;

	*a = *b;
	*b = byteSwap;
}

RC4_Key_Struct* RC4CreateKey(const byte *pbyteKeyData, uint32 dwLen, RC4_Key_Struct *key, bool bSkipDiscard/*=false*/)
{
	unsigned	uiIdx1, uiIdx2;
	byte		*pbyteState;

	if (key == NULL)
		key = new RC4_Key_Struct;

	pbyteState= &key->abyteState[0];
	for (int i = 0; i < 256; i++)
		pbyteState[i] = static_cast<byte>(i);

	key->byteX = 0;
	key->byteY = 0;
	uiIdx1 = uiIdx2 = 0;
	for (int i = 0; i < 256; i++)
	{
		uiIdx2 = (pbyteKeyData[uiIdx1] + pbyteState[i] + uiIdx2) & 255;
		swap_byte(&pbyteState[i], &pbyteState[uiIdx2]);
		uiIdx1 = ((uiIdx1 + 1u) % dwLen) & 255u;
	}
	if (!bSkipDiscard)
		RC4Crypt(NULL, NULL, 1024, key);
	return key;
}

void RC4Crypt(const byte *pbyteIn, byte *pbyteOut, uint32 dwLen, RC4_Key_Struct *key)
{
	ASSERT(key != NULL && dwLen > 0);
	if (key == NULL)
		return;

	unsigned	uiXorIdx, uiX = key->byteX, uiY = key->byteY;
	byte		*pbyteState = &key->abyteState[0];

	for (uint32 i = 0; i < dwLen; i++)
	{
		uiX = (uiX + 1u) & 255u;
		uiY = (pbyteState[uiX] + uiY) & 255;
		swap_byte(&pbyteState[uiX], &pbyteState[uiY]);
		
		if (pbyteIn != NULL)
		{
			uiXorIdx = (pbyteState[uiX] + pbyteState[uiY]) & 255;
			pbyteOut[i] = pbyteIn[i] ^ pbyteState[uiXorIdx];
		}
	}
	key->byteX = static_cast<byte>(uiX);
	key->byteY = static_cast<byte>(uiY);
}
