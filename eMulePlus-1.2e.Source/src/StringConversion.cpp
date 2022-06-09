//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	Copyright (C) 2007-2008 Eklmn ( eklmn@users.sourceforge.net / http://www.emule-project.net )

#include "stdafx.h"
#include "StringConversion.h"
#include "emule.h"
#include "otherfunctions.h"
#include <share.h>

//////////////////////////////////////////////////////////////////////////////////////////////////
//	The function decodes the Unicode string from UTF8 byte stream
//	UTF-8 Bit distribution:
//	1 byte - 0xxxxxxx								(allows to encode 7 bit)
//	2 byte - 110yyyyy 10xxxxxx						(allows to encode 11 bit)
//	3 byte - 1110zzzz 10yyyyyy 10xxxxxx				(allows to encode 16 bit)
//	4 byte - 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx	(allows to encode 21 bit)
//	Return value:
//	length of the decoded string or < 0 if input string doesn't conform to UTF8
//	Note:
//	all values over 16-bit are encoded with surrogate pairs
//	Details:
//	http://msdn2.microsoft.com/en-us/library/ms776414.aspx
//	http://www.unicode.org/versions/Unicode4.0.0/ch03.pdf
static int Utf8ToWc(const char *pcUtf8, unsigned uiUtf8Size, LPWSTR pwc, unsigned uiWideCharSize)
{
	uint32		dwEncChar, dwDecChar, dwEncLen;
	unsigned	uiWideCharSize0 = uiWideCharSize;

	while (uiUtf8Size != 0 && uiWideCharSize != 0)
	{
		if (uiUtf8Size > 3)
			dwEncChar = PEEK_DWORD(pcUtf8);
		else if (uiUtf8Size > 2)
			dwEncChar = PEEK_WORD(pcUtf8) | (*((byte*)(pcUtf8 + 2)) << 16);
		else if (uiUtf8Size > 1)
			dwEncChar = PEEK_WORD(pcUtf8);
		else
			dwEncChar = *((byte*)(pcUtf8));

	//	One byte
		if ((dwEncChar & 0xFF) < 0x80)
		{
			dwDecChar = dwEncChar & 0x7F;
		//	Note: the '\0' in the middle of the string is not allowed
			if (dwDecChar == 0)
			{
				if (uiUtf8Size != 1)
					return -1;
				break;	//don't return error if stream has trailing zero
			}
			dwEncLen = 1;
		}
	//	Two bytes
		else if (((dwEncChar & 0xC0E0) == 0x80C0))
		{
			dwDecChar = ((dwEncChar & 0x1F) << 6) | ((dwEncChar & 0x3F00) >> 8);
		//	Verify if decoded value could be encoded with less number of bytes
			if (dwDecChar < 0x80)
				return -1;
			dwEncLen = 2;
		}
	//	Three bytes
		else if (((dwEncChar & 0xC0C0F0) == 0x8080E0))
		{
			dwDecChar = ((dwEncChar & 0x0F) << 12) | ((dwEncChar & 0x3F00) >> 2) | ((dwEncChar & 0x3F0000) >> 16);
		//	Verify if decoded value could be encoded with less number of bytes
			if (dwDecChar < 0x0800)
				return -1;
			dwEncLen = 3;
		}
	//	Four bytes
		else if (((dwEncChar & 0xC0C0C0F8) == 0x808080F0) && uiWideCharSize > 1)
		{
		//	Note: These values can not be represented by Windows 16-bit WCHAR therefore
		//	Windows represents them with 2 WCHARs (known as surrogate pair) according to UTF-16 format.
		//	UTF-16 format Bit Distribution
		//	1 word:		xxxxxxxx.xxxxxxxx			xxxxxxxx.xxxxxxxx
		//	2 words:	000uuuuu.xxxxxxxx.xxxxxxxx	110110ww.wwxxxxxx 110111xx.xxxxxxxx (wwww = uuuuu - 1.)
			dwDecChar = ((dwEncChar & 0x07) << 18) | ((dwEncChar & 0x3F00) << 4) | ((dwEncChar & 0x3F0000) >> 10) | ((dwEncChar & 0x3F00000) >> 24);
		//	Verify if decoded value could be encoded with less number of bytes
			if (dwDecChar < 0x10000)
				return -1;
			dwEncLen = 4;
		//	Encode with surrogate pair
			*pwc++ = static_cast<WCHAR>(0xD800 | ((dwDecChar - 0x10000) >> 10) | ((dwDecChar & 0x00FC) >> 10));
			uiWideCharSize--;
			dwDecChar = 0xDC00 | (dwDecChar & 0x3FF);
		}
		else
			return -1;

		*pwc++ = static_cast<WCHAR>(dwDecChar);
		pcUtf8 += dwEncLen;
		uiUtf8Size -= dwEncLen;
		uiWideCharSize--;
	}

	return static_cast<int>(uiWideCharSize0 - uiWideCharSize);
}
#if 0
//////////////////////////////////////////////////////////////////////////////////////////////////
//	The function converts a byte array into string based on hexadecimal radix
//	Return value:
//	length of the string
static int MB2HexStr(CString *pstrTarget, const char *pcSrc, int iLength)
{
	int			iOutLen = iLength * 2 + 4;
	TCHAR		*pcAllocatedBuf = pstrTarget->GetBuffer(iOutLen);
	unsigned	uiCh;

	*pcAllocatedBuf++ = _T('h');
	*pcAllocatedBuf++ = _T('e');
	*pcAllocatedBuf++ = _T('x');
	*pcAllocatedBuf++ = _T('-');

	for (int i = 0; i < iLength; i++, pcAllocatedBuf += 2)
	{
		uiCh = static_cast<unsigned int>(pcSrc[i] & 0xFF);
		pcAllocatedBuf[0] = g_acHexDigits[uiCh >> 4];
		pcAllocatedBuf[1] = g_acHexDigits[uiCh & 0xF];
	}

	pstrTarget->ReleaseBuffer(iOutLen);

	return iOutLen;
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////
//	UNICODE:
//	The function reads the string from file and converts from multibyte to unicode form
//	ANSI:
//	The function reads from file direct into the string
//	Note:
//	Container of the returned string can be larger than string length, thus commonly used
//	class strings are not recommented as output to avoid extra memory usage.
void ReadMB2Str(ECodingFormat eCF, CString *pstrTarget, CFile &file, unsigned dwLength)
{
#ifdef _UNICODE
	CStringA strUTF8Buf;
	LPSTR pcAllocatedBuf = strUTF8Buf.GetBuffer(dwLength);

	file.Read(pcAllocatedBuf, dwLength);
	MB2Str(eCF, pstrTarget, pcAllocatedBuf, dwLength);
#else
	LPSTR pcAllocatedBuffer = pstrTarget->GetBuffer(dwLength);
	NOPRM(eCF);

	file.Read(pcAllocatedBuffer, dwLength);
	pstrTarget->ReleaseBuffer(dwLength);
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//	UNICODE:
//	The function converts the unicode string into multibyte form and writes the results into the file
//	ANSI:
//	The function writes the string direct into the file
void WriteStr2MB(ECodingFormat eCF, const CString &strSrc, CFile &file)
{
#ifdef _UNICODE
	CStringA strUTF8Buf;

	if (Str2MB(eCF, &strUTF8Buf, strSrc.GetString(), strSrc.GetLength()) != 0)
		file.Write(strUTF8Buf.GetString(), strUTF8Buf.GetLength());
#else
	NOPRM(eCF);
	file.Write(strSrc, strSrc.GetLength());
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//	UNICODE:
//	The function converts char array multibyte form to unicode string
//	ANSI:
//	The function just copy char buffer into the string object
//	Note:
//	Container of the returned string can be larger than string length, thus commonly used
//	class strings are not recommented as output to avoid extra memory usage.
int MB2Str(ECodingFormat eCF, CString *pstrTarget, const char *pcSrc, int iLength)
{
	if (iLength < 0)
		iLength = static_cast<int>(strlen(pcSrc));

#ifdef _UNICODE
	if (iLength == 0)
	{
		pstrTarget->Truncate(0);
		return 0;
	}

	if ((iLength >= 4) && HAS_BOM(pcSrc))
	{
		pcSrc += 3;
		iLength -= 3;
		eCF = cfUTF8;
	}

	LPWSTR pwcAllocatedBuffer = pstrTarget->GetBuffer(iLength);
	int iLen;

	for (;;)
	{
		if (eCF != cfLocalCodePage)
		{
		//	Note: In case of UTF8 decoding MultiByteToWideChar() behaves differently 
		//	on different Windows systems (Win9x and NT-based)
		//	therefore we are using our own implementation
			iLen = Utf8ToWc(pcSrc, iLength, pwcAllocatedBuffer, iLength);
			if (iLen >= 0)
				break;
		}

		iLen = MultiByteToWideChar(CP_ACP, // code page
							0, // character-type options
							pcSrc, // string to map
							iLength,       // number of bytes in string
							pwcAllocatedBuffer,  // wide-character buffer
							iLength);     // size of buffer
		break;
	}

	pstrTarget->ReleaseBuffer(iLen);

	return iLen;
#else
	NOPRM(eCF);

	pstrTarget->SetString(pcSrc, iLength);

	return pstrTarget->GetLength();
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// The function converts the string in multibyte form to unicode string
//	Note:
//	Container of the returned string can be larger than string length, thus commonly used
//	class strings are not recommented as output to avoid extra memory usage.
int MB2Str(ECodingFormat eCF, CString *pstrTarget, const CStringA &strSrc)
{
	return MB2Str(eCF, pstrTarget, strSrc.GetString(), strSrc.GetLength());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// The function converts the unicode string in multibyte string
int Str2MB(ECodingFormat eCF, CStringA *pstrTarget, const CString &strSrc)
{
	return Str2MB(eCF, pstrTarget, strSrc.GetString(), strSrc.GetLength());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//	UNICODE:
//	The function converts the unicode array in multibyte string
//	ANSI:
//	The function just copy char buffer into the string object
int Str2MB(ECodingFormat eCF, CStringA *pstrTarget, LPCTSTR ptcSrc, int iLength)
{
	if (iLength < 0)
		iLength = static_cast<int>(_tcslen(ptcSrc));

#ifdef _UNICODE
	if (iLength == 0)
	{
		pstrTarget->Truncate(0);
		return 0;
	}

	// calculate maximal possible size of the string in UTF8 format
	int iUTF8BufLen = (iLength * 4 + 3);
	LPSTR pcAllocatedBuf = pstrTarget->GetBuffer(iUTF8BufLen);
	UINT uiCodePage = (eCF != cfLocalCodePage) ? CP_UTF8 : CP_ACP;
	uint32 dwOffset = 0;

	if (eCF == cfUTF8withBOM)
	{
		POKE_DWORD(pcAllocatedBuf, 0x00BFBBEF);
		pcAllocatedBuf += 3;
		iUTF8BufLen -= 3;
		dwOffset = 3;
	}

	int iLen = WideCharToMultiByte(uiCodePage,            // code page
						0,            // performance and mapping flags
						ptcSrc,    // wide-character string
						iLength,          // number of chars in string
						pcAllocatedBuf,     // buffer for new string
						iUTF8BufLen,          // size of buffer
						NULL,     // default for unmappable chars
						NULL);  // set when default char used

	iUTF8BufLen = iLen + dwOffset;
	pstrTarget->ReleaseBuffer(iUTF8BufLen);

	return iUTF8BufLen;
#else
	NOPRM(eCF);

	pstrTarget->SetString(ptcSrc, iLength);

	return pstrTarget->GetLength();
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Encode ed2k specific URI
// see also: http://tools.ietf.org/html/rfc3986
CString Ed2kURIEncode(const CString &strSource)
{
	CString	strURL;
	const char	*pcSrc;

#ifdef _UNICODE
	CStringA strUTF8;

	Str2MB(cfUTF8, &strUTF8, strSource);
	CStringA *pstrSource = &strUTF8;
#else
	const CString *pstrSource = &strSource;
#endif

// preallocate maximal possible amount of memory to prevent permanent reallocation
	TCHAR *ptcOut = strURL.GetBuffer(pstrSource->GetLength()*3);
	pcSrc = pstrSource->GetString();

	for (int i = 0; i < pstrSource->GetLength(); i++)
	{
		unsigned uiSrc = static_cast<unsigned>(pcSrc[i] & 0xFF);

		if (uiSrc < 0x7Fu)
			*ptcOut++ = static_cast<TCHAR>(uiSrc);
		else
		{
			*ptcOut++ = _T('%');
			*ptcOut++ = g_acHexDigits[uiSrc >> 4];
			*ptcOut++ = g_acHexDigits[uiSrc & 0xf];
		}
	}
	*ptcOut = _T('\0');
	strURL.ReleaseBuffer();

	return strURL;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Convert Unicode into UTF8 & then to URL format
// see also: http://www.ietf.org/rfc/rfc1738.txt & http://tools.ietf.org/html/rfc3986
CString URLEncode(const CString &strSource)
{
	CString	strURL;
	const char	*pcSrc;

#ifdef _UNICODE
	CStringA strUTF8;

	Str2MB(cfUTF8, &strUTF8, strSource);
	CStringA *pstrSource = &strUTF8;
#else
	const CString *pstrSource = &strSource;
#endif

// preallocate maximal possible amount of memory to prevent permanent reallocation
	TCHAR *ptcOut = strURL.GetBuffer(pstrSource->GetLength()*3);
	pcSrc = pstrSource->GetString();

	for (int i = 0; i < pstrSource->GetLength(); i++)
	{
		unsigned uiSrc = static_cast<unsigned>(pcSrc[i] & 0xFF);

		if (isalnum(uiSrc))
			*ptcOut++ = static_cast<TCHAR>(uiSrc);
		else if (isspace(uiSrc))
			*ptcOut++ = _T('+');
		else
		{
			*ptcOut++ = _T('%');
			*ptcOut++ = g_acHexDigits[uiSrc >> 4];
			*ptcOut++ = g_acHexDigits[uiSrc & 0xf];
		}
	}
	*ptcOut = _T('\0');
	strURL.ReleaseBuffer();

	return strURL;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//	UNICODE:
//	decode the string from URL. Detect & handle 3 scenarios that are possible:
//	1) URL is a raw Unicode string
//	2) URL is UTF8 code 
//	3) URL is UTF8 code with escape sequence
//	ANSI:
//	decode the string from URL. Detect & handle 2 scenarios that are possible:
//	1) URL is UTF8 code 
//	2) URL is UTF8 code with escape sequence
CString URLDecode(const CString &strURL, bool bAllowCR/*false*/)
{
#ifdef _UNICODE
	CStringW	strDecoded;
	CStringA	strUTF8;
	unsigned	uiChar, uiDgt1, uiDgt2;

// preallocate memory to prevent permanent reallocation
	strUTF8.Preallocate(strURL.GetLength());

	for (int x = 0; x < strURL.GetLength(); ++x)
	{
		uiChar = static_cast<unsigned>(strURL.GetAt(x));
		if (uiChar > 0x00FF)	// test if we have an Unicode string
			return strURL;

		if ( (static_cast<TCHAR>(uiChar) == _T('%')) && ((x + 2) < strURL.GetLength()) &&
			((uiDgt1 = HexChr2Num(strURL.GetAt(x + 1))) <= 15) &&
			((uiDgt2 = HexChr2Num(strURL.GetAt(x + 2))) <= 15) )
		{
			uiChar = (uiDgt1 << 4) | uiDgt2;
			x += 2;
		}

	// don't allow the addition of the control symbols
		if ((uiChar > 0x001F) || (bAllowCR && (uiChar == _T('\n') || uiChar == _T('\r'))))
			strUTF8.AppendChar(static_cast<char>(uiChar));
	}

	MB2Str(cfUTF8, &strDecoded, strUTF8);

	return strDecoded;
#else
	EMULE_TRY

	CString		outStr;
	int			more = -1;
	unsigned	uiDgt1, uiDgt2;

	for (int x = 0; x < strURL.GetLength(); ++x)
	{
		if ( (strURL.GetAt(x) == '%') && ((x + 2) < strURL.GetLength()) &&
			((uiDgt1 = HexChr2Num(strURL.GetAt(x + 1))) <= 15) &&
			((uiDgt2 = HexChr2Num(strURL.GetAt(x + 2))) <= 15) )
		{
			int Result;
			int sumb;
			x += 2;

			// EC 11.07.03 UTF-8 Decoding
			Result = (uiDgt1 << 4) | uiDgt2;
			if ((Result & 0xc0) == 0x80) // 10xxxxxx (continuation byte)
			{
				sumb = (sumb << 6) | (Result & 0x3f) ;	// Add 6 bits to sumb
				if (--more == 0 && static_cast<byte>(sumb) > 0x1F) 
					outStr.AppendChar((char)sumb) ; // Add char to sbuf
			}
			else if ((Result & 0x80) == 0x00) 		// 0xxxxxxx (yields 7 bits)
			{
				outStr.AppendChar((char)Result);			// Append Character
			}
			else if ((Result & 0xe0) == 0xc0) 		// 110xxxxx (yields 5 bits)
			{
				sumb = Result & 0x1f;
				more = 1;				// Expect 1 more byte
			}
			else if ((Result & 0xf0) == 0xe0) 		// 1110xxxx (yields 4 bits)
			{
				sumb = Result & 0x0f;
				more = 2;				// Expect 2 more bytes
			}
			else if ((Result & 0xf8) == 0xf0)
			{		// 11110xxx (yields 3 bits)
				sumb = Result & 0x07;
				more = 3;				// Expect 3 more bytes
			}
			else if ((Result & 0xfc) == 0xf8)
			{		// 111110xx (yields 2 bits)
				sumb = Result & 0x03;
				more = 4;				// Expect 4 more bytes
			}
			else /*if ((b & 0xfe) == 0xfc)*/ 	// 1111110x (yields 1 bit)
			{
				sumb = Result & 0x01;
				more = 5;				// Expect 5 more bytes
			}
		}
		else
		{
			outStr.AppendChar(strURL.GetAt(x));
		}
		// EC 11.07.03 UTF-8 Decoding Ends
	}
	return outStr;

	EMULE_CATCH

	return strURL;
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// test if string must be encoded with UTF8
bool IsUTF8Required(LPCTSTR pcStr)
{
#ifdef _UNICODE
	unsigned	ui;

	for (;;)
	{
		if ((ui = ((unsigned)*pcStr++ - 1u)) >= 0x007E)
			break;
	}
	return ((int)ui < 0) ? false : true;	//less than zero when break on '\0'
#else
	NOPRM(pcStr);
	return false;
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//	The function verifies if the string conforms to the UTF-8 Bit distribution:
//	1 byte - 0xxxxxxx
//	2 byte - 110yyyyy 10xxxxxx
//	3 byte - 1110zzzz 10yyyyyy 10xxxxxx
//	4 byte - 11110uuu 10uuzzzz 10yyyyyy 10xxxxxx
//
//	Note that ASCII characters are conform to the UTF8.
bool IsUTF8(LPCSTR pcStr)
{
	static const uint32 dwUTF8MaskArray[3][2] = {
												{0x0000E0C0, 0x0000C080},
												{0x00F0C0C0, 0x00E08080},
												{0xF8C0C0C0, 0xF0808080} };
	static const byte byteUTF8LengthArray[64] = {
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// chars 0xC0 - 0xCF
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,		// chars 0xD0 - 0xDF
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,		// chars 0xE0 - 0xEF
		4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 };	// chars 0xF0 - 0xFF
	char	cCh;

	while ((cCh = *pcStr) != '\0')
	{
		if ((cCh & 0x80) == 0)
		{
			pcStr++;
			continue;
		}
		if ((cCh & 0x40) == 0)
			return false;

		int iLenIdx = static_cast<int>(byteUTF8LengthArray[cCh & 0x3F]) - 1;

		if (iLenIdx < 0)	// found invalid UTF8 character
			return false;

		uint32	dwCh, dwUTF8Enc = 0;
		int		i = iLenIdx;

		do
		{
			if ((dwCh = static_cast<byte>(*pcStr++)) == '\0')
				return false;	// string terminator in a middle of the encoded UTF8

			dwUTF8Enc <<= 8;
			dwUTF8Enc |= dwCh;
		} while(--i >= 0);

		iLenIdx--;
		if ((dwUTF8Enc & dwUTF8MaskArray[iLenIdx][0]) != dwUTF8MaskArray[iLenIdx][1])
			return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//	The functions detects and returns the supported file formats.
ETextFileFormat GetTextFileFormat(LPCTSTR pcFile)
{
	ETextFileFormat eTFF = tffUnknown;

#ifdef _UNICODE
	FILE	*pFile = _tfsopen(pcFile, _T("rb"), _SH_DENYWR);

	if (pFile != NULL)
	{
		uint32 dwBOM = 0;

		fread(&dwBOM, 1, 3, pFile);
		fclose(pFile);

		if ((dwBOM & 0xFFFFu) == 0xFEFFu)
			eTFF = tffUnicode;
		else if (dwBOM == 0x00BFBBEFul)
			eTFF = tffUTF8;
		else
			eTFF = tffANSI;
	}
#else
	NOPRM(pcFile);
#endif

	return eTFF;
}
