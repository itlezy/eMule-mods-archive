//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#pragma once

#include "KnownFile.h"
#include "memcpy_amd.h"	// To be renamed
#ifndef NEW_SOCKETS_ENGINE
#include "PartFile.h"
#endif //NEW_SOCKETS_ENGINE

#define _WINVER_NT4_	0x0004
#define _WINVER_95_		0x0004
#define _WINVER_98_		0x0A04
#define _WINVER_ME_		0x5A04
#define _WINVER_2K_		0x0005
#define _WINVER_XP_		0x0105
#define _WINVER_SE_		0x0205	//Win Server 2003
#define _WINVER_VISTA_	0x0006

#ifdef _DEBUG
#define OUTPUT_DEBUG_TRACE() { \
	try{ \
	CString strTrace; \
	strTrace.Format(_T("%s:%u, %s\n"), _tcsrchr(_T(__FILE__), _T('\\')) + 1, __LINE__, _T(__FUNCTION__)); \
	OutputDebugString(strTrace); \
	} catch(...){} \
}
#else
#define OUTPUT_DEBUG_TRACE() ((void)0)
#endif

extern const TCHAR g_acHexDigits[16];

double __fastcall GetPercent(uint64 qwValue, uint64 qwTotalValue);
double __fastcall GetPercent(uint32 dwValue, uint32 dwTotal);
CString __stdcall CastItoXBytes(uint64 count);
CString __stdcall CastItoIShort(uint64 number);
CString __stdcall CastItoThousands(_int64 count);
CString __stdcall CastSecondsToHM(sint32 iCount);
CString __stdcall CastSecondsToLngHM(uint32 dwSecs);
void 	ShellOpenFile(const CString& name);
CString __stdcall GetResString(UINT dwStringID);
CString __stdcall GetResString(UINT dwStringID, WORD uLanguageID);
__declspec(noinline) void __stdcall GetResString(CString *pstrOutput, UINT dwStringID);

//Simple encryption
CString Crypt(const CString &strNormal);
CString Decrypt(const CString &strCrypted);

ULONGLONG GetDiskFileSize(LPCTSTR pszFilePath);

bool CheckIsRegistrySet();
bool Ask4RegFix(bool dontAsk = false); // Barry - Allow forced update without prompt
void RevertReg(void); // Barry - Restore previous values
int		GetMaxConnections();
int		UpdateURLMenu(CMenu &menu);
void RunURL(CAbstractFile *file, CString urlpattern);

WORD	DetectWinVersion(bool *pbNTBased);
uint64	GetFreeDiskSpaceX(LPCTSTR pDirectory);
//	FillImgLstWith16x16Icons() fills image list with 16x16 icons.
void __stdcall FillImgLstWith16x16Icons(CImageList *pImgLst, const uint16 *uResIDs, unsigned uiResAmount);
//For Rate File
CString GetRatingString(EnumPartFileRating eRating);
int		GetAppImageListColorFlag();

#if 0	// Can be required later for AICH
CString EncodeBase32(const unsigned char* buffer, unsigned int bufLen);
#endif
CString StringLimit(const CString& in, unsigned uiLen);
CString ConcatFullPath(const CString &strPath, const CString &strFname);
// Returns client version string based on client type
CString GetClientVersionString(EnumClientTypes eClienType, uint32 dwVersion);
// returns upload queue client status based as string
void GetStatusULQueueString(CString *pstrOut, EnumULQState State);
// returns client type as string
CString GetClientNameString(EnumClientTypes clienttype);
unsigned HexChr2Num(TCHAR cCh);
CString BrowseFolder(HWND hwndOwner, LPCTSTR pszTitle, LPCTSTR pszStartFolder); //<<-- enkeyDEV(Ottavio84) -ChangeDir-
bool DialogBrowseFile(CString &rstrPath, LPCTSTR pcFilters, LPCTSTR pcDefFileName = NULL, DWORD dwFlags = 0, bool bOpenFile = true, LPCTSTR pcDefDir = NULL);
CString CleanupFilename(const CString &inStr);
int GetErrorMessage(DWORD dwError, CString &rstrErrorMsg, DWORD dwFlags = 0);
CString GetErrorMessage(DWORD dwError, DWORD dwFlags = 0);
CString GetErrorMessage(CException* e);
bool 	IsRightToLeftLanguage();
CString	GetLocalDecimalPoint(bool bSystem = false);
CString	GetLocalThousandsSep(bool bSystem = false);
CString	GetLocalNegativeSign(bool bSystem = false);
bool	IsStolenName(const CString& strUserName);
bool	IsLeecherType(const CString& strModVersion);
#ifndef NEW_SOCKETS_ENGINE
#define YesNoStr(bYes)	GetResString((bYes) ? IDS_YES : IDS_NO)
#else
#define YesNoStr(bYes)	_T("")
#endif //NEW_SOCKETS_ENGINE
int __stdcall GetItemUnderMouse(CListCtrl *ctrl);

#define	MAX_HASHSTR_SIZE (16*2+1)
CString __stdcall HashToString(const uchar *hash);
TCHAR* md4str(const uchar *hash, TCHAR *pszHash);

//	Convert char to upper case
#define CHR2UP(ch)		((ch) & ~(_T('A') ^ _T('a')))

//	StringToHash() converts hexadecimal string into hash (case insensitive).
byte* __stdcall StringToHash(const CString &strHexStr, byte *pbyteHash);

__inline int CompareUnsigned(uint32 dwVal1, uint32 dwVal2)
{
	if (dwVal1 < dwVal2)
		return -1;
	if (dwVal1 > dwVal2)
		return 1;
	return 0;
}

__inline int CompareInt64(uint64 qwVal1, uint64 qwVal2)
{
	uint64	qwRes = (qwVal1 - qwVal2);

	return (qwRes == 0) ? 0 : (static_cast<int>(qwRes >> 32ui64) | 1);
}

__inline CString GetPathToFile(const CString& p)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];

	_tsplitpath(p, drive, dir, NULL, NULL);

	return CString(drive) + dir;
}

__inline CString GetFileExtension(const CString& p)
{
	TCHAR ext[_MAX_EXT];
	_tsplitpath(p, NULL, NULL, NULL, ext);
	return CString(ext);
}

__inline CString RemoveFileExtension(const CString& p)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];

	_tsplitpath(p, drive, dir, fname, NULL);

	return CString(drive) + dir + fname;
}

//	IP conversions
#ifdef _UNICODE
uint32 inet_addr(LPCTSTR cp);
#endif
CString ipstr(uint32 dwIP);
void ipstr(CString *pstrOut, uint32 dwIP);

__inline CString ipstr(in_addr IP)
{
	return ipstr(*reinterpret_cast<uint32*>(&IP));
}

__inline void ipstr(CString *pstrOut, in_addr IP)
{
	ipstr(pstrOut, *reinterpret_cast<uint32*>(&IP));
}

//	Use intrinsic operations instead of calling ws2_32.dll
#define fast_htonl	_byteswap_ulong
#define fast_htons	_byteswap_ushort

__inline bool IsLowID(uint32 dwID)
{
	return (dwID < 0x1000000);
}

//	The validity check of HybridID based on invalid IP ranges
bool IsGoodHybridID(uint32 dwHybridID);

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
bool SelectDir(HWND hWnd, LPTSTR pszPath, LPCTSTR pszTitle = NULL, LPCTSTR pszDlgTitle = NULL);
void MakeFolderName(TCHAR* path);
int CompareDirectories(const CString& rstrDir1, const CString& rstrDir2);

TCHAR* stristr(const TCHAR *str1, const TCHAR *str2);
size_t fast_strlen(const char *pcStr);

COLORREF LightenColor(COLORREF crColor, int i);


__inline void *memcpy2(void* dst, const void *src, size_t size)
{
	return ::memcpy_optimized(dst, src, size);
}

__inline void *memset2(void* dst, int c, size_t size)
{
	return ::memset_optimized(dst, c, size);
}

__inline void memzero(void* dst, size_t size)
{
	::memzero_optimized(dst, size);
}

typedef struct
{
	BYTE		hash[16];
} HashType;

// md4cpy -- replacement for memcpy(dst,src,16)
__inline void md4cpy(void* dst, const void* src)
{
	*reinterpret_cast<HashType*>(dst) = *reinterpret_cast<const HashType*>(src);
}

// md4cmp -- replacement for memcmp(hash1,hash2,16)
// Like 'memcmp' this function returns 0, if hash1==hash2, and !0, if hash1!=hash2.
// NOTE: Do *NOT* use that function for determining if hash1<hash2 or hash1>hash2.
__inline int md4cmp(const void* hash1, const void* hash2) {
	return !(((uint32*)hash1)[0] == ((uint32*)hash2)[0] &&
		((uint32*)hash1)[1] == ((uint32*)hash2)[1] &&
		((uint32*)hash1)[2] == ((uint32*)hash2)[2] &&
		((uint32*)hash1)[3] == ((uint32*)hash2)[3]);
}
// Like 'memcmp' this function returns 0 if hash1==0, and !0 if hash1!=0
// This implementation improves CPU jump prediction as it has only one cmp/jmp felt
__inline int md4cmp0(const void *pH) {
	return (((uint32*)pH)[0] | ((uint32*)pH)[1] | ((uint32*)pH)[2] | ((uint32*)pH)[3]);
}

// md4clr -- replacement for memset(hash,0,16)
__inline void md4clr(const void* hash) {
	((uint32*)hash)[0] = ((uint32*)hash)[1] = ((uint32*)hash)[2] = ((uint32*)hash)[3] = 0;
}

//	Services to peek/poke integers from/to buffer
#define PEEK_QWORD(ptr)			*((uint64*)(ptr))
#define PEEK_DWORD(ptr)			*((uint32*)(ptr))
#define PEEK_WORD(ptr)			*((uint16*)(ptr))
#define POKE_QWORD(ptr, qw)		*((uint64*)(ptr)) = qw
#define POKE_DWORD(ptr, dw)		*((uint32*)(ptr)) = dw
#define POKE_WORD(ptr, w)		*((uint16*)(ptr)) = w

// called in the PreTranslateMessage function for shortcut managemment of client items
// (i.e. in CDownloadListCtrl, CUploadListCtrl, CQueueListCtrl and CClientListCtrl)
int GetClientListActionFromShortcutCode(short nCode, CUpDownClient* pClient);

EnumClientTypes GetClientTypeFromCompatibilityTag(uint32 dwTagValue);

//	CreateAllDirectories() creates full path (not only last directory like CreateDirectory()).
void CreateAllDirectories(const CString *pstrPath);

void FractionalRate2String(CString *pstrOut, uint32 dwRate);
#define String2FranctionalRate(str)		static_cast<uint32>((_tstof(str) * 10) + 0.5)

__inline uint32 TieUploadDownload(uint32 dwUL, uint32 dwDL)
{
	if (dwUL < 100)
	{
		if (dwUL < 40)
		{
			if ((3 * dwUL) < dwDL)
				dwDL = 3 * dwUL;
		}
		else if ((4 * dwUL) < dwDL)
			dwDL = 4 * dwUL;
	}
	return dwDL;
}

bool AdjustNTFSDaylightFileTime(uint32 *pdwFileDate, const CString &strFilePath);
bool IsFileOnFATVolume(const CString &strFilePath);
void GetAdjustedFileTime(const CString &strFName, uint32 *pdwTime);

//	GetFileTypeForWebServer() returns file type according to WebServer requirements
const TCHAR* GetFileTypeForWebServer(const CString &strFileName);

#ifdef EP_SPIDERWEB
//	StructuredExceptionHandler() intercepts all system crashes and logs crash information.
void StructuredExceptionHandler(unsigned int uiCode, EXCEPTION_POINTERS *pExp);
#endif

///////////////////////////////////////////////////////////////////////////////
// RC4 Encryption
struct RC4_Key_Struct
{
	byte abyteState[256];
	byte byteX;
	byte byteY;
};

RC4_Key_Struct* RC4CreateKey(const byte *pbyteKeyData, uint32 dwLen, RC4_Key_Struct *key, bool bSkipDiscard = false);
void RC4Crypt(const byte *pbyteIn, byte *pbyteOut, uint32 dwLen, RC4_Key_Struct *key);

///////////////////////////////////////////////////////////////////////////////
// Debugging
CString DbgGetClientTCPOpcode(bool bEMulePck, byte byteOpcode);
