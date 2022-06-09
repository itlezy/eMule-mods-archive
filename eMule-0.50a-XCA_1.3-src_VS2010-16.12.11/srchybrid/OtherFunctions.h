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
#pragma once
#include <zlib/zlib.h>

class CAbstractFile;
class CKnownFile;
struct Requested_Block_Struct;
class CUpDownClient;
class CAICHHash;
class CPartFile;
class CSafeMemFile;
class CShareableFile;

enum EFileType { 
	FILETYPE_UNKNOWN,
	FILETYPE_EXECUTABLE,
	ARCHIVE_ZIP,
	ARCHIVE_RAR,
	ARCHIVE_ACE,
	IMAGE_ISO,
	AUDIO_MPEG,
	VIDEO_AVI,
	VIDEO_MPG,
	WM,
	PIC_JPG,
	PIC_PNG,
	PIC_GIF,
	DOCUMENT_PDF
};


#define ROUND(x) (floor((float)x+0.5f))

///////////////////////////////////////////////////////////////////////////////
// Low level str
//
__inline char* nstrdup(const char* const todup){
   size_t len = strlen(todup) + 1;
   return (char*)memcpy(new char[len], todup, len);
}

const char *stristr(const char *str1, const char *str2);
const TCHAR *wcsistr(const TCHAR *str1, const TCHAR *str2);
CString GetNextString(const CString& rstr, LPCTSTR pszTokens, int& riStart);
CString GetNextString(const CString& rstr, TCHAR chToken, int& riStart);

bool IsHexDigit(sint_ptr c);

///////////////////////////////////////////////////////////////////////////////
// String conversion
//
//Xman Xtreme Mod
//default is 99, this means, we use the old method (from 0.30)
CString CastItoXBytes(uint16 count, bool isK = false, bool isPerSec = false, uint32 decimal = 99);
CString CastItoXBytes(uint32 count, bool isK = false, bool isPerSec = false, uint32 decimal = 99);
CString CastItoXBytes(uint64 count, bool isK = false, bool isPerSec = false, uint32 decimal = 99);
CString CastItoXBytes(float count, bool isK = false, bool isPerSec = false, uint32 decimal = 99);
CString CastItoXBytes(double count, bool isK = false, bool isPerSec = false, uint32 decimal = 99);
#if defined(_DEBUG) && defined(USE_DEBUG_EMFILESIZE)
CString CastItoXBytes(EMFileSize count, bool isK = false, bool isPerSec = false, uint32 decimal = 99);
#endif
//Xman end
/*
CString CastItoXBytes(uint16 count, bool isK = false, bool isPerSec = false, uint32 decimal = 2);
CString CastItoXBytes(uint32 count, bool isK = false, bool isPerSec = false, uint32 decimal = 2);
CString CastItoXBytes(uint64 count, bool isK = false, bool isPerSec = false, uint32 decimal = 2);
CString CastItoXBytes(float count, bool isK = false, bool isPerSec = false, uint32 decimal = 2);
CString CastItoXBytes(double count, bool isK = false, bool isPerSec = false, uint32 decimal = 2);
#if defined(_DEBUG) && defined(USE_DEBUG_EMFILESIZE)
CString CastItoXBytes(EMFileSize count, bool isK = false, bool isPerSec = false, uint32 decimal = 2);
#endif
*/
CString CastItoIShort(uint16 count, bool isK = false, uint32 decimal = 2);
CString CastItoIShort(uint32 count, bool isK = false, uint32 decimal = 2);
CString CastItoIShort(uint64 count, bool isK = false, uint32 decimal = 2);
CString CastItoIShort(float count, bool isK = false, uint32 decimal = 2);
CString CastItoIShort(double count, bool isK = false, uint32 decimal = 2);
CString CastSecondsToHM(uint_ptr seconds);
CString	CastSecondsToLngHM(uint_ptr seconds);
CString GetFormatedUInt(ULONG ulVal);
CString GetFormatedUInt64(ULONGLONG ullVal);
void SecToTimeLength(uint_ptr ulSec, CStringA& rstrTimeLength);
void SecToTimeLength(uint_ptr ulSec, CStringW& rstrTimeLength);
bool RegularExpressionMatch(CString regexpr, CString teststring);

///////////////////////////////////////////////////////////////////////////////
// URL conversion
//
CString URLDecode(const CString& sIn, bool bKeepNewLine = false);
CString URLEncode(const CString& sIn);
CString EncodeURLQueryParam(const CString& rstrQuery);
CString MakeStringEscaped(CString in);
CString RemoveAmbersand(const CString& rstr);
CString	StripInvalidFilenameChars(const CString& strText);


///////////////////////////////////////////////////////////////////////////////
// Hex conversion
//
CString EncodeBase32(const unsigned char* buffer, uint_ptr bufLen);
CString EncodeBase16(const unsigned char* buffer, uint_ptr bufLen);
uint_ptr DecodeLengthBase16(uint_ptr base16Length);
bool DecodeBase16(const TCHAR *base16Buffer, uint_ptr base16BufLen, byte *buffer, uint_ptr bufflen);
uint_ptr DecodeBase32(LPCSTR pszInput, uchar* paucOutput, uint_ptr nBufferLen);
uint_ptr DecodeBase32(LPCTSTR pszInput, uchar* paucOutput, uint_ptr nBufferLen);
uint_ptr DecodeBase32(LPCTSTR pszInput, CAICHHash& Hash);

///////////////////////////////////////////////////////////////////////////////
// File/Path string helpers
//
void MakeFoldername(CString &path);
CString RemoveFileExtension(const CString& rstrFilePath);
int CompareDirectories(const CString& rstrDir1, const CString& rstrDir2);
CString StringLimit(CString in, UINT length);
CString CleanupFilename(CString filename, bool bExtension = true);
CString ValidFilename(CString filename);
bool ExpandEnvironmentStrings(CString& rstrStrings);
int CompareLocaleString(LPCTSTR psz1, LPCTSTR psz2);
int CompareLocaleStringNoCase(LPCTSTR psz1, LPCTSTR psz2);
int __cdecl CompareCStringPtrLocaleString(const void* p1, const void* p2);
int __cdecl CompareCStringPtrLocaleStringNoCase(const void* p1, const void* p2);
void Sort(CAtlArray<CString>& astr, int (__cdecl *pfnCompare)(const void*, const void*) = CompareCStringPtrLocaleStringNoCase);
int __cdecl CompareCStringPtrPtrLocaleString(const void* p1, const void* p2);
int __cdecl CompareCStringPtrPtrLocaleStringNoCase(const void* p1, const void* p2);
void		Sort(CSimpleArray<const CString*>& apstr, int (__cdecl *pfnCompare)(const void*, const void*) = CompareCStringPtrPtrLocaleStringNoCase);
void		StripTrailingCollon(CString& rstr);
bool		IsUnicodeFile(LPCTSTR pszFilePath);
//int			GetPathDriveNumber(CString path);
EFileType	GetFileTypeEx(CShareableFile* kfile, bool checkextention=true, bool checkfileheader=true, bool nocached=false);
CString		GetFileTypeName(EFileType ftype);
int			IsExtensionTypeOf(EFileType type, LPCTSTR ext);
uint32		LevenshteinDistance(const CString& str1, const CString& str2);
bool		_tmakepathlimit(TCHAR *path, const TCHAR *drive, const TCHAR *dir, const TCHAR *fname, const TCHAR *ext);

///////////////////////////////////////////////////////////////////////////////
// GUI helpers
//
void InstallSkin(LPCTSTR pszSkinPackage);
bool CheckFileOpen(LPCTSTR pszFilePath, LPCTSTR pszFileTitle = NULL);
void ShellOpenFile(CString name);
void ShellOpenFile(CString name, LPCTSTR pszVerb);
bool ShellDeleteFile(LPCTSTR pszFilePath);
CString ShellGetFolderPath(int iCSIDL);
bool SelectDir(HWND hWnd, LPTSTR pszPath, LPCTSTR pszTitle = NULL, LPCTSTR pszDlgTitle = NULL);
BOOL DialogBrowseFile(CString& rstrPath, LPCTSTR pszFilters, LPCTSTR pszDefaultFileName = NULL, DWORD dwFlags = 0,bool openfilestyle=true);
void AddBuddyButton(HWND hwndEdit, HWND hwndButton);
bool InitAttachedBrowseButton(HWND hwndButton, HICON &ricoBrowse);
void GetPopupMenuPos(CListCtrl& lv, CPoint& point);
void GetPopupMenuPos(CTreeCtrl& tv, CPoint& point);
void InitWindowStyles(CWnd* pWnd);
CString GetRateString(UINT rate);
//HWND GetComboBoxEditCtrl(CComboBox& cb);
///HWND ReplaceRichEditCtrl(CWnd* pwndRE, CWnd* pwndParent, CFont* pFont);
int  FontPointSizeToLogUnits(int nPointSize);
bool CreatePointFont(CFont &rFont, int nPointSize, LPCTSTR lpszFaceName);
bool CreatePointFontIndirect(CFont &rFont, const LOGFONT *lpLogFont);


///////////////////////////////////////////////////////////////////////////////
// Resource strings
//
#ifdef USE_STRING_IDS
#define	RESSTRIDTYPE		LPCTSTR
#define	IDS2RESIDTYPE(id)	#id
#define GetResString(id)	_GetResString(#id)
CString _GetResString(RESSTRIDTYPE StringID);
#else//USE_STRING_IDS
#define	RESSTRIDTYPE		UINT
#define	IDS2RESIDTYPE(id)	id
CString GetResString(RESSTRIDTYPE StringID);
#define _GetResString(id)	GetResString(id)
#endif//!USE_STRING_IDS
#ifdef _DEBUG
void InitThreadLocale();
#else
#define InitThreadLocale()          ((void)0)
#endif


///////////////////////////////////////////////////////////////////////////////
// Error strings, Debugging, Logging
//
int GetSystemErrorString(DWORD dwError, CString &rstrError);
int GetModuleErrorString(DWORD dwError, CString &rstrError, LPCTSTR pszModule);
int GetErrorMessage(DWORD dwError, CString &rstrErrorMsg, DWORD dwFlags = 0);
CString GetErrorMessage(DWORD dwError, DWORD dwFlags = 0);
LPCTSTR	GetShellExecuteErrMsg(DWORD_PTR dwShellExecError);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
CString DbgGetHexDump(const uint8* data, UINT size);
void DbgSetThreadName(LPCSTR szThreadName, ...);
void Debug(LPCTSTR pszFmtMsg, ...);
void DebugHexDump(const uint8* data, UINT lenData);
void DebugHexDump(CFile& file);
int GetHashType(const uchar* hash);
LPCTSTR DbgGetHashTypeString(const uchar* hash);
#else
#define DbgSetThreadName(...)          ((void)0)
#endif
CString DbgGetFileInfo(const uchar* hash);
CString DbgGetFileStatus(UINT nPartCount, CSafeMemFile* data);
CString DbgGetClientID(uint32 nClientID);
CString DbgGetDonkeyClientTCPOpcode(UINT opcode);
CString DbgGetMuleClientTCPOpcode(UINT opcode);
CString DbgGetClientTCPOpcode(UINT protocol, UINT opcode);
CString DbgGetClientTCPPacket(UINT protocol, UINT opcode, UINT size);
CString DbgGetBlockInfo(const Requested_Block_Struct* block);
CString DbgGetBlockInfo(uint64 StartOffset, uint64 EndOffset);
CString DbgGetBlockFileInfo(const Requested_Block_Struct* block, const CPartFile* partfile);
CString DbgGetFileMetaTagName(UINT uMetaTagID);
CString DbgGetFileMetaTagName(LPCSTR pszMetaTagID);
CString DbgGetSearchOperatorName(UINT uOperator);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
void DebugRecv(LPCSTR pszMsg, const CUpDownClient* client, const uchar* packet = NULL, uint32 nIP = 0);
void DebugRecv(LPCSTR pszOpcode, uint32 ip, uint16 port);
void DebugSend(LPCSTR pszMsg, const CUpDownClient* client, const uchar* packet = NULL);
void DebugSend(LPCSTR pszOpcode, uint32 ip, uint16 port);
void DebugSendF(LPCSTR pszOpcode, uint32 ip, uint16 port, LPCTSTR pszMsg, ...);
void DebugHttpHeaders(const CStringAArray& astrHeaders);
#endif



///////////////////////////////////////////////////////////////////////////////
// Win32 specifics
//
bool Ask4RegFix(bool checkOnly, bool dontAsk = false, bool bAutoTakeCollections = false); // Barry - Allow forced update without prompt
void BackupReg(void); // Barry - Store previous values
void RevertReg(void); // Barry - Restore previous values
bool DoCollectionRegFix(bool checkOnly);
void AddAutoStart();
void RemAutoStart();
ULONGLONG GetModuleVersion(LPCTSTR pszFilePath);
ULONGLONG GetModuleVersion(HMODULE hModule);

//int GetMaxWindowsTCPConnections();

/*#define _WINVER_95_		0x0400	// 4.0
#define _WINVER_NT4_	0x0401	// 4.1 (baked version)
#define _WINVER_98_		0x040A	// 4.10
#define _WINVER_ME_		0x045A	// 4.90*/
#define _WINVER_2K_		0x0500	// 5.0
#define _WINVER_XP_		0x0501	// 5.1
#define _WINVER_2003_	0x0502	// 5.2
#define _WINVER_VISTA_	0x0600	// 6.0
#define _WINVER_7_		0x0601	// 6.1
#define	_WINVER_S2008_	0x0601	// 6.1

WORD		DetectWinVersion();
sint_ptr	IsRunningXPSP2();
sint_ptr	IsRunningXPSP2OrHigher();
ULONGLONG	GetDiskFileSize(LPCTSTR pszFilePath);
int			GetAppImageListColorFlag();
int			GetDesktopColorDepth();
bool		AddIconGrayscaledToImageList(CImageList& rList, HICON hIcon);


///////////////////////////////////////////////////////////////////////////////
// MD4 helpers
//

__inline BYTE toHex(const BYTE &x){
	return x > 9 ? x + 55: x + 48;
}

//Xman 
// Maella -Code Improvement-
// md4cmp -- replacement for memcmp(hash1,hash2,16)
// Like 'memcmp' this function returns 0, if hash1==hash2, and !0, if hash1!=hash2.
// NOTE: Do *NOT* use that function for determining if hash1<hash2 or hash1>hash2.
__inline int md4cmp(const void* const hash1, const void* const hash2) {
	return !(((uint64*)hash1)[0] == ((uint64*)hash2)[0] &&
		((uint64*)hash1)[1] == ((uint64*)hash2)[1]);
}

__inline bool isnulmd4(const void* hash) {
	return  (((uint64*)hash)[0] == 0 &&
		((uint64*)hash)[1] == 0);
}

// md4clr -- replacement for memset(hash,0,16)
__inline void md4clr(const void* hash) {
	((uint64*)hash)[1] = ((uint64*)hash)[0] = 0;
}

// md4cpy -- replacement for memcpy(dst,src,16)
__inline void md4cpy(void* dst, const void* src) {
	((uint64*)dst)[0] = ((uint64*)src)[0];
	((uint64*)dst)[1] = ((uint64*)src)[1];
}
// Maella

#define	MAX_HASHSTR_SIZE (16*2+1)
CString md4str(const uchar* hash);
CStringA md4strA(const uchar* hash);
void md4str(const uchar* hash, TCHAR* pszHash);
void md4strA(const uchar* hash, CHAR* pszHash);
bool strmd4(const char* pszHash, uchar* hash);
bool wcsmd4(const TCHAR* rstr, uchar* hash);


///////////////////////////////////////////////////////////////////////////////
// Compare helpers
//
#ifdef _WIN64
#define CompareUnsigned_PTR CompareUnsigned64
#define GetFormatedUInt_PTR GetFormatedUInt64
#else
#define CompareUnsigned_PTR CompareUnsigned
#define GetFormatedUInt_PTR GetFormatedUInt
#endif

__inline int CompareUnsigned(uint32 uSize1, uint32 uSize2)
{
	if (uSize1 < uSize2)
		return -1;
	if (uSize1 > uSize2)
		return 1;
	return 0;
}

__inline int CompareUnsignedUndefinedAtBottom(uint32 uSize1, uint32 uSize2, bool bSortAscending)
{
	if (uSize1 == 0 && uSize2 == 0)
		return 0;
	if (uSize1 == 0)
		return bSortAscending ? 1 : -1;
	if (uSize2 == 0)
		return bSortAscending ? -1 : 1;
	return CompareUnsigned(uSize1, uSize2);
}

__inline int CompareUnsigned64(uint64 uSize1, uint64 uSize2)
{
	if (uSize1 < uSize2)
		return -1;
	if (uSize1 > uSize2)
		return 1;
	return 0;
}

__inline int CompareFloat(float uSize1, float uSize2)
{
	if (uSize1 < uSize2)
		return -1;
	if (uSize1 > uSize2)
		return 1;
	return 0;
}

__inline int CompareOptLocaleStringNoCase(LPCTSTR psz1, LPCTSTR psz2)
{
	if (psz1 && psz2)
		return CompareLocaleStringNoCase(psz1, psz2);
	if (psz1)
		return -1;
	if (psz2)
		return 1;
	return 0;
}

__inline int CompareOptLocaleStringNoCaseUndefinedAtBottom(const CString &str1, const CString &str2, bool bSortAscending)
{
	if (str1.IsEmpty() && str2.IsEmpty())
		return 0;
	if (str1.IsEmpty())
		return bSortAscending ? 1 : -1;
	if (str2.IsEmpty())
		return bSortAscending ? -1 : 1;
	return CompareOptLocaleStringNoCase(str1, str2);
}


///////////////////////////////////////////////////////////////////////////////
// ED2K File Type
//
enum EED2KFileType
{
	ED2KFT_ANY				= 0,
	ED2KFT_AUDIO			= 1,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_VIDEO			= 2,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_IMAGE			= 3,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_PROGRAM			= 4,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_DOCUMENT			= 5,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_ARCHIVE			= 6,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_CDIMAGE			= 7,	// ED2K protocol value (eserver 17.6+)
	ED2KFT_EMULECOLLECTION	= 8
};

CString GetFileTypeByName(LPCTSTR pszFileName);
CString GetFileTypeDisplayStrFromED2KFileType(LPCTSTR pszED2KFileType);
LPCSTR GetED2KFileTypeSearchTerm(EED2KFileType iFileID);
EED2KFileType GetED2KFileTypeSearchID(EED2KFileType iFileID);
EED2KFileType GetED2KFileTypeID(LPCTSTR pszFileName);
bool gotostring(CFile &file, const uchar *find, LONGLONG plen);

///////////////////////////////////////////////////////////////////////////////
// IP/UserID
//
void TriggerPortTest(uint16 tcp, uint16 udp);
bool IsGoodIP(uint32 nIP, bool forceCheck = false);
bool IsGoodIPPort(uint32 nIP, uint16 nPort);
bool IsLANIP(uint32 nIP);
uint8 GetMyConnectOptions(bool bEncryption = true, bool bCallback = true);
//No longer need seperate lowID checks as we now know the servers just give *.*.*.0 users a lowID
__inline bool IsLowID(uint32 id){
	return (id < 16777216);
}
CString ipstr(uint32 nIP);
CString ipstr(uint32 nIP, uint16 nPort);
CString ipstr(LPCTSTR pszAddress, uint16 nPort);
CStringA ipstrA(uint32 nIP);
void ipstrA(CHAR* pszAddress, int iMaxAddress, uint32 nIP);
__inline CString ipstr(in_addr nIP){
	return ipstr(*(uint32*)&nIP);
}
__inline CStringA ipstrA(in_addr nIP){
	return ipstrA(*(uint32*)&nIP);
}


///////////////////////////////////////////////////////////////////////////////
// Date/Time
//
time_t safe_mktime(struct tm* ptm);
bool AdjustNTFSDaylightFileTime(uint64& ruFileDate, LPCTSTR pszFilePath);// X: [64T] - [64BitTime]


///////////////////////////////////////////////////////////////////////////////
// RC4 Encryption
//
struct RC4_Key_Struct{
	uint8 abyState[256];
	uint8 byX;
	uint8 byY;
};

RC4_Key_Struct* RC4CreateKey(const uchar* const pachKeyData, const size_t nLen, RC4_Key_Struct* key = NULL, const bool bSkipDiscard = false);
void RC4Crypt(const uchar* const pachIn, uchar* const pachOut, const size_t nLen, RC4_Key_Struct* const key);

/*
// netfinity: Convert between 32 and 64 bit time values
///////////////////////////////////////////////////////////////////////////////
// 64bit support
//
inline
time_t ConvertFromTime32(const uint32 uTime)
{
#if defined(_TIME64_T_DEFINED) && !defined(_USE_32BIT_TIME_T)
	const time_t	now = time(NULL);
	time_t	uTimeNew = (now & 0xFFFFFFFF00000000LL) | static_cast<time_t>(static_cast<uint64>(uTime));
	// Just in case eMule would still live in the 22nd century
	if (uTimeNew > (now + 0x80000000LL))
		uTimeNew -= 0x100000000LL;
	return uTimeNew;
#else
	return static_cast<time_t>(uTime);
#endif
}

inline
uint32 ConvertToTime32(const time_t uTime)
{
	return static_cast<uint32>(uTime);
}*/

void HeapSort(CAtlArray<uint16> &count, size_t first, size_t last);
inline sint_ptr charhexval(uint_ptr c);
bool extInList(LPCTSTR extlist,LPCTSTR ext);
bool checkExt(LPCTSTR extlist, LPCTSTR filename);
float GetMaxSlotSpeed(float maxUp);
// crt fast version
uint32 strtoul(const char*_src, const char **_dest);
uint32 wcstoul(const wchar_t*_src, const wchar_t**_dest);
char* inet_addr_ntohl(char*_src, uint32& ip);

uint32 GetHostIP();

int GZipCompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level);
int GZipUncompress(Bytef **dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
