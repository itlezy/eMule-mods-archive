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
//	Copyright (C) 2007 Eklmn ( eklmn@users.sourceforge.net / http://www.emule-project.net )
#pragma once

typedef enum ECodingFormat
{
	cfLocalCodePage,
	cfUTF8,
	cfUTF8withBOM
};

typedef enum ETextFileFormat
{
	tffUnknown,
	tffANSI,
	tffUnicode,
	tffUTF8
};

#define HAS_BOM(pBuffer) ((*(uint32*)(pBuffer)&0x00FFFFFFul) == 0x00BFBBEFul)

void ReadMB2Str(ECodingFormat eCF, CString *pstrTarget, CFile &file, unsigned dwLength);
void WriteStr2MB(ECodingFormat eCF, const CString &strSrc, CFile &file);
int MB2Str(ECodingFormat eCF, CString *pstrTarget, const char *pchSource, int iLength = -1);
int MB2Str(ECodingFormat eCF, CString *pstrTarget, const CStringA &strSrc);
int Str2MB(ECodingFormat eCF, CStringA *pstrTarget, const CString &strSrc);
int Str2MB(ECodingFormat eCF, CStringA *pstrTarget, LPCTSTR pchSrc, int iLength = -1);
CString Ed2kURIEncode(const CString &strSource);
CString URLEncode(const CString &strSource);
CString URLDecode(const CString &strURL, bool bAllowCR = false);
bool IsUTF8Required(LPCTSTR pchStr);
bool IsUTF8(LPCSTR pchStr);
ETextFileFormat GetTextFileFormat(LPCTSTR pcFile);
